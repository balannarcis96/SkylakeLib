//!
//! \file EntityStore.h
//! 
//! \brief Thread safe Entity Store
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    template<typename TEntityStore>
    struct SharedEntityDeallocator
    {
        using RootComponent         = typename TEntityStore::RootComponent;
        using TDecayObject          = RootComponent;
        using MyMemoryPolicy        = MemoryPolicy::SharedMemoryPolicy;
        using MyMemoryPolicyApplier = SKL::MemoryPolicy::MemoryPolicyApplier<MyMemoryPolicy>;

        void operator()( RootComponent* InPtr ) const noexcept
        {
            Deallocate( InPtr );
        }

        static void Deallocate( RootComponent* InPtr ) noexcept;
    };

    template<TEntityType MyEntityType, typename TEntityId, size_t TMaxEntities, typename TRootComponentData, typename ...TComponents>
    struct EntityStore
    {
        using MyType           = EntityStore<MyEntityType, TEntityId, TMaxEntities, TRootComponentData, TComponents...>;
        using OnAllFreedTask   = ASD::UniqueFunctorWrapper<32, void(SKL_CDECL*)()noexcept>;

        using EntityId                           = TEntityId;
        using NonAtomicEntityId                  = SKL::EntityId<typename TEntityId::Variant, TEntityId::CExtendexIndex, false>;
        using IndexType                          = typename EntityId::TIndexType;
        using RootComponentData                  = TRootComponentData;
        using Componenets                        = std::tuple<TComponents...>;
        static constexpr size_t MaxEntities      = TMaxEntities;
        static constexpr size_t ComponentsCount  = 1 + sizeof...( TComponents );
        static constexpr IndexType IdentityValue = 0;
        static constexpr TEntityType EntityType  = MyEntityType;
        
        static_assert( std::is_unsigned_v<IndexType> );
        static_assert( TMaxEntities <= static_cast<size_t>( std::numeric_limits<IndexType>::max() ) );

        struct RootComponent : TRootComponentData
        {
            //! get the id of this entity
            SKL_FORCEINLINE NonAtomicEntityId GetId() const noexcept { return Id; }

            //! get the owning entity store
            SKL_FORCEINLINE MyType& GetEntityStore() noexcept 
            { 
                SKL_ASSERT( nullptr != MyStore );
                return *MyStore;
            }
        
            //! get the owning entity store
            SKL_FORCEINLINE const MyType& GetEntityStore() const noexcept 
            { 
                SKL_ASSERT( nullptr != MyStore );
                return *MyStore;
            }
        
            //! get component for entity
            template<typename TComponent>
            SKL_FORCEINLINE TComponent& GetComponent() noexcept
            {
                return GetEntityStore().template GetComponent<TComponent>( Id.GetIndex() );
            }

            //! get component for entity
            template<typename TComponent>
            SKL_FORCEINLINE const TComponent& GetComponent() const noexcept
            {
                return GetEntityStore().template GetComponent<TComponent>( Id.GetIndex() );
            }

        private:
            TEntityId Id;                 //!< entity id
            MyType*   MyStore{ nullptr }; //!< ptr to owning store

            friend MyType;
            friend SharedEntityDeallocator<MyType>;
        };

        struct SharedRootComponent: 
                  protected MemoryPolicy::ControlBlock
                , RootComponent
        {
            SharedRootComponent() noexcept
                : MemoryPolicy::ControlBlock( 0, sizeof( SharedRootComponent ) ), RootComponent() {}
            ~SharedRootComponent() noexcept = default;

            friend MyType;
            friend SharedEntityDeallocator<MyType>;
        };

        using TEntitySharedPtr = TSharedPtr<RootComponent, SharedEntityDeallocator<MyType>>;
        using TEntityPtr       = RootComponent *;
        using MyStore          = SymmetricStore<IndexType, TMaxEntities, SharedRootComponent, TComponents...>;
        using MyIdStore        = UIDStore<IndexType, IdentityValue, static_cast<IndexType>( TMaxEntities )>;
        using Variant          = typename TEntityId::Variant;

        //! initialize the store
        RStatus Initialize() noexcept
        {
            IdStore.SetOnAllFreed( [ this ]() noexcept -> void 
            {
                if( false == OnAllFreed.IsNull() )
                {
                    OnAllFreed();
                }
            } );

            // initialize the entities
            for( size_t i = 0; i < TMaxEntities; ++i )
            {
                auto& RComponent{ Store.GetComponent<SharedRootComponent>( static_cast<IndexType>( i ) ) };
             
                // set ref count
                RComponent.ReferenceCount.store( 0, std::memory_order_relaxed );
                
                // set id base data
                RComponent.Id = TEntityId{ MyEntityType, static_cast<IndexType>( i ), Variant{} };

                // set the store ptr
                RComponent.MyStore = this;
            }

            return RSuccess;
        }
        
        //! activate the store
        void Activate() noexcept
        {
            IdStore.Activate();
        }
        
        //! deactivate the store, when all entities are freed the callback is called
        void Deactivate() noexcept
        {
            IdStore.Deactivate();
        }

        //! allocate new entity
        template<typename ...TArgs>
        TEntitySharedPtr AllocateEntity( Variant InIdVariantData, TArgs... InArgs ) noexcept
        {
            TEntitySharedPtr Result{ nullptr };

            if( true == IsActive() ) SKL_LIKELY
            {
                const auto NewUID{ IdStore.Allocate() };
                if( IdentityValue != NewUID )
                {
                    SharedRootComponent& RComponent{ Store.GetComponent<SharedRootComponent>( NewUID ) };
                    
                    SKL_ASSERT( 0 == RComponent.ReferenceCount.load( std::memory_order_relaxed ) );

                    // update the id
                    RComponent.Id.SetVariant( InIdVariantData );

                    // set the init ref count
                    RComponent.ReferenceCount.store( 1, std::memory_order_relaxed );

                    // update the ptr inside Result
                    auto* ResultPtr{ static_cast<RootComponent*>( &RComponent ) };
                    EnditSharedPtr<TEntitySharedPtr>::SetRawPtr( Result, ResultPtr );
                }
                else
                {
                    SKL_VER( "EntityStore::AllocateEntity() Reached max entities!" );
                }
            }

            return Result;
        }
        
        //! deallocate entity
        void DeallocateEntity( TEntityPtr InPtr ) noexcept
        {
            const SharedRootComponent* RC{ reinterpret_cast<const SharedRootComponent*>( 
                reinterpret_cast<const uint8_t*>( InPtr ) - sizeof( MemoryPolicy::ControlBlock )
            ) };

            SKL_ASSERT( 0 == RC->ReferenceCount.load( std::memory_order_relaxed ) );
            const IndexType Id{ static_cast<IndexType>( InPtr->GetId().GetIndex() ) };
            IdStore.Deallocate( Id );
        }
        
        //! are all entities valid and ready to use
        bool IsValid() const noexcept
        {
            return Store.IsValid();
        }

        //! is the store active
        bool IsActive() const noexcept { return IdStore.IsActive(); }

        //! set the functor to be dispatched when all entities are dellocated and the store is not active
        template<typename TFunctor>
        void SetOnAllFreed( TFunctor&& InFunctor ) noexcept
        {
            OnAllFreed += std::forward<TFunctor>( InFunctor );
        }
        
        //! get component for entity
        template<typename TComponent> requires( false == std::is_same_v<TComponent, TRootComponentData> )
        SKL_FORCEINLINE TComponent& GetComponent( NonAtomicEntityId InEntityId ) noexcept
        {
            return Store.GetComponent<TComponent>( InEntityId.GetIndex() );
        }

        //! get component for entity
        template<typename TComponent> requires( false == std::is_same_v<TComponent, TRootComponentData> )
        SKL_FORCEINLINE const TComponent& GetComponent( NonAtomicEntityId InEntityId ) const noexcept
        {
            return Store.GetComponent<TComponent>( InEntityId.GetIndex() );
        }

        //! get component for entity
        template<typename TComponent> requires( false == std::is_same_v<TComponent, TRootComponentData> )
        SKL_FORCEINLINE TComponent& GetComponent( IndexType InEntityIndex ) noexcept
        {
            return Store.GetComponent<TComponent>( InEntityIndex );
        }

        //! get component for entity
        template<typename TComponent> requires( false == std::is_same_v<TComponent, TRootComponentData> )
        SKL_FORCEINLINE const TComponent& GetComponent( IndexType InEntityIndex ) const noexcept
        {
            return Store.GetComponent<TComponent>( InEntityIndex );
        }
        
        //! get entity root component
        SKL_FORCEINLINE const RootComponent& GetEntityRaw( NonAtomicEntityId InEntityId ) const noexcept
        {
            const auto& RComponent{ Store.GetComponent<SharedRootComponent>( InEntityId.GetIndex() ) };
            return static_cast<const RootComponent&>( RComponent );
        }
        
        //! get entity root component
        SKL_FORCEINLINE RootComponent& GetEntityRaw( NonAtomicEntityId InEntityId ) noexcept
        {
            auto& RComponent{ Store.GetComponent<SharedRootComponent>( InEntityId.GetIndex() ) };
            return static_cast<RootComponent&>( RComponent );
        }
        
        //! get entity root component
        SKL_FORCEINLINE const RootComponent& GetEntityRaw( IndexType InEntityIndex ) const noexcept
        {
            const auto& RComponent{ Store.GetComponent<SharedRootComponent>( InEntityIndex ) };
            return static_cast<const RootComponent&>( RComponent );
        }
        
        //! get entity root component
        SKL_FORCEINLINE RootComponent& GetEntityRaw( IndexType InEntityIndex ) noexcept
        {
            auto& RComponent{ Store.GetComponent<SharedRootComponent>( InEntityIndex ) };
            return static_cast<RootComponent&>( RComponent );
        }
        
    private:
        MyStore        Store;      //!< entities store   
        MyIdStore      IdStore;    //!< ids store
        OnAllFreedTask OnAllFreed; //!< functor to dispatch when all entities are freed and the store is not active
    };

    template<typename TEntityStore>
    void SharedEntityDeallocator<TEntityStore>::Deallocate( typename TEntityStore::RootComponent* InPtr ) noexcept
    {
        auto* RC{ reinterpret_cast<typename TEntityStore::SharedRootComponent*>( 
            reinterpret_cast<uint8_t*>( InPtr ) - sizeof( MemoryPolicy::ControlBlock )
        ) };

        if( true == RC->ReleaseReference() )
        {
            auto& Store{ InPtr->GetEntityStore() };
            SKL_ASSERT( true == Store.IsValid() );
            Store.DeallocateEntity( InPtr );
        }
    }
}
