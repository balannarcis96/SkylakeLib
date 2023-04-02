#include <gtest/gtest.h>

#include <SkylakeLib.h>

namespace MManagementTestsSuite
{
    struct MyType
    {
        MyType() noexcept : a { nullptr }{}
        MyType( int* a ) noexcept : a { a } {} 
        ~MyType() noexcept { 
            if( nullptr != a )
            {
                *a = 23;
            }
        }

        int *a;
    };
    
    TEST( MManagementTestsSuite, LocalObjectPool_Atomic_Test )
    {
        using PoolType = SKL::LocalObjectPool<MyType, 128, false, false, false, false>;

        PoolType MyPool{};

        EXPECT_EQ( SKL::RSuccess, MyPool.Preallocate() );

        SKL_IFMEMORYSTATS( const uint64_t AllocationsBefore = MyPool.GetTotalAllocations() );
        SKL_IFMEMORYSTATS( const uint64_t DeallocationsBefore = MyPool.GetTotalDeallocations() );
        SKL_IFMEMORYSTATS( const uint64_t OSAllocationsBefore = MyPool.GetTotalOSAllocations() );
        SKL_IFMEMORYSTATS( const uint64_t OSDeallocationsBefore = MyPool.GetTotalOSDeallocations() );
        SKL_IFMEMORYSTATS( EXPECT_EQ( 0, AllocationsBefore ) );
        SKL_IFMEMORYSTATS( EXPECT_EQ( 0, DeallocationsBefore ) );
        SKL_IFMEMORYSTATS( EXPECT_EQ( 0, OSAllocationsBefore ) );
        SKL_IFMEMORYSTATS( EXPECT_EQ( 0, OSDeallocationsBefore ) );

        for( int32_t i = 0 ; i < 4096; ++i )
        {
            MyType* AllocResult{ MyPool.Allocate() };
            EXPECT_NE( nullptr, AllocResult );
            MyPool.Deallocate( AllocResult );
        }

        SKL_IFMEMORYSTATS( const uint64_t AllocationsAfter = MyPool.GetTotalAllocations() );
        SKL_IFMEMORYSTATS( const uint64_t DeallocationsAfter = MyPool.GetTotalDeallocations() );
        SKL_IFMEMORYSTATS( const uint64_t OSAllocationsAfter = MyPool.GetTotalOSAllocations() );
        SKL_IFMEMORYSTATS( const uint64_t OSDeallocationsAfter = MyPool.GetTotalOSDeallocations() );
        SKL_IFMEMORYSTATS( EXPECT_EQ( 4096, AllocationsAfter ) );
        SKL_IFMEMORYSTATS( EXPECT_EQ( 4096, DeallocationsAfter ) );
        SKL_IFMEMORYSTATS( EXPECT_EQ( 0, OSAllocationsAfter ) );
        SKL_IFMEMORYSTATS( EXPECT_EQ( 0, OSDeallocationsAfter ) );
    }
    
