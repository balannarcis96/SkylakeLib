#include <gtest/gtest.h>

#include <SkylakeLib.h>

namespace ECSTests
{
    struct Component1
    {
        int a { 0 };
    };

    struct Component2
    {
        int b{ 0 };
        int c{ 1 };
    };

    using MyStaticStore = SKL::StaticSymmetricStore<uint16_t
                                      , 1024
                                      , Component1
                                      , Component2>;
                                      
    using MyStore = SKL::SymmetricStore<uint16_t
                                      , 1024
                                      , Component1
                                      , Component2>;
    
    TEST( ECSTests, ECS_StaticSymmetricStore )
    {
       MyStaticStore Store{};
       
       ASSERT_TRUE( 1024 == MyStaticStore::Traits::EntitiesCount );

       for( uint16_t i = 0; i < 1024; ++i )
       {
            ASSERT_TRUE( 1 == Store.GetComponent<Component2>( i ).c );
       }
    }
    
    TEST( ECSTests, ECS_SymmetricStore )
    {
       MyStore Store{};
       
       ASSERT_TRUE( true == Store.IsValid() );
       ASSERT_TRUE( 1024 == MyStore::Traits::EntitiesCount );

       for( uint16_t i = 0; i < 1024; ++i )
       {
            ASSERT_TRUE( 1 == Store.GetComponent<Component2>( i ).c );
       }
    }

    TEST( ECSTests, UIDStore_API )
    {
        SKL::UIDStore<uint16_t, 0, 1024> UIDStore{};

        int32_t Counter = 0;

        UIDStore.SetOnAllFreed( [&Counter]() noexcept -> void
        {
            ++Counter;
        } );

        ASSERT_TRUE( 0 == UIDStore.Allocate() );

        UIDStore.Activate();

        for( int32_t i = 0; i < 1024; ++i )
        {
            ASSERT_TRUE( 0 != UIDStore.Allocate() );
        }

        ASSERT_TRUE( 0 == UIDStore.Allocate() );

        for( uint16_t i = 1; i <= 1024; ++i )
        {
            UIDStore.Deallocate( i );
        }

        ASSERT_TRUE( 0 == Counter );

        for( int32_t i = 0; i < 1024; ++i )
        {
            ASSERT_TRUE( 0 != UIDStore.Allocate() );
        }

        ASSERT_TRUE( 0 == UIDStore.Allocate() );
        ASSERT_TRUE( 0 == Counter );

        UIDStore.Deactivate();

        ASSERT_TRUE( 0 == UIDStore.Allocate() );

        for( uint16_t i = 1; i <= 1024; ++i )
        {
            UIDStore.Deallocate( i );
        }

        ASSERT_TRUE( 1 == Counter );
    }
    
    TEST( ECSTests, UIDAllocationCache_API )
    {
        struct UIDAllocationCacheToIndexConvert
        {
            SKL_NODISCARD SKL_FORCEINLINE static size_t ConvertToIndex( uint16_t Id ) noexcept
            {
                // We assume TUIDType its uint16_t
                return static_cast<size_t>( Id );
            }
        };

        SKL::UIDAllocationCache<uint16_t, 0, 1024, UIDAllocationCacheToIndexConvert> UIDCache{};

        int32_t Counter = 0;

        UIDCache.SetOnAllFreed( [&Counter]() noexcept -> void
        {
            ++Counter;
        } );

        ASSERT_EQ( false, UIDCache.Allocate( 1 ) );

        UIDCache.Activate();

        for( int32_t i = 0; i < 1024; ++i )
        {
            ASSERT_EQ( true, UIDCache.Allocate( i + 1 ) );
        }

        ASSERT_EQ( false, UIDCache.Allocate( 1 ) );

        for( uint16_t i = 1; i <= 1024; ++i )
        {
            ASSERT_EQ( true, UIDCache.Deallocate( i ) );
        }

        ASSERT_EQ( 0, Counter );

        for( int32_t i = 0; i < 1024; ++i )
        {
            ASSERT_EQ( true, UIDCache.Allocate( i + 1 ) );
        }

        ASSERT_EQ( 0, Counter );

        UIDCache.Deactivate();

        ASSERT_EQ( false, UIDCache.Allocate( 1 ) );

        for( uint16_t i = 1; i <= 1024; ++i )
        {
            ASSERT_EQ( true, UIDCache.Deallocate( i ) );
        }

        ASSERT_EQ( 1, Counter );
    }

