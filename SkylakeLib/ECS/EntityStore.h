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
    struct EntityStoreFlags
    {
        bool bExtendRootComponentToAsyncDispatchedObject{ true };
        bool bPaddEntityRootToMultipleOfCacheLine       { true };
        bool bRequireOnDestroy                          { true };
        bool bRequireOnCreate                           { false };
        bool bUseCachedAllocationUIDStore               { false };
        bool bDestructEntity                            { true };
    };

    struct ComponentWithPtrToRoot
    {
        template<typename TRootEntity>
        TRootEntity& GetRoot() noexcept
        {
            uint8_t* SelfPtr{ reinterpret_cast<uint8_t*>( this ) };
            SelfPtr -= sizeof( void * );
            return *( *reinterpret_cast<TRootEntity**>( SelfPtr ) );
        }

        template<typename TRootEntity>
        const TRootEntity& GetRoot() const noexcept
        {
            const uint8_t* SelfPtr{ reinterpret_cast<const uint8_t*>( this ) };
            SelfPtr -= sizeof( void * );
            return *( *reinterpret_cast<const TRootEntity* const*>( SelfPtr ) );
        }
    };

    template<TEntityType MyEntityType, typename TEntityId, size_t TMaxEntities, EntityStoreFlags TFlags, typename TRootComponentData, typename ...TComponents>
    struct EntityStore
    {
        static_assert( alignof( TRootComponentData ) <= SKL_ALIGNMENT, "RootComponentData must be max aligned to SKL_ALIGNMENT" );
        static_assert( ( sizeof( TRootComponentData ) % SKL_ALIGNMENT ) == 0U, "Size of RootComponentData must be a multiple of SKL_ALIGNMENT" );

        using MyType           = EntityStore<MyEntityType, TEntityId, TMaxEntities, TFlags, TRootComponentData, TComponents...>;
        using OnAllFreedTask   = ASD::UniqueFunctorWrapper<32, void(SKL_CDECL*)()noexcept>;
    
        using EntityId                                    = TEntityId;
        using AtomicEntityId                              = SKL::TEntityId<typename TEntityId::Variant, TEntityId::CExtendexIndex, true>;
        using NonAtomicEntityId                           = SKL::TEntityId<typename TEntityId::Variant, TEntityId::CExtendexIndex, false>;
        using IndexType                                   = typename EntityId::TIndexType;
        using RootComponentData                           = TRootComponentData;
        using Components                                  = std::tuple<TComponents...>;
        static constexpr size_t           MaxEntities     = TMaxEntities + 1;
        static constexpr size_t           ComponentsCount = 1 + sizeof...( TComponents );
        static constexpr IndexType        IdentityValue   = 0;
        static constexpr TEntityType      EntityType      = MyEntityType;
        static constexpr EntityStoreFlags Flags           = TFlags;
        
        static_assert( std::is_unsigned_v<IndexType>, "Unaccepted IndexType!" );
        static_assert( MaxEntities <= static_cast<size_t>( std::numeric_limits<IndexType>::max() ), "MaxEntities value exceeds the max value that a var of type IndexType can hold!" );
        static_assert( true == std::is_class_v<TRootComponentData>, "The root component data a class/struct type" );
        static_assert( false == std::is_polymorphic_v<TRootComponentData>, "The root component data must not be a polymorphic or abstract type" );
        
        struct alignas( SKL_ALIGNMENT ) RootComponentInternalData
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
            AtomicEntityId Id     { IdentityValue }; //!< entity id
            MyType*        MyStore{ nullptr };       //!< ptr to owning store

            friend MyType;
        };

        static_assert( sizeof( RootComponentInternalData ) == ( sizeof( void* ) * 2 ) );

        struct alignas( SKL_ALIGNMENT ) RootComponent_AsyncDispatched: 
              AOD::CustomObject
            , RootComponentInternalData
            , TRootComponentData
        {
            RootComponent_AsyncDispatched() noexcept
                : AOD::CustomObject{}
                , RootComponentInternalData{}
                , TRootComponentData{} {}
            ~RootComponent_AsyncDispatched() noexcept = default;

            friend MyType;
        };

        static_assert( ( sizeof( RootComponent_AsyncDispatched ) - sizeof( TRootComponentData ) ) == ( sizeof( void* ) * 6 ) );
        
        struct alignas( SKL_ALIGNMENT ) RootComponent_Not_AsyncDispatched: 
              RootComponentInternalData
            , TRootComponentData
        {
            RootComponent_Not_AsyncDispatched() noexcept
                : RootComponentInternalData{}
                , TRootComponentData{} {}
            ~RootComponent_Not_AsyncDispatched() noexcept = default;

            friend MyType;
        };

        static_assert( ( sizeof( RootComponent_Not_AsyncDispatched ) - sizeof( TRootComponentData ) ) == ( sizeof( void* ) * 2 ) );

        using RootComponentBase = std::conditional_t<Flags.bExtendRootComponentToAsyncDispatchedObject, RootComponent_AsyncDispatched, RootComponent_Not_AsyncDispatched>;

        struct RootComponentWithVirtualDeleter : RootComponentBase
        {
            RootComponentWithVirtualDeleter() noexcept = default;
            ~RootComponentWithVirtualDeleter() noexcept = default;

            void* VirtualDeleterPlaceholder{ &MyType::Delete };
        };

        using RootComponent = std::conditional_t<Flags.bExtendRootComponentToAsyncDispatchedObject, RootComponentWithVirtualDeleter, RootComponentBase>;
        using TEntityType   = RootComponent;

        struct PaddToMultipleOfCachelineBase
        {
            struct alignas( SKL_ALIGNMENT ) TestType 
                : protected MemoryPolicy::ControlBlock
                , TEntityType {};

            static consteval size_t CalculatePaddingSize()
            {
                constexpr double BSize   { static_cast<double>( sizeof( TestType ) ) };
                constexpr double Multiple{ BSize / static_cast<double>( SKL_CACHE_LINE_SIZE ) };
                constexpr double Fraction{ Multiple - static_cast<double>( static_cast<size_t>( Multiple ) ) };

                double FinalMultiple{ Multiple };
                if constexpr( Fraction > 0.0 )
                {
                    ++FinalMultiple;
                }

                return ( static_cast<size_t>( FinalMultiple ) * static_cast<size_t>( SKL_CACHE_LINE_SIZE ) )  - sizeof( TestType );
            }

            static constexpr size_t BaseSize    = sizeof( TestType );
            static constexpr size_t PaddingSize = CalculatePaddingSize();
        };

        //! does the store need to pad the root component to multiple of cache line (size)
        static constexpr bool CNeedsToPaddToMultipleOfCacheLine = ( TRUE == Flags.bPaddEntityRootToMultipleOfCacheLine ) && ( 0 != PaddToMultipleOfCachelineBase::PaddingSize );

        struct alignas( SKL_ALIGNMENT ) PaddedSharedRootComponent: 
                  protected MemoryPolicy::ControlBlock
                , TEntityType
        {
            PaddedSharedRootComponent() noexcept
                : MemoryPolicy::ControlBlock( 0, sizeof( TEntityType ) + sizeof( MemoryPolicy::ControlBlock ) ), TEntityType() 
            {
                SKL_ASSERT( 0 == ( reinterpret_cast<uint64_t>( this ) % SKL_CACHE_LINE_SIZE ) );
            }
            ~PaddedSharedRootComponent() noexcept = default;

            friend MyType;
        private:
            uint8_t Padding[PaddToMultipleOfCachelineBase::PaddingSize];
        };

        struct alignas( SKL_ALIGNMENT ) NotPaddedSharedRootComponent: 
                  protected MemoryPolicy::ControlBlock
                , TEntityType
        {
            NotPaddedSharedRootComponent() noexcept
                : MemoryPolicy::ControlBlock( 0, sizeof( TEntityType ) + sizeof( MemoryPolicy::ControlBlock ) ), TEntityType() {}
            ~NotPaddedSharedRootComponent() noexcept = default;

            friend MyType;
        };

        using SharedRootComponent = std::conditional_t<CNeedsToPaddToMultipleOfCacheLine, PaddedSharedRootComponent, NotPaddedSharedRootComponent>;

        //! total size of the root component in memory (bytes)
        static constexpr size_t CRootComponent_TotalSize        
                = sizeof( SharedRootComponent );

        //! portion of the root component memory used by the user (bytes)
        static constexpr size_t CRootComponent_UsedBytesByUser  
                = sizeof( TRootComponentData );

        //! portion of the root component memory used by the store ( we don't take in calculation the 8 bytes for the virtual deleter here )
        static constexpr size_t CRootComponent_UsedBytesByStore 
                = PaddToMultipleOfCachelineBase::BaseSize 
                - CRootComponent_UsedBytesByUser 
                - ( Flags.bExtendRootComponentToAsyncDispatchedObject ? sizeof( TVirtualDeleter<TEntityType> ) : 0U ); 

        //! is all user data on the first cache line of the root component
        static constexpr size_t CRootComponent_IsAllUserDataOnFirstCacheLine 
                = SKL_CACHE_LINE_SIZE >= CRootComponent_TotalSize;

        //! is any of the user data on the first cache line of the root component
        static constexpr size_t CRootComponent_IsAnyUserDataOnFirstCacheLine 
                = SKL_CACHE_LINE_SIZE > CRootComponent_UsedBytesByStore;

        //! how many bytes of the user type reside on the first cache line of the root component
        static constexpr size_t CRootComponent_AvailableBytesForUserOnFirstCacheLine = static_cast<size_t>( SKL_CACHE_LINE_SIZE ) > CRootComponent_UsedBytesByStore ? 
                                                                                       static_cast<size_t>( SKL_CACHE_LINE_SIZE ) - CRootComponent_UsedBytesByStore : 0U;

        //! how many bytes are left on the first cache line for the whole root component to use
        static constexpr size_t CRootComponent_BytesLeftOnFirstCacheLine = static_cast<size_t>( SKL_CACHE_LINE_SIZE ) > ( CRootComponent_UsedBytesByStore + CRootComponent_UsedBytesByUser ) ? 
                                                                           static_cast<size_t>( SKL_CACHE_LINE_SIZE ) - ( CRootComponent_UsedBytesByStore + CRootComponent_UsedBytesByUser ) : 0U;

        //! how many bytes of padding must the store add to the root component to rich cache line size multiplicity
        static constexpr size_t CRootComponent_BytesOfPadding = PaddToMultipleOfCachelineBase::PaddingSize;

        //! does the store use virtual deleter for the root component
        static constexpr bool CHasVirtualDeleter = Flags.bExtendRootComponentToAsyncDispatchedObject;

        // at least 8 bytes available for the user on the first cache line of the root component
        static_assert( sizeof( void* ) <= CRootComponent_AvailableBytesForUserOnFirstCacheLine );

        static_assert( ( false == Flags.bPaddEntityRootToMultipleOfCacheLine )
                    || ( 0U == ( sizeof( SharedRootComponent ) % SKL_CACHE_LINE_SIZE ) ), "Internal bug!?!?" );

        struct TRootComponentDataTraits
        {
            struct HasOnDestroy
            {
            private:
                template<typename  > static std::false_type test( ... );
                template<typename U> static auto            test( int32_t ) -> decltype( std::declval<U>().OnDestroy(), std::true_type() );
            public:
                static constexpr bool value = std::is_same_v<decltype( test<TRootComponentData>( 0 ) ), std::true_type>;
            };
            
            template<typename ...TArgs>
            struct HasOnCreate
            {
            private:
                template<typename  > static std::false_type test( ... );
                template<typename U> static auto            test( int32_t, TArgs... Args ) -> decltype( std::declval<U>().OnCreate( Args... ), std::true_type() );
            public:
                static constexpr bool value = std::is_same_v<decltype( test<TRootComponentData>( 0, std::declval<TArgs>()... ) ), std::true_type>;
            };

            static constexpr bool has_ondestroy_v = HasOnDestroy::value;
        };

        static_assert( ( false == Flags.bRequireOnDestroy )
                    || ( true == TRootComponentDataTraits::has_ondestroy_v ), "The given TRootComponentData must have a method [void OnDestroy()] defined!" );
        
        struct CustomUIDAllocationCacheToIndexConvert
        {
            SKL_NODISCARD SKL_FORCEINLINE static size_t ConvertToIndex( IndexType Id ) noexcept
            {
                return static_cast<size_t>( Id );
            }
        };

        template<typename TComponent>
        struct ComponentPaddingCondition
        {
            static constexpr size_t CPaddingSize = sizeof( void * );

            constexpr ComponentPaddingCondition() = default;
            constexpr ~ComponentPaddingCondition() = default;

            // should TComponent be padded
            static constexpr bool ShouldPadd() noexcept
            {
                return std::is_base_of_v<ComponentWithPtrToRoot, TComponent>;
            }

            // get padding size
            static constexpr size_t GetPaddingSize() noexcept { return CPaddingSize; }
        };

        struct MySharedMemoryStrategy
        {
            using MemoryPolicy        = SKL::MemoryPolicy::SharedMemoryPolicy<false>;
            using MemoryPolicyApplier = SKL::MemoryPolicy::MemoryPolicyApplier<MemoryPolicy>;

            struct Deallocator
            {
                // std api
                SKL_FORCEINLINE void operator()( TEntityType* InPtr ) const noexcept
                {
                    Deallocate( InPtr );
                }

                static void Deallocate( TEntityType* InPtr ) noexcept
                {
                    SKL::MemoryPolicy::ControlBlock& CB{ MemoryPolicy::GetControlBlockForObject( InPtr ) };
                    if( CB.ReleaseReference() )
                    {
                        Delete( InPtr );
                    }
                }
            };

            using Deallocator         = Deallocator;
            using DestructDeallocator = Deallocator;
        };

        using MyMemoryStrategy           = std::conditional_t<Flags.bExtendRootComponentToAsyncDispatchedObject, MemoryStrategy::SharedMemoryStrategy<TEntityType, true>, MySharedMemoryStrategy>;
        using TEntityPtr                 = TEntityType*;
        using TEntityRef                 = TEntityType&;
        using TEntityConstRef            = const TEntityType&;
        using TEntitySharedPtr           = TSharedPtr<TEntityType, MyMemoryStrategy, Flags.bDestructEntity>;
        using MyStore                    = SymmetricStoreWithConditionalPadding<IndexType, MaxEntities, ComponentPaddingCondition, SharedRootComponent, TComponents...>;
        using MyBasicIdStore             = UIDStore<IndexType, IdentityValue, static_cast<IndexType>( MaxEntities )>;
        using MyCachedAllocationsIdStore = UIDAllocationCache<IndexType, IdentityValue, static_cast<IndexType>( MaxEntities ), CustomUIDAllocationCacheToIndexConvert>;
        using MyIdStore                  = std::conditional_t<Flags.bUseCachedAllocationUIDStore, MyCachedAllocationsIdStore, MyBasicIdStore>;
        using Variant                    = typename TEntityId::Variant;

        static_assert( CHasVirtualDeleter == TEntitySharedPtr::CHasVirtualDeleter );

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
            for( size_t i = 0; i < MaxEntities; ++i )
            {
                auto& RComponent{ Store.GetComponent<SharedRootComponent>( static_cast<IndexType>( i ) ) };
             
                // set ref count
                RComponent.ReferenceCount.store( 0, std::memory_order_relaxed );
                
                // set id base data
                RComponent.Id = ( TEntityId{ MyEntityType, static_cast<IndexType>( i ), Variant{} } ).GetId();

                // set the store ptr
                RComponent.MyStore = this;

                // initialize the components as needed
                if constexpr( 0 < sizeof...( TComponents ) )
                {
                    InitializeComponents<TComponents...>( static_cast<IndexType>( i ) );
                }
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
            static_assert( FALSE == Flags.bUseCachedAllocationUIDStore, "Use AllocateSpecificEntity( ... )" );

            constexpr bool bHasOnCreate = TRootComponentDataTraits::HasOnCreate<TArgs...>::value;

            static_assert( ( FALSE == Flags.bRequireOnCreate )
                        || ( true == bHasOnCreate ), "TRootComponentData must define a method [void OnCreate( TArgs... )] with TArgs as parameters!" );

            static_assert( ( 0 == sizeof...( TArgs ) ) 
                        || ( true == bHasOnCreate ), "The OnCreate method does not take the given arguments as parameters!" );

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

                    if constexpr( bHasOnCreate )
                    {
                        ( void )RComponent.OnCreate( std::forward<TArgs>( InArgs )... );
                    }

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
        
        //! allocate specific entity
        template<typename... TArgs>
        SKL_NODISCARD TEntitySharedPtr AllocateSpecificEntity( IndexType IndexToAllocate, Variant InInitVariantData, TArgs... InArgs ) noexcept
        {
            static_assert( TRUE == Flags.bUseCachedAllocationUIDStore, "Use AllocateEntity( ... )" );

            constexpr bool bHasOnCreate = TRootComponentDataTraits::HasOnCreate<TArgs...>::value;

            static_assert( ( FALSE == Flags.bRequireOnCreate )
                        || ( true == bHasOnCreate ), "TRootComponentData must define a method [void OnCreate( TArgs... )] with TArgs as parameters!" );

            static_assert( ( 0 == sizeof...( TArgs ) ) 
                        || ( true == bHasOnCreate ), "The OnCreate method does not take the given arguments as parameters!" );

            TEntitySharedPtr Result{ nullptr };

            if( true == IsActive() ) SKL_LIKELY
            {
                const bool bAllocationStatus{ IdStore.Allocate( IndexToAllocate ) };
                if( bAllocationStatus ) SKL_UNLIKELY
                {
                    SharedRootComponent& RComponent{ Store.GetComponent<SharedRootComponent>( IndexToAllocate ) };
                    
                    SKL_ASSERT( 0 == RComponent.ReferenceCount.load( std::memory_order_relaxed ) );

                    // update the id
                    RComponent.Id.SetVariant( InInitVariantData );

                    // set the init ref count
                    RComponent.ReferenceCount.store( 1, std::memory_order_relaxed );

                    if constexpr( bHasOnCreate )
                    {
                        ( void )RComponent.OnCreate( std::forward<TArgs>( InArgs )... );
                    }

                    // update the ptr inside Result
                    auto* ResultPtr{ static_cast<RootComponent*>( &RComponent ) };
                    EditSharedPtr<TEntitySharedPtr>::SetRawPtr( Result, ResultPtr );
                }
                else
                {
                    SKLL_VER( "EntityStore::AllocateSpecificEntity() Entity already allocated or reached max entities!" );
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

            if constexpr( Flags.bUseCachedAllocationUIDStore )
            {
                const bool bResult{ IdStore.Deallocate( Id ) };
                SKL_ASSERT( true == bResult );
            }
            else
            {
                IdStore.Deallocate( Id );
            }
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
        SKL_FORCEINLINE SKL_NODISCARD RootComponent& GetEntityRaw( NonAtomicEntityId InEntityId ) noexcept
        {
            auto& RComponent{ Store.GetComponent<SharedRootComponent>( InEntityId.GetIndex() ) };
            return static_cast<RootComponent&>( RComponent );
        }
        
        //! get entity root component
        SKL_FORCEINLINE SKL_NODISCARD const RootComponent& GetEntityRaw( NonAtomicEntityId InEntityId ) const noexcept
        {
            const auto& RComponent{ Store.GetComponent<SharedRootComponent>( InEntityId.GetIndex() ) };
            return static_cast<const RootComponent&>( RComponent );
        }
        
        //! get entity root component
        SKL_FORCEINLINE SKL_NODISCARD RootComponent& GetEntityRaw( IndexType InEntityIndex ) noexcept
        {
            auto& RComponent{ Store.GetComponent<SharedRootComponent>( InEntityIndex ) };
            return static_cast<RootComponent&>( RComponent );
        }
        
        //! get entity root component
        SKL_FORCEINLINE SKL_NODISCARD const RootComponent& GetEntityRaw( IndexType InEntityIndex ) const noexcept
        {
            const auto& RComponent{ Store.GetComponent<SharedRootComponent>( InEntityIndex ) };
            return static_cast<const RootComponent&>( RComponent );
        }
        
        //! get the number of allocated entities
        SKL_FORCEINLINE SKL_NODISCARD size_t GetAllocatedEntitiesCount() const noexcept { return IdStore.GetAllocatedIdsCount(); }

        //! given the reference to root component data get the encompassing root component reference
        SKL_FORCEINLINE SKL_NODISCARD static TEntityRef Static_GetRootComponentDataParent( RootComponentData& InData ) noexcept
        {
            return static_cast<TEntityRef>( InData );
        }
        
        //! given the reference to root component data get the encompassing root component reference
        SKL_FORCEINLINE SKL_NODISCARD static TEntityConstRef Static_GetRootComponentDataParent( const RootComponentData& InData ) noexcept
        {
            return static_cast<TEntityConstRef>( InData );
        }

        //! construct a compatible EntityId for this store
        SKL_FORCEINLINE SKL_NODISCARD static TEntityId Static_ConstructEntityId( IndexType InIndex, Variant InVariant ) noexcept { return TEntityId{ MyEntityType, InIndex, InVariant }; }

        //! custom deleter
        static void Delete( void* InPtr ) noexcept
        {
            TEntityType* Root{ reinterpret_cast<TEntityType*>( InPtr ) };

            if constexpr( Flags.bDestructEntity )
            {
                GDestructNothrow<TEntityType>( Root );
            }

            auto& Store{ Root->GetEntityStore() };
            SKL_ASSERT( true == Store.IsValid() );
            Store.DeallocateEntity( Root );
        }
        
    private:
        template<typename TComponent, typename ...TOtherComponents>
        void InitializeComponents( IndexType InIndex ) noexcept
        {
            if constexpr( ComponentPaddingCondition<TComponent>::ShouldPadd() )
            {
                TEntityPtr& RootPtrRef{ Store.GetComponentPaddingAsT<TComponent, TEntityPtr>( InIndex ) };
                RootComponent& Root{ GetEntityRaw( InIndex ) };
                RootPtrRef = &Root;
            }

            if constexpr( sizeof...( TOtherComponents ) )
            {
                InitializeComponents<TOtherComponents...>( InIndex );
            }
        }

    private:
        MyStore        Store;      //!< entities store   
        MyIdStore      IdStore;    //!< ids store
        OnAllFreedTask OnAllFreed; //!< functor to dispatch when all entities are freed and the store is not active
    };

    template<typename TEntity, typename TRootData>
    TEntity& CastRootDataToEntity( TRootData& InRootData ) noexcept
    {
        return static_cast<TEntity&>( InRootData );
    }
    
    template<typename TEntity, typename TRootData>
    const TEntity& CastRootDataToEntity( const TRootData& InRootData ) noexcept
    {
        return static_cast<const TEntity&>( InRootData );
    }
}
