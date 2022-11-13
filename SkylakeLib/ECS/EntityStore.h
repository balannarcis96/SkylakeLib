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
        using AtomicEntityId                     = SKL::TEntityId<typename TEntityId::Variant, TEntityId::CExtendexIndex, true>;
        using NonAtomicEntityId                  = SKL::TEntityId<typename TEntityId::Variant, TEntityId::CExtendexIndex, false>;
        using IndexType                          = typename EntityId::TIndexType;
        using RootComponentData                  = TRootComponentData;
        using Componenets                        = std::tuple<TComponents...>;
        static constexpr size_t MaxEntities      = TMaxEntities;
        static constexpr size_t ComponentsCount  = 1 + sizeof...( TComponents );
        static constexpr IndexType IdentityValue = 0;
        static constexpr TEntityType EntityType  = MyEntityType;
        
        static_assert( std::is_unsigned_v<IndexType> );
        static_assert( TMaxEntities <= static_cast<size_t>( std::numeric_limits<IndexType>::max() ) );
        static_assert( true == std::is_class_v<TRootComponentData>, "The root component data a class/struct type" );
        static_assert( false == std::is_polymorphic_v<TRootComponentData>, "The root component data must not be a polymorphic or abstract type" );

        struct RootComponentInternalData
        {
            //! get the id of this entity
            SKL_FORCEINLINE SKL_NODISCARD NonAtomicEntityId GetId() const noexcept { return Id; }

            //! get the owning entity store
            SKL_FORCEINLINE SKL_NODISCARD MyType& GetEntityStore() noexcept 
            { 
                SKL_ASSERT( nullptr != MyStore );
                return *MyStore;
            }
        
            //! get the owning entity store
            SKL_FORCEINLINE SKL_NODISCARD const MyType& GetEntityStore() const noexcept 
            { 
                SKL_ASSERT( nullptr != MyStore );
                return *MyStore;
            }
        
            //! get component for entity
            template<typename TComponent>
            SKL_FORCEINLINE SKL_NODISCARD TComponent& GetComponent() noexcept
            {
                return GetEntityStore().template GetComponent<TComponent>( Id.GetIndex() );
            }

            //! get component for entity
            template<typename TComponent>
            SKL_FORCEINLINE SKL_NODISCARD const TComponent& GetComponent() const noexcept
            {
                return GetEntityStore().template GetComponent<TComponent>( Id.GetIndex() );
            }

        private:
            AtomicEntityId Id;      //!< entity id
            MyType*        MyStore; //!< ptr to owning store

            friend MyType;
            friend SharedEntityDeallocator<MyType>;
        };

        struct RootComponent : 
              AOD::CustomObject
            , RootComponentInternalData
            , TRootComponentData
        {
            RootComponent() noexcept
                : AOD::CustomObject( &CustomObjectDeleter )
                , RootComponentInternalData{}
                , TRootComponentData{} {}
            ~RootComponent() noexcept = default;

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

        struct TRootComponentDataTraits
        {
            struct HasOnDestroy
            {
            private:
                template<typename  > static std::false_type test(...);
                template<typename U> static auto            test(int) -> decltype( std::declval<U>().OnDestroy(), std::true_type() );
            public:
                static constexpr bool value = std::is_same_v<decltype( test<TRootComponentData>( 0 ) ), std::true_type>;
            };

            static constexpr bool has_ondestroy_v = HasOnDestroy::value;
        };

        using TEntitySharedPtr = TSharedPtr<RootComponent, SharedEntityDeallocator<MyType>>;
        using TEntityPtr       = RootComponent *;
        using MyStore          = SymmetricStore<IndexType, TMaxEntities, SharedRootComponent, TComponents...>;
        using MyIdStore        = UIDStore<IndexType, IdentityValue, static_cast<IndexType>( TMaxEntities )>;
        using Variant          = typename TEntityId::Variant;

        //! initialize the store
        SKL_NODISCARD RStatus Initialize() noexcept
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
                RComponent.Id = ( TEntityId{ MyEntityType, static_cast<IndexType>( i ), Variant{} } ).GetId();

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
        
        //! are all entities valid and ready to use
        SKL_FORCEINLINE SKL_NODISCARD bool IsValid() const noexcept { return Store.IsValid(); }

        //! is the store active
        SKL_FORCEINLINE SKL_NODISCARD bool IsActive() const noexcept { return IdStore.IsActive(); }

        //! is ready to be destroyed
        SKL_FORCEINLINE SKL_NODISCARD bool IsShutdownAndReadyToDestroy() const noexcept { IdStore.IsShutdownAndReadyToDestroy(); }

        //! allocate new entity
        template<typename... TArgs>
        SKL_NODISCARD TEntitySharedPtr AllocateEntity( Variant InIdVariantData, TArgs... InArgs ) noexcept
        {
            TEntitySharedPtr Result{ nullptr };

            if( true == IsActive() ) SKL_LIKELY
            {
                const auto NewUID{ IdStore.Allocate() };
                if( IdentityValue != NewUID ) SKL_UNLIKELY
                {
                    SharedRootComponent& RComponent{ Store.GetComponent<SharedRootComponent>( NewUID ) };
                    
                    SKL_ASSERT( 0 == RComponent.ReferenceCount.load( std::memory_order_relaxed ) );

                    // update the id
                    RComponent.Id.SetVariant( InIdVariantData );

                    // set the init ref count
                    RComponent.ReferenceCount.store( 1, std::memory_order_relaxed );

                    // update the ptr inside Result
                    auto* ResultPtr{ static_cast<RootComponent*>( &RComponent ) };
                    EditSharedPtr<TEntitySharedPtr>::SetRawPtr( Result, ResultPtr );
                }
                else
                {
                    SKLL_VER( "EntityStore::AllocateEntity() Reached max entities!" );
                }
            }

            return Result;
        }
        
        //! deallocate entity
        void DeallocateEntity( TEntityPtr InPtr ) noexcept
        {
            static_assert( CExpectMemoryPolicyVersion( 1 ) );
            SharedRootComponent* RC
            { 
                reinterpret_cast<SharedRootComponent*>
                ( 
                    reinterpret_cast<uint8_t*>( InPtr ) - sizeof( MemoryPolicy::ControlBlock )
                ) 
            };
            
            if constexpr( TRootComponentDataTraits::has_ondestroy_v )
            {
                RC->OnDestroy();
            }

            SKL_ASSERT( 0 == RC->ReferenceCount.load( std::memory_order_relaxed ) );
            const IndexType Id{ static_cast<IndexType>( InPtr->GetId().GetIndex() ) };
            IdStore.Deallocate( Id );
        }
        
        //! set the functor to be dispatched when all entities are deallocated and the store is not active
        template<typename TFunctor>
        void SetOnAllFreed( TFunctor&& InFunctor ) noexcept
        {
            OnAllFreed += std::forward<TFunctor>( InFunctor );
        }
        
        //! get component for entity
        template<typename TComponent>
        SKL_FORCEINLINE SKL_NODISCARD TComponent& GetComponent( NonAtomicEntityId InEntityId ) noexcept
        {
            static_assert( false == std::is_same_v<TComponent, TRootComponentData> );
            return Store.GetComponent<TComponent>( InEntityId.GetIndex() );
        }

        //! get component for entity
        template<typename TComponent>
        SKL_FORCEINLINE SKL_NODISCARD const TComponent& GetComponent( NonAtomicEntityId InEntityId ) const noexcept
        {
            static_assert( false == std::is_same_v<TComponent, TRootComponentData> );
            return Store.GetComponent<TComponent>( InEntityId.GetIndex() );
        }

        //! get component for entity
        template<typename TComponent> 
        SKL_FORCEINLINE SKL_NODISCARD TComponent& GetComponent( IndexType InEntityIndex ) noexcept
        {
            static_assert( false == std::is_same_v<TComponent, TRootComponentData> );
            return Store.GetComponent<TComponent>( InEntityIndex );
        }

        //! get component for entity
        template<typename TComponent>
        SKL_FORCEINLINE SKL_NODISCARD const TComponent& GetComponent( IndexType InEntityIndex ) const noexcept
        {
            static_assert( false == std::is_same_v<TComponent, TRootComponentData> );
            return Store.GetComponent<TComponent>( InEntityIndex );
        }
        
        //! get entity root component
        SKL_FORCEINLINE SKL_NODISCARD const RootComponent& GetEntityRaw( NonAtomicEntityId InEntityId ) const noexcept
        {
            const auto& RComponent{ Store.GetComponent<SharedRootComponent>( InEntityId.GetIndex() ) };
            return static_cast<const RootComponent&>( RComponent );
        }
        
        //! get entity root component
        SKL_FORCEINLINE SKL_NODISCARD RootComponent& GetEntityRaw( NonAtomicEntityId InEntityId ) noexcept
        {
            auto& RComponent{ Store.GetComponent<SharedRootComponent>( InEntityId.GetIndex() ) };
            return static_cast<RootComponent&>( RComponent );
        }
        
        //! get entity root component
        SKL_FORCEINLINE SKL_NODISCARD const RootComponent& GetEntityRaw( IndexType InEntityIndex ) const noexcept
        {
            const auto& RComponent{ Store.GetComponent<SharedRootComponent>( InEntityIndex ) };
            return static_cast<const RootComponent&>( RComponent );
        }
        
        //! get entity root component
        SKL_FORCEINLINE SKL_NODISCARD RootComponent& GetEntityRaw( IndexType InEntityIndex ) noexcept
        {
            auto& RComponent{ Store.GetComponent<SharedRootComponent>( InEntityIndex ) };
            return static_cast<RootComponent&>( RComponent );
        }
        
        //! get the number of allocated entities
        SKL_FORCEINLINE SKL_NODISCARD size_t GetAllocatedEntitiesCount() const noexcept { return IdStore.GetAllocatedIdsCount(); }

    private:
        static void CustomObjectDeleter( AOD::CustomObject* InCustomObject ) noexcept
        {
            RootComponent* RC   { reinterpret_cast<RootComponent*>( InCustomObject ) };
            MyType&        Store{ RC->GetEntityStore() };
            SKL_ASSERT( true == Store.IsValid() );

            Store.DeallocateEntity( RC );
        }

    private:
        MyStore        Store;      //!< entities store   
        MyIdStore      IdStore;    //!< ids store
        OnAllFreedTask OnAllFreed; //!< functor to dispatch when all entities are freed and the store is not active
    };

    template<typename TEntityStore>
    void SharedEntityDeallocator<TEntityStore>::Deallocate( typename TEntityStore::RootComponent* InPtr ) noexcept
    {
        static_assert( CExpectMemoryPolicyVersion( 1 ) );
        auto* RC
        { 
            reinterpret_cast<typename TEntityStore::SharedRootComponent*>
            ( 
                reinterpret_cast<uint8_t*>( InPtr ) - sizeof( MemoryPolicy::ControlBlock )
            ) 
        };

        if( true == RC->ReleaseReference() )
        {
            auto& Store{ InPtr->GetEntityStore() };
            SKL_ASSERT( true == Store.IsValid() );
            Store.DeallocateEntity( InPtr );
        }
    }
}