    TEST( ECSTests, EntityId_API )
    {
        struct PlayerIdDescription
        {
            uint16_t Value1;
            uint16_t Value2;
        };

        using PlayerId = SKL::TEntityId<PlayerIdDescription, false, false>;
        using AtomicPlayerId = SKL::TEntityId<PlayerIdDescription, false, true>;
        using ExtendedPlayerId = SKL::TEntityId<PlayerIdDescription, true, false>;
        using ExtendedAtomicPlayerId = SKL::TEntityId<PlayerIdDescription, true, true>;

        {
            PlayerId               pId{ 0 };
            AtomicPlayerId         pId2{ 0 };
            ExtendedPlayerId       pId3{ 0 };
            ExtendedAtomicPlayerId pId4{ 0 };

            ASSERT_TRUE( true  == pId.IsNone() );
            ASSERT_TRUE( false == pId.IsValid() );
        
            ASSERT_TRUE( true  == pId2.IsNone() );
            ASSERT_TRUE( false == pId2.IsValid() );
        
            ASSERT_TRUE( true  == pId3.IsNone() );
            ASSERT_TRUE( false == pId3.IsValid() );
        
            ASSERT_TRUE( true  == pId4.IsNone() );
            ASSERT_TRUE( false == pId4.IsValid() );
        }
        
        {
            PlayerId               pId { 1, PlayerId::CBasicIdMaxValue, PlayerIdDescription{ .Value1 = 32, .Value2 = 121 } };
            AtomicPlayerId         pId2{ 1, PlayerId::CBasicIdMaxValue, PlayerIdDescription{ .Value1 = 32, .Value2 = 121 } };
            ExtendedPlayerId       pId3{ 1, PlayerId::CExtendedIdMaxValue,  PlayerIdDescription{ .Value1 = 32, .Value2 = 121 } };
            ExtendedAtomicPlayerId pId4{ 1, PlayerId::CExtendedIdMaxValue,  PlayerIdDescription{ .Value1 = 32, .Value2 = 121 } };
                                                                        
            ASSERT_TRUE( false == pId.IsNone()   );
            ASSERT_TRUE( true  == pId.IsValid()  );
            ASSERT_TRUE( false == pId2.IsNone()  );
            ASSERT_TRUE( true  == pId2.IsValid() );
            ASSERT_TRUE( false == pId3.IsNone()  );
            ASSERT_TRUE( true  == pId3.IsValid() );
            ASSERT_TRUE( false == pId4.IsNone()  );
            ASSERT_TRUE( true  == pId4.IsValid() );
                              
            ASSERT_TRUE( 32  ==  pId.GetVariant().Value1 );
            ASSERT_TRUE( 121 ==  pId.GetVariant().Value2 );
            ASSERT_TRUE( 32  == pId2.GetVariant().Value1 );
            ASSERT_TRUE( 121 == pId2.GetVariant().Value2 );
            ASSERT_TRUE( 32  == pId3.GetVariant().Value1 );
            ASSERT_TRUE( 121 == pId3.GetVariant().Value2 );
            ASSERT_TRUE( 32  == pId4.GetVariant().Value1 );
            ASSERT_TRUE( 121 == pId4.GetVariant().Value2 );

            ASSERT_TRUE( 1 == pId.GetType() );
            ASSERT_TRUE( 1 == pId2.GetType() );
            ASSERT_TRUE( 1 == pId3.GetType() );
            ASSERT_TRUE( 1 == pId4.GetType() );
                              
            ASSERT_TRUE( PlayerId::CBasicIdMaxValue ==  pId.GetIndex() );
            ASSERT_TRUE( PlayerId::CBasicIdMaxValue == pId2.GetIndex() );
            ASSERT_TRUE( PlayerId::CExtendedIdMaxValue  == pId3.GetIndex() );
            ASSERT_TRUE( PlayerId::CExtendedIdMaxValue  == pId4.GetIndex() );
        }
    }