    TEST( MManagementTestsSuite, LocalObjectPool_Atomic_Test_2 )
    {
        using PoolType = SKL::LocalObjectPool<MyType, 128, false, false, false, false>;

        PoolType MyPool{};

        EXPECT_EQ( SKL::RSuccess, MyPool.Preallocate() );

        SKL_IFMEMORYSTATS( const uint64_t AllocationsBefore = MyPool.GetTotalAllocations() );
        SKL_IFMEMORYSTATS( const uint64_t DeallocationsBefore = MyPool.GetTotalDeallocations() );
        SKL_IFMEMORYSTATS( const uint64_t OSAllocationsBefore = MyPool.GetTotalOSAllocations() );
        SKL_IFMEMORYSTATS( const uint64_t OSDeallocationsBefore = MyPool.GetTotalOSDeallocations() );
        SKL_IFMEMORYSTATS( EXPECT_EQ( 0, AllocationsBefore ) );
        SKL_IFMEMORYSTATS( EXPECT_EQ( 0, DeallocationsBefore ) );
        SKL_IFMEMORYSTATS( EXPECT_EQ( 0, OSAllocationsBefore ) );
        SKL_IFMEMORYSTATS( EXPECT_EQ( 0, OSDeallocationsBefore ) );

        std::jthread Threads[4];

        constexpr uint64_t AllocationsCount = 64;

        for( auto& t : Threads )
        {
            t = std::jthread( [&MyPool]() noexcept
            {
                for( uint64_t i = 0 ; i < AllocationsCount; ++i )
                {
                    MyType* AllocResult{ MyPool.Allocate() };
                    EXPECT_NE( nullptr, AllocResult );
                    MyPool.Deallocate( AllocResult );
                }
            } );
        }

        for( auto& t : Threads )
            t.join();

        SKL_IFMEMORYSTATS( const uint64_t AllocationsAfter = MyPool.GetTotalAllocations() );
        SKL_IFMEMORYSTATS( const uint64_t DeallocationsAfter = MyPool.GetTotalDeallocations() );
        SKL_IFMEMORYSTATS( const uint64_t OSAllocationsAfter = MyPool.GetTotalOSAllocations() );
        SKL_IFMEMORYSTATS( const uint64_t OSDeallocationsAfter = MyPool.GetTotalOSDeallocations() );
        SKL_IFMEMORYSTATS( EXPECT_EQ( AllocationsCount * 4, AllocationsAfter ) );
        SKL_IFMEMORYSTATS( EXPECT_EQ( AllocationsCount * 4, DeallocationsAfter ) );
        SKL_IFMEMORYSTATS( EXPECT_EQ( 0, OSAllocationsAfter ) );
        SKL_IFMEMORYSTATS( EXPECT_EQ( 0, OSDeallocationsAfter ) );
    }
    
    TEST( MManagementTestsSuite, LocalObjectPool_SpinLock_Test )
    {
        using PoolType = SKL::LocalObjectPool<MyType, 128, false, true, false, false>;

        PoolType MyPool{};

        EXPECT_EQ( SKL::RSuccess, MyPool.Preallocate() );

        SKL_IFMEMORYSTATS( const uint64_t AllocationsBefore = MyPool.GetTotalAllocations() );
        SKL_IFMEMORYSTATS( const uint64_t DeallocationsBefore = MyPool.GetTotalDeallocations() );
        SKL_IFMEMORYSTATS( const uint64_t OSAllocationsBefore = MyPool.GetTotalOSAllocations() );
        SKL_IFMEMORYSTATS( const uint64_t OSDeallocationsBefore = MyPool.GetTotalOSDeallocations() );
        SKL_IFMEMORYSTATS( EXPECT_EQ( 0, AllocationsBefore ) );
        SKL_IFMEMORYSTATS( EXPECT_EQ( 0, DeallocationsBefore ) );
        SKL_IFMEMORYSTATS( EXPECT_EQ( 0, OSAllocationsBefore ) );
        SKL_IFMEMORYSTATS( EXPECT_EQ( 0, OSDeallocationsBefore ) );

        std::jthread Threads[4];

        constexpr uint64_t AllocationsCount = 4096;

        for( auto& t : Threads )
        {
            t = std::jthread( [&MyPool]() noexcept
            {
                for( uint64_t i = 0 ; i < AllocationsCount; ++i )
                {
                    MyType* AllocResult{ MyPool.Allocate() };
                    EXPECT_NE( nullptr, AllocResult );
                    MyPool.Deallocate( AllocResult );
                }
            } );
        }

        for( auto& t : Threads )
            t.join();

        SKL_IFMEMORYSTATS( const uint64_t AllocationsAfter = MyPool.GetTotalAllocations() );
        SKL_IFMEMORYSTATS( const uint64_t DeallocationsAfter = MyPool.GetTotalDeallocations() );
        SKL_IFMEMORYSTATS( const uint64_t OSAllocationsAfter = MyPool.GetTotalOSAllocations() );
        SKL_IFMEMORYSTATS( const uint64_t OSDeallocationsAfter = MyPool.GetTotalOSDeallocations() );
        SKL_IFMEMORYSTATS( EXPECT_EQ( AllocationsCount * 4, AllocationsAfter ) );
        SKL_IFMEMORYSTATS( EXPECT_EQ( AllocationsCount * 4, DeallocationsAfter ) );
        SKL_IFMEMORYSTATS( EXPECT_EQ( 0, OSAllocationsAfter ) );
        SKL_IFMEMORYSTATS( EXPECT_EQ( 0, OSDeallocationsAfter ) );
    }