    TEST( ECSTests, EntityStore_ExtendedId_API )
    {   
        constexpr SKL::EntityStoreFlags MyEntityStoreFlags {
              .bExtendRootComponentToAsyncDispatchedObject = false
            , .bPaddRootEntityToMultipleOfCacheLine        = true
            , .bRequireOnDestroy                           = true
            , .bRequireOnCreate                            = true
            , .bUseCachedAllocationUIDStore                = false
            , .bDestructEntity                             = false
        };

        struct RootComponentData
        {
             RootComponentData() noexcept = default;
             ~RootComponentData() noexcept = default;

             uint32_t A { 55 };
             uint8_t Buffer[16];

             void OnDestroy() noexcept
             {
                
             }

             void OnCreate() noexcept
             {
             
             }
        };
        struct OtherComponent
        {
             OtherComponent() noexcept = default;
             ~OtherComponent() noexcept = default;

             int32_t A{ 123 };
        };

        constexpr SKL::TEntityType CMyEntityType = 2;

        using MyEntityId = SKL::TEntityId<uint32_t, true, true>;
        using MyEntityStore = SKL::EntityStore<CMyEntityType, MyEntityId, 1024, MyEntityStoreFlags, RootComponentData, OtherComponent>;
        using MyEntitySharedPtr = MyEntityStore::TEntitySharedPtr;

        const size_t aa{ MyEntityStore::CRootComponent_BytesLeftOnFirstCacheLine };
        const size_t bb{ MyEntityStore::CRootComponent_UsedBytesByUser };
        const size_t cc{ MyEntityStore::CRootComponent_UsedBytesByStore };
        const size_t dd{ MyEntityStore::CRootComponent_AvailableBytesForUserOnFirstCacheLine };
        printf( "\n\tBytesLeftOnFirstCacheLine:%llu\n\tUsedBytesByUser:%llu\n\tUsedBytesByStore:%llu\n\tAvailableBytesForUserOnFirstCacheLine:%llu\n", aa, bb, cc,dd );

        MyEntityStore Store{};

        int32_t Counter = 0;

        Store.SetOnAllFreed( [&Counter]() noexcept -> void 
        {
            ++Counter;
        } );

        {
            ASSERT_TRUE( true == Store.IsValid() );
            ASSERT_TRUE( SKL::RSuccess == Store.Initialize( ) );
            ASSERT_TRUE( false == Store.IsActive() );

            const auto InactiveAllocResult{ Store.AllocateEntity( 56 ) };
            ASSERT_TRUE( nullptr == InactiveAllocResult.get() );

            Store.Activate();
            ASSERT_TRUE( true == Store.IsActive() );

            MyEntitySharedPtr AllocResult{ Store.AllocateEntity( 141 ) };
            ASSERT_TRUE( nullptr != AllocResult.get() );

            ASSERT_TRUE( 1 == AllocResult->GetId().GetIndex() );
        }

        {
            auto AllocResult{ Store.AllocateEntity( 141 ) };
            ASSERT_TRUE( nullptr != AllocResult.get() );
            ASSERT_TRUE( 1 == AllocResult->GetId().GetIndex() );

             auto AllocResult2{ Store.AllocateEntity( 141 ) };
            ASSERT_TRUE( nullptr != AllocResult2.get() );
            ASSERT_TRUE( 2 == AllocResult2->GetId().GetIndex() );

            auto AllocResult3{ Store.AllocateEntity( 141 ) };
            ASSERT_TRUE( nullptr != AllocResult3.get() );
            ASSERT_TRUE( 3 == AllocResult3->GetId().GetIndex() );
        }

        {
            auto AllocResult{ Store.AllocateEntity( 141 ) };
            ASSERT_TRUE( nullptr != AllocResult.get() );

            ASSERT_TRUE( 141 == AllocResult->GetId().GetVariant() );
            ASSERT_TRUE( 55 == AllocResult->A );
            ASSERT_TRUE( 1 == AllocResult->GetId().GetIndex() );

            const auto CB{ reinterpret_cast<SKL::MemoryPolicy::ControlBlock*>( MyEntitySharedPtr::Static_GetBlockPtr( AllocResult.get() ) ) };
            ASSERT_TRUE( nullptr != CB );
            ASSERT_TRUE( 1 == CB->ReferenceCount.load( std::memory_order_relaxed ) );

            ASSERT_TRUE( 1 == MyEntitySharedPtr::Static_GetReferenceCount( AllocResult.get() ) );
        }

        {
            auto AllocResult{ Store.AllocateEntity( 142 ) };
            ASSERT_TRUE( nullptr != AllocResult.get() );

            ASSERT_TRUE( CMyEntityType == AllocResult->GetId().GetType() );
            ASSERT_TRUE( 142 == AllocResult->GetId().GetVariant() );
            ASSERT_TRUE( 55 == AllocResult->A );
            ASSERT_TRUE( 1 == AllocResult->GetId().GetIndex() );

            auto& RawEntity{ Store.GetEntityRaw( AllocResult->GetId() ) };
            ASSERT_TRUE( &RawEntity == AllocResult.get() );
        }
        
        {
            auto AllocResult{ Store.AllocateEntity( 141 ) };
            ASSERT_TRUE( nullptr != AllocResult.get() );

            auto& OComponent{ AllocResult->GetComponent<OtherComponent>() };
            ASSERT_TRUE( 123 == OComponent.A );

            auto& OComponent2{ Store.GetComponent<OtherComponent>( AllocResult->GetId() ) };
            ASSERT_TRUE( 123 == OComponent2.A );
        }

        ASSERT_TRUE( 0 == Counter );

        {
            auto AllocResult{ Store.AllocateEntity( 141 ) };
            ASSERT_TRUE( nullptr != AllocResult.get() );

            ASSERT_TRUE( 1 == AllocResult->GetId().GetIndex() );

            Store.Deactivate();
        }

        ASSERT_TRUE( 1 == Counter );

        {
            auto AllocResult{ Store.AllocateEntity( 141 ) };
            ASSERT_TRUE( nullptr == AllocResult.get() );
        }

        ASSERT_TRUE( 1 == Counter );
    }
    