    TEST( MManagementTestsSuite, Init_Test_Case___ )
    {
        SKL::KPIContext::Create();

        //SKL::SkylakeGlobalMemoryManager::Preallocate();
        //SKL::SkylakeGlobalMemoryManager::LogStatistics();
        
        SKL_IFMEMORYSTATS( const uint64_t AllocationsBefore = SKL::SkylakeGlobalMemoryManager::TotalAllocations );
        SKL_IFMEMORYSTATS( const uint64_t DeallocationsBefore = SKL::SkylakeGlobalMemoryManager::TotalDeallocations );

        auto AllocResult = SKL::SkylakeGlobalMemoryManager::Allocate<24>();
        ASSERT_TRUE( true == AllocResult.IsValid() );

        SKL::SkylakeGlobalMemoryManager::Deallocate( AllocResult );
        ASSERT_TRUE( false == AllocResult.IsValid() );

        SKL_IFMEMORYSTATS( const uint64_t AllocationsAfter = SKL::SkylakeGlobalMemoryManager::TotalAllocations );
        SKL_IFMEMORYSTATS( const uint64_t DeallocationsAfter = SKL::SkylakeGlobalMemoryManager::TotalDeallocations );
        SKL_IFMEMORYSTATS( ASSERT_TRUE( AllocationsAfter - AllocationsBefore == 1 ) );
        SKL_IFMEMORYSTATS( ASSERT_TRUE( DeallocationsAfter - DeallocationsBefore == 1 ) );

        SKL::KPIContext::Destroy();
    }

    TEST( MManagementTestsSuite, MakeUnique_Test_Case )
    {
        SKL::KPIContext::Create();
        int b = 5;
    
        SKL_IFMEMORYSTATS( const uint64_t AllocationsBefore = SKL::SkylakeGlobalMemoryManager::TotalAllocations );
        SKL_IFMEMORYSTATS( const uint64_t DeallocationsBefore = SKL::SkylakeGlobalMemoryManager::TotalDeallocations );

        {
            auto UniqueItem = SKL::MakeUnique<MyType>( &b );
            ASSERT_TRUE( nullptr != UniqueItem.get() );
            ASSERT_TRUE( 5 == *UniqueItem->a );
        }

        SKL_IFMEMORYSTATS( const uint64_t AllocationsAfter = SKL::SkylakeGlobalMemoryManager::TotalAllocations );
        SKL_IFMEMORYSTATS( const uint64_t DeallocationsAfter = SKL::SkylakeGlobalMemoryManager::TotalDeallocations );
        SKL_IFMEMORYSTATS( ASSERT_TRUE( AllocationsAfter - AllocationsBefore == 1 ) );
        SKL_IFMEMORYSTATS( ASSERT_TRUE( DeallocationsAfter - DeallocationsBefore == 1 ) );

        ASSERT_TRUE( 23 == b );

        SKL::KPIContext::Destroy();
    }

    TEST( MManagementTestsSuite, MakeUniqueNoDeconstructAndConstruct )
    {
        SKL::KPIContext::Create();
        int b = 5;
    
        SKL_IFMEMORYSTATS( const uint64_t AllocationsBefore = SKL::SkylakeGlobalMemoryManager::TotalAllocations );
        SKL_IFMEMORYSTATS( const uint64_t DeallocationsBefore = SKL::SkylakeGlobalMemoryManager::TotalDeallocations );

        {
            auto UniqueItem = SKL::MakeUniqueNoDeconstruct<MyType>();
            ASSERT_TRUE( nullptr != UniqueItem.get() );
            //ASSERT_TRUE( 5 == *UniqueItem->a );
        }

        SKL_IFMEMORYSTATS( const uint64_t AllocationsAfter = SKL::SkylakeGlobalMemoryManager::TotalAllocations );
        SKL_IFMEMORYSTATS( const uint64_t DeallocationsAfter = SKL::SkylakeGlobalMemoryManager::TotalDeallocations );
        SKL_IFMEMORYSTATS( ASSERT_TRUE( AllocationsAfter - AllocationsBefore == 1 ) );
        SKL_IFMEMORYSTATS( ASSERT_TRUE( DeallocationsAfter - DeallocationsBefore == 1 ) );

        ASSERT_TRUE( 5 == b );

        SKL::KPIContext::Destroy();
    }

    TEST( MManagementTestsSuite, MakeUniqueArray_Test_Case )
    {
        SKL::KPIContext::Create();
        SKL_IFMEMORYSTATS( const uint64_t AllocationsBefore = SKL::SkylakeGlobalMemoryManager::TotalAllocations );
        SKL_IFMEMORYSTATS( const uint64_t DeallocationsBefore = SKL::SkylakeGlobalMemoryManager::TotalDeallocations );

        {
            auto UniqueItem = SKL::MakeUniqueArray<MyType>( 32 );
            ASSERT_TRUE( nullptr != UniqueItem.get() );

            for( uint32_t i = 0; i < 32; ++i )
            {
                ASSERT_TRUE( nullptr == UniqueItem[ i ].a );
            }
        }

        SKL_IFMEMORYSTATS( const uint64_t AllocationsAfter = SKL::SkylakeGlobalMemoryManager::TotalAllocations );
        SKL_IFMEMORYSTATS( const uint64_t DeallocationsAfter = SKL::SkylakeGlobalMemoryManager::TotalDeallocations );
        SKL_IFMEMORYSTATS( ASSERT_TRUE( AllocationsAfter - AllocationsBefore == 1 ) );
        SKL_IFMEMORYSTATS( ASSERT_TRUE( DeallocationsAfter - DeallocationsBefore == 1 ) );

        SKL::KPIContext::Destroy();
    }

    TEST( MManagementTestsSuite, MakeUniqueArrayWithNoDestructAndConstruct )
    {
        SKL::KPIContext::Create();
        SKL_IFMEMORYSTATS( const uint64_t AllocationsBefore = SKL::SkylakeGlobalMemoryManager::TotalAllocations );
        SKL_IFMEMORYSTATS( const uint64_t DeallocationsBefore = SKL::SkylakeGlobalMemoryManager::TotalDeallocations );

        {
            auto UniqueItem = SKL::MakeUniqueArrayWithNoDestruct<MyType>( 32 );
            ASSERT_TRUE( nullptr != UniqueItem.get() );

            /*for( uint32_t i = 0; i < 32; ++i )
            {
                ASSERT_TRUE( nullptr == UniqueItem[ i ].a );
            }*/
        }

        SKL_IFMEMORYSTATS( const uint64_t AllocationsAfter = SKL::SkylakeGlobalMemoryManager::TotalAllocations );
        SKL_IFMEMORYSTATS( const uint64_t DeallocationsAfter = SKL::SkylakeGlobalMemoryManager::TotalDeallocations );
        SKL_IFMEMORYSTATS( ASSERT_TRUE( AllocationsAfter - AllocationsBefore == 1 ) );
        SKL_IFMEMORYSTATS( ASSERT_TRUE( DeallocationsAfter - DeallocationsBefore == 1 ) );

        SKL::KPIContext::Destroy();
    }

    TEST( MManagementTestsSuite, MakeUniqueArrayWithNoDestructButConstruct )
    {
        SKL::KPIContext::Create();
        SKL_IFMEMORYSTATS( const uint64_t AllocationsBefore = SKL::SkylakeGlobalMemoryManager::TotalAllocations );
        SKL_IFMEMORYSTATS( const uint64_t DeallocationsBefore = SKL::SkylakeGlobalMemoryManager::TotalDeallocations );

        {
            auto UniqueItem = SKL::MakeUniqueArrayWithNoDestruct<MyType, true>( 32 );
            ASSERT_TRUE( nullptr != UniqueItem.get() );

            for( uint32_t i = 0; i < 32; ++i )
            {
                ASSERT_TRUE( nullptr == UniqueItem[ i ].a );
            }
        }

        SKL_IFMEMORYSTATS( const uint64_t AllocationsAfter = SKL::SkylakeGlobalMemoryManager::TotalAllocations );
        SKL_IFMEMORYSTATS( const uint64_t DeallocationsAfter = SKL::SkylakeGlobalMemoryManager::TotalDeallocations );
        SKL_IFMEMORYSTATS( ASSERT_TRUE( AllocationsAfter - AllocationsBefore == 1 ) );
        SKL_IFMEMORYSTATS( ASSERT_TRUE( DeallocationsAfter - DeallocationsBefore == 1 ) );

        SKL::KPIContext::Destroy();
    }

    TEST( MManagementTestsSuite, MakeShared_Test_Case )
    {
        SKL::KPIContext::Create();
        int b = 5;
     
        SKL_IFMEMORYSTATS( const uint64_t AllocationsBefore = SKL::SkylakeGlobalMemoryManager::TotalAllocations );
        SKL_IFMEMORYSTATS( const uint64_t DeallocationsBefore = SKL::SkylakeGlobalMemoryManager::TotalDeallocations );

        {
            auto SharedItem = SKL::MakeShared<MyType>( &b );
            ASSERT_TRUE( nullptr != SharedItem.get() );
            ASSERT_TRUE( true == static_cast<bool>( SharedItem ) );
            ASSERT_TRUE( 1 == SharedItem.use_count() );
            ASSERT_TRUE( 5 == *SharedItem->a );

            { //copy
                SKL::TSharedPtr<MyType> SharedItemRef2 { SharedItem };
                ASSERT_TRUE( 2 == SharedItem.use_count() );
                ASSERT_TRUE( true == static_cast<bool>( SharedItemRef2 ) );

                { //move
                    SKL::TSharedPtr<MyType> SharedItemRef2_Move { std::move( SharedItemRef2 ) };
                    ASSERT_TRUE( 2 == SharedItem.use_count() );
                    ASSERT_TRUE( false == static_cast<bool>( SharedItemRef2 ) );
                    ASSERT_TRUE( true == static_cast<bool>( SharedItemRef2_Move ) );
                }
            }
    
            ASSERT_TRUE( 1 == SharedItem.use_count() );
        }

        SKL_IFMEMORYSTATS( const uint64_t AllocationsAfter = SKL::SkylakeGlobalMemoryManager::TotalAllocations );
        SKL_IFMEMORYSTATS( const uint64_t DeallocationsAfter = SKL::SkylakeGlobalMemoryManager::TotalDeallocations );
        SKL_IFMEMORYSTATS( ASSERT_TRUE( AllocationsAfter - AllocationsBefore == 1 ) );
        SKL_IFMEMORYSTATS( ASSERT_TRUE( DeallocationsAfter - DeallocationsBefore == 1 ) );

        ASSERT_TRUE( 23 == b );

        SKL::KPIContext::Destroy();
    }

    TEST( MManagementTestsSuite, MakeSharedNoDestructAndConstruct )
    {
        SKL::KPIContext::Create();
        int b = 5;
     
        SKL_IFMEMORYSTATS( const uint64_t AllocationsBefore = SKL::SkylakeGlobalMemoryManager::TotalAllocations );
        SKL_IFMEMORYSTATS( const uint64_t DeallocationsBefore = SKL::SkylakeGlobalMemoryManager::TotalDeallocations );

        {
            auto SharedItem = SKL::MakeSharedNoDestruct<MyType>();
            ASSERT_TRUE( nullptr != SharedItem.get() );
            ASSERT_TRUE( true == static_cast<bool>( SharedItem ) );
            ASSERT_TRUE( 1 == SharedItem.use_count() );
            //ASSERT_TRUE( nullptr == SharedItem->a );

            { //copy
                SKL::TSharedPtrNoDestruct<MyType> SharedItemRef2 { SharedItem };
                ASSERT_TRUE( 2 == SharedItem.use_count() );
                ASSERT_TRUE( true == static_cast<bool>( SharedItemRef2 ) );

                { //move
                    SKL::TSharedPtrNoDestruct<MyType> SharedItemRef2_Move { std::move( SharedItemRef2 ) };
                    ASSERT_TRUE( 2 == SharedItem.use_count() );
                    ASSERT_TRUE( false == static_cast<bool>( SharedItemRef2 ) );
                    ASSERT_TRUE( true == static_cast<bool>( SharedItemRef2_Move ) );
                }
            }
    
            ASSERT_TRUE( 1 == SharedItem.use_count() );
        }

        SKL_IFMEMORYSTATS( const uint64_t AllocationsAfter = SKL::SkylakeGlobalMemoryManager::TotalAllocations );
        SKL_IFMEMORYSTATS( const uint64_t DeallocationsAfter = SKL::SkylakeGlobalMemoryManager::TotalDeallocations );
        SKL_IFMEMORYSTATS( ASSERT_TRUE( AllocationsAfter - AllocationsBefore == 1 ) );
        SKL_IFMEMORYSTATS( ASSERT_TRUE( DeallocationsAfter - DeallocationsBefore == 1 ) );

        ASSERT_TRUE( 5 == b );

        SKL::KPIContext::Destroy();
    }

    TEST( MManagementTestsSuite, MakeSharedNoDestructButWithConstruct )
    {
        SKL::KPIContext::Create();
        int b = 5;
     
        SKL_IFMEMORYSTATS( const uint64_t AllocationsBefore = SKL::SkylakeGlobalMemoryManager::TotalAllocations );
        SKL_IFMEMORYSTATS( const uint64_t DeallocationsBefore = SKL::SkylakeGlobalMemoryManager::TotalDeallocations );

        {
            auto SharedItem = SKL::MakeSharedNoDestruct<MyType, true>( &b );
            ASSERT_TRUE( nullptr != SharedItem.get() );
            ASSERT_TRUE( true == static_cast<bool>( SharedItem ) );
            ASSERT_TRUE( 1 == SharedItem.use_count() );
            ASSERT_TRUE( &b == SharedItem->a );

            { //copy
                SKL::TSharedPtrNoDestruct<MyType> SharedItemRef2 { SharedItem };
                ASSERT_TRUE( 2 == SharedItem.use_count() );
                ASSERT_TRUE( true == static_cast<bool>( SharedItemRef2 ) );

                { //move
                    SKL::TSharedPtrNoDestruct<MyType> SharedItemRef2_Move { std::move( SharedItemRef2 ) };
                    ASSERT_TRUE( 2 == SharedItem.use_count() );
                    ASSERT_TRUE( false == static_cast<bool>( SharedItemRef2 ) );
                    ASSERT_TRUE( true == static_cast<bool>( SharedItemRef2_Move ) );
                }
            }
    
            ASSERT_TRUE( 1 == SharedItem.use_count() );
        }

        SKL_IFMEMORYSTATS( const uint64_t AllocationsAfter = SKL::SkylakeGlobalMemoryManager::TotalAllocations );
        SKL_IFMEMORYSTATS( const uint64_t DeallocationsAfter = SKL::SkylakeGlobalMemoryManager::TotalDeallocations );
        SKL_IFMEMORYSTATS( ASSERT_TRUE( AllocationsAfter - AllocationsBefore == 1 ) );
        SKL_IFMEMORYSTATS( ASSERT_TRUE( DeallocationsAfter - DeallocationsBefore == 1 ) );

        ASSERT_TRUE( 5 == b ); // the destructor was not called

        SKL::KPIContext::Destroy();
    }

    TEST( MManagementTestsSuite, MakeSharedArray_Test_Case )
    {
        SKL::KPIContext::Create();
        SKL_IFMEMORYSTATS( const uint64_t AllocationsBefore = SKL::SkylakeGlobalMemoryManager::TotalAllocations );
        SKL_IFMEMORYSTATS( const uint64_t DeallocationsBefore = SKL::SkylakeGlobalMemoryManager::TotalDeallocations );

        {
            auto SharedArray = SKL::MakeSharedArray<MyType>( 32 );
            ASSERT_TRUE( nullptr != SharedArray.get() );

            for( uint32_t i = 0; i < 32; ++i )
            {
                ASSERT_TRUE( nullptr == SharedArray[ i ].a );
            }
        }

        SKL_IFMEMORYSTATS( const uint64_t AllocationsAfter = SKL::SkylakeGlobalMemoryManager::TotalAllocations );
        SKL_IFMEMORYSTATS( const uint64_t DeallocationsAfter = SKL::SkylakeGlobalMemoryManager::TotalDeallocations );
        SKL_IFMEMORYSTATS( ASSERT_TRUE( AllocationsAfter - AllocationsBefore == 1 ) );
        SKL_IFMEMORYSTATS( ASSERT_TRUE( DeallocationsAfter - DeallocationsBefore == 1 ) );

        SKL::KPIContext::Destroy();
    }

    TEST( MManagementTestsSuite, MakeSharedArrayNoDestructAndConstruct )
    {
        SKL::KPIContext::Create();
        SKL_IFMEMORYSTATS( const uint64_t AllocationsBefore = SKL::SkylakeGlobalMemoryManager::TotalAllocations );
        SKL_IFMEMORYSTATS( const uint64_t DeallocationsBefore = SKL::SkylakeGlobalMemoryManager::TotalDeallocations );

        {
            auto SharedArray = SKL::MakeSharedArrayNoDestruct<MyType>( 32 );
            ASSERT_TRUE( nullptr != SharedArray.get() );
            ASSERT_TRUE( 1 == SharedArray.use_count() );

           /* for( uint32_t i = 0; i < 32; ++i )
            {
                ASSERT_TRUE( nullptr == SharedArray[ i ].a );
            }*/
        }

        SKL_IFMEMORYSTATS( const uint64_t AllocationsAfter = SKL::SkylakeGlobalMemoryManager::TotalAllocations );
        SKL_IFMEMORYSTATS( const uint64_t DeallocationsAfter = SKL::SkylakeGlobalMemoryManager::TotalDeallocations );
        SKL_IFMEMORYSTATS( ASSERT_TRUE( AllocationsAfter - AllocationsBefore == 1 ) );
        SKL_IFMEMORYSTATS( ASSERT_TRUE( DeallocationsAfter - DeallocationsBefore == 1 ) );

        SKL::KPIContext::Destroy();
    }

    TEST( MManagementTestsSuite, MakeSharedArrayNoDestructButWithConstruct )
    {
        SKL::KPIContext::Create();
        SKL_IFMEMORYSTATS( const uint64_t AllocationsBefore = SKL::SkylakeGlobalMemoryManager::TotalAllocations );
        SKL_IFMEMORYSTATS( const uint64_t DeallocationsBefore = SKL::SkylakeGlobalMemoryManager::TotalDeallocations );

        {
            auto SharedArray = SKL::MakeSharedArrayNoDestruct<MyType, true>( 32 );
            ASSERT_TRUE( nullptr != SharedArray.get() );
            ASSERT_TRUE( 1 == SharedArray.use_count() );

            for( uint32_t i = 0; i < 32; ++i )
            {
                ASSERT_TRUE( nullptr == SharedArray[ i ].a );
            }
        }

        SKL_IFMEMORYSTATS( const uint64_t AllocationsAfter = SKL::SkylakeGlobalMemoryManager::TotalAllocations );
        SKL_IFMEMORYSTATS( const uint64_t DeallocationsAfter = SKL::SkylakeGlobalMemoryManager::TotalDeallocations );
        SKL_IFMEMORYSTATS( ASSERT_TRUE( AllocationsAfter - AllocationsBefore == 1 ) );
        SKL_IFMEMORYSTATS( ASSERT_TRUE( DeallocationsAfter - DeallocationsBefore == 1 ) );

        SKL::KPIContext::Destroy();
    }

    TEST( MManagementTestsSuite, MakeSharedVirtualDeleted_API )
    {
        SKL::KPIContext::Create();
        SKL_IFMEMORYSTATS( const uint64_t AllocationsBefore = SKL::SkylakeGlobalMemoryManager::TotalAllocations );
        SKL_IFMEMORYSTATS( const uint64_t DeallocationsBefore = SKL::SkylakeGlobalMemoryManager::TotalDeallocations );

        int32_t b = 5;

        {
            auto VirtualDeletedObject = SKL::MakeSharedVirtualDeleted<MyType>( { &SKL::GlobalAllocatedDeleter<MyType> }, &b );
            ASSERT_TRUE( nullptr != VirtualDeletedObject.get() );
            ASSERT_TRUE( 1 == VirtualDeletedObject.use_count() );


            SKL::TVirtualDeleter<MyType>& Deleter = VirtualDeletedObject.get_virtual_deleter();
            ASSERT_EQ( Deleter.GetPointer(), &SKL::GlobalAllocatedDeleter<MyType> );
        }

        ASSERT_EQ( 23, b );

        SKL_IFMEMORYSTATS( const uint64_t AllocationsAfter = SKL::SkylakeGlobalMemoryManager::TotalAllocations );
        SKL_IFMEMORYSTATS( const uint64_t DeallocationsAfter = SKL::SkylakeGlobalMemoryManager::TotalDeallocations );
        SKL_IFMEMORYSTATS( ASSERT_TRUE( AllocationsAfter - AllocationsBefore == 1 ) );
        SKL_IFMEMORYSTATS( ASSERT_TRUE( DeallocationsAfter - DeallocationsBefore == 1 ) );

        SKL::KPIContext::Destroy();
    }
}

int main( int argc, char** argv )
{
    testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS( );
}