    TEST( ECSTests, EntityStore_ExtendedId_API_CacheAllocations )
    {   
        constexpr SKL::EntityStoreFlags MyEntityStoreFlags {
              .bExtendRootComponentToAsyncDispatchedObject = true
            , .bPaddRootEntityToMultipleOfCacheLine        = true
            , .bRequireOnDestroy                           = true
            , .bRequireOnCreate                            = true
            , .bUseCachedAllocationUIDStore                = true
            , .bDestructEntity                             = false
        };

        struct RootComponentData
        {
             RootComponentData() noexcept = default;
             ~RootComponentData() noexcept = default;

             uint32_t A { 55 };
             uint8_t Buffer[16];

             void OnDestroy() noexcept
             {
                
             }

             void OnCreate() noexcept
             {
             
             }
        };
        struct OtherComponent
        {
             OtherComponent() noexcept = default;
             ~OtherComponent() noexcept = default;

             int32_t A{ 123 };
        };

        constexpr SKL::TEntityType CMyEntityType = 2;

        using MyEntityId = SKL::TEntityId<uint32_t, true, true>;
        using MyEntityStore = SKL::EntityStore<CMyEntityType, MyEntityId, 1024, MyEntityStoreFlags, RootComponentData, OtherComponent>;
        using MyEntitySharedPtr = MyEntityStore::TEntitySharedPtr;

        const size_t aa{ MyEntityStore::CRootComponent_BytesLeftOnFirstCacheLine };
        const size_t bb{ MyEntityStore::CRootComponent_UsedBytesByUser };
        const size_t cc{ MyEntityStore::CRootComponent_UsedBytesByStore };
        const size_t dd{ MyEntityStore::CRootComponent_AvailableBytesForUserOnFirstCacheLine };
        printf( "\n\tBytesLeftOnFirstCacheLine:%llu\n\tUsedBytesByUser:%llu\n\tUsedBytesByStore:%llu\n\tAvailableBytesForUserOnFirstCacheLine:%llu\n", aa, bb, cc,dd );

        MyEntityStore Store{};

        int32_t Counter = 0;

        Store.SetOnAllFreed( [&Counter]() noexcept -> void 
        {
            ++Counter;
        } );

        {
            ASSERT_TRUE( true == Store.IsValid() );
            ASSERT_TRUE( SKL::RSuccess == Store.Initialize( ) );
            ASSERT_TRUE( false == Store.IsActive() );

            const MyEntitySharedPtr InactiveAllocResult{ Store.AllocateSpecificEntity( 1, 56 ) };
            ASSERT_TRUE( nullptr == InactiveAllocResult.get() );

            Store.Activate();
            ASSERT_TRUE( true == Store.IsActive() );

            auto AllocResult{ Store.AllocateSpecificEntity( 1, 141 ) };
            ASSERT_TRUE( nullptr != AllocResult.get() );

            ASSERT_TRUE( 1 == AllocResult->GetId().GetIndex() );
        }
        
        {
            auto AllocResult{ Store.AllocateSpecificEntity( 1, 141 ) };
            ASSERT_TRUE( nullptr != AllocResult.get() );
            ASSERT_TRUE( 1 == AllocResult->GetId().GetIndex() );
            
            auto AllocResult_again{ Store.AllocateSpecificEntity( 1, 141 ) };
            ASSERT_TRUE( nullptr == AllocResult_again.get() );

            AllocResult.reset();
            
            auto AllocResult_again2{ Store.AllocateSpecificEntity( 1, 141 ) };
            ASSERT_TRUE( nullptr != AllocResult_again2.get() );
            ASSERT_TRUE( 1 == AllocResult_again2->GetId().GetIndex() );

            auto AllocResult2{ Store.AllocateSpecificEntity( 2, 141 ) };
            ASSERT_TRUE( nullptr != AllocResult2.get() );
            ASSERT_TRUE( 2 == AllocResult2->GetId().GetIndex() );

            auto AllocResult3{ Store.AllocateSpecificEntity( 3, 141 ) };
            ASSERT_TRUE( nullptr != AllocResult3.get() );
            ASSERT_TRUE( 3 == AllocResult3->GetId().GetIndex() );
        }
        
        {
            auto AllocResult{ Store.AllocateSpecificEntity( 1, 141 ) };
            ASSERT_TRUE( nullptr != AllocResult.get() );
            ASSERT_TRUE( 1 == AllocResult->GetId().GetIndex() );

            EXPECT_EQ( 141, AllocResult->GetId().GetVariant() );
            EXPECT_EQ( 55, AllocResult->A );
            EXPECT_EQ( 1, AllocResult->GetId().GetIndex() );

            const auto CB{ reinterpret_cast<SKL::MemoryPolicy::ControlBlock*>( MyEntitySharedPtr::Static_GetBlockPtr( AllocResult.get() ) ) };
            ASSERT_TRUE( nullptr != CB );
            ASSERT_TRUE( 1 == CB->ReferenceCount.load( std::memory_order_relaxed ) );

            ASSERT_TRUE( 1 == MyEntitySharedPtr::Static_GetReferenceCount( AllocResult.get() ) );
        }
        
        {
            auto AllocResult{ Store.AllocateSpecificEntity( 1, 141 ) };
            ASSERT_TRUE( nullptr != AllocResult.get() );

            auto& OComponent{ AllocResult->GetComponent<OtherComponent>() };
            ASSERT_TRUE( 123 == OComponent.A );

            auto& OComponent2{ Store.GetComponent<OtherComponent>( AllocResult->GetId() ) };
            ASSERT_TRUE( 123 == OComponent2.A );
        }

        ASSERT_TRUE( 0 == Counter );
        
        {
            auto AllocResult{ Store.AllocateSpecificEntity( 1, 141 ) };
            ASSERT_TRUE( nullptr != AllocResult.get() );

            ASSERT_TRUE( 1 == AllocResult->GetId().GetIndex() );

            Store.Deactivate();
        }

        ASSERT_TRUE( 1 == Counter );

        {
            auto AllocResult{ Store.AllocateSpecificEntity( 1, 141 ) };
            ASSERT_TRUE( nullptr == AllocResult.get() );
        }

        ASSERT_TRUE( 1 == Counter );
    }

    
    TEST( ECSTests, EntityStore_ExtendedId_DestrucEntity )
    {   
        constexpr SKL::EntityStoreFlags MyEntityStoreFlags {
              .bExtendRootComponentToAsyncDispatchedObject = true
            , .bPaddRootEntityToMultipleOfCacheLine        = true
            , .bRequireOnDestroy                           = true
            , .bRequireOnCreate                            = true
            , .bUseCachedAllocationUIDStore                = false
            , .bDestructEntity                             = true
        };

        struct RootComponentData
        {
             RootComponentData() noexcept = default;
             ~RootComponentData() noexcept = default;

             uint32_t A { 55 };
             uint8_t Buffer[16];

             void OnDestroy() noexcept
             {
                
             }

             void OnCreate() noexcept
             {
             
             }
        };
        struct OtherComponent
        {
             OtherComponent() noexcept = default;
             ~OtherComponent() noexcept = default;

             int32_t A{ 123 };
        };

        constexpr SKL::TEntityType CMyEntityType = 2;

        using MyEntityId = SKL::TEntityId<uint32_t, true, true>;
        using MyEntityStore = SKL::EntityStore<CMyEntityType, MyEntityId, 1024, MyEntityStoreFlags, RootComponentData, OtherComponent>;
        using MyEntitySharedPtr = MyEntityStore::TEntitySharedPtr;

        const size_t aa{ MyEntityStore::CRootComponent_BytesLeftOnFirstCacheLine };
        const size_t bb{ MyEntityStore::CRootComponent_UsedBytesByUser };
        const size_t cc{ MyEntityStore::CRootComponent_UsedBytesByStore };
        const size_t dd{ MyEntityStore::CRootComponent_AvailableBytesForUserOnFirstCacheLine };
        printf( "\n\tBytesLeftOnFirstCacheLine:%llu\n\tUsedBytesByUser:%llu\n\tUsedBytesByStore:%llu\n\tAvailableBytesForUserOnFirstCacheLine:%llu\n", aa, bb, cc,dd );

        MyEntityStore Store{};

        int32_t Counter = 0;

        Store.SetOnAllFreed( [&Counter]() noexcept -> void 
        {
            ++Counter;
        } );

        {
            ASSERT_TRUE( true == Store.IsValid() );
            ASSERT_TRUE( SKL::RSuccess == Store.Initialize( ) );
            ASSERT_TRUE( false == Store.IsActive() );

            const auto InactiveAllocResult{ Store.AllocateEntity( 56 ) };
            ASSERT_TRUE( nullptr == InactiveAllocResult.get() );

            Store.Activate();
            ASSERT_TRUE( true == Store.IsActive() );

            MyEntitySharedPtr AllocResult{ Store.AllocateEntity( 141 ) };
            ASSERT_TRUE( nullptr != AllocResult.get() );

            ASSERT_TRUE( 1 == AllocResult->GetId().GetIndex() );
        }

        {
            auto AllocResult{ Store.AllocateEntity( 141 ) };
            ASSERT_TRUE( nullptr != AllocResult.get() );
            ASSERT_TRUE( 1 == AllocResult->GetId().GetIndex() );

             auto AllocResult2{ Store.AllocateEntity( 141 ) };
            ASSERT_TRUE( nullptr != AllocResult2.get() );
            ASSERT_TRUE( 2 == AllocResult2->GetId().GetIndex() );

            auto AllocResult3{ Store.AllocateEntity( 141 ) };
            ASSERT_TRUE( nullptr != AllocResult3.get() );
            ASSERT_TRUE( 3 == AllocResult3->GetId().GetIndex() );
        }

        {
            auto AllocResult{ Store.AllocateEntity( 141 ) };
            ASSERT_TRUE( nullptr != AllocResult.get() );

            ASSERT_TRUE( 141 == AllocResult->GetId().GetVariant() );
            ASSERT_TRUE( 55 == AllocResult->A );
            ASSERT_TRUE( 1 == AllocResult->GetId().GetIndex() );

            const auto CB{ reinterpret_cast<SKL::MemoryPolicy::ControlBlock*>( MyEntitySharedPtr::Static_GetBlockPtr( AllocResult.get() ) ) };
            ASSERT_TRUE( nullptr != CB );
            ASSERT_TRUE( 1 == CB->ReferenceCount.load( std::memory_order_relaxed ) );

            ASSERT_TRUE( 1 == MyEntitySharedPtr::Static_GetReferenceCount( AllocResult.get() ) );
        }

        {
            auto AllocResult{ Store.AllocateEntity( 142 ) };
            ASSERT_TRUE( nullptr != AllocResult.get() );

            ASSERT_TRUE( CMyEntityType == AllocResult->GetId().GetType() );
            ASSERT_TRUE( 142 == AllocResult->GetId().GetVariant() );
            ASSERT_TRUE( 55 == AllocResult->A );
            ASSERT_TRUE( 1 == AllocResult->GetId().GetIndex() );

            auto& RawEntity{ Store.GetEntityRaw( AllocResult->GetId() ) };
            ASSERT_TRUE( &RawEntity == AllocResult.get() );
        }
        
        {
            auto AllocResult{ Store.AllocateEntity( 141 ) };
            ASSERT_TRUE( nullptr != AllocResult.get() );

            auto& OComponent{ AllocResult->GetComponent<OtherComponent>() };
            ASSERT_TRUE( 123 == OComponent.A );

            auto& OComponent2{ Store.GetComponent<OtherComponent>( AllocResult->GetId() ) };
            ASSERT_TRUE( 123 == OComponent2.A );
        }

        ASSERT_TRUE( 0 == Counter );

        {
            auto AllocResult{ Store.AllocateEntity( 141 ) };
            ASSERT_TRUE( nullptr != AllocResult.get() );

            ASSERT_TRUE( 1 == AllocResult->GetId().GetIndex() );

            Store.Deactivate();
        }

        ASSERT_TRUE( 1 == Counter );

        {
            auto AllocResult{ Store.AllocateEntity( 141 ) };
            ASSERT_TRUE( nullptr == AllocResult.get() );
        }

        ASSERT_TRUE( 1 == Counter );
    }
    
}

int main( int argc, char** argv )
{
    testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS( );
}