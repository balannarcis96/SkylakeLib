#include <gtest/gtest.h>

#include <SkylakeLib.h>

namespace MManagementTests
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

    TEST( MManagementTests, Init )
    {
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
    }

    TEST( MManagementTests, MakeUnique )
    {
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
    }

    TEST( MManagementTests, MakeUniqueNoDeconstructAndConstruct )
    {
        int b = 5;
    
        SKL_IFMEMORYSTATS( const uint64_t AllocationsBefore = SKL::SkylakeGlobalMemoryManager::TotalAllocations );
        SKL_IFMEMORYSTATS( const uint64_t DeallocationsBefore = SKL::SkylakeGlobalMemoryManager::TotalDeallocations );

        {
            auto UniqueItem = SKL::MakeUniqueNoDeconstruct<MyType>( &b );
            ASSERT_TRUE( nullptr != UniqueItem.get() );
            //ASSERT_TRUE( 5 == *UniqueItem->a );
        }

        SKL_IFMEMORYSTATS( const uint64_t AllocationsAfter = SKL::SkylakeGlobalMemoryManager::TotalAllocations );
        SKL_IFMEMORYSTATS( const uint64_t DeallocationsAfter = SKL::SkylakeGlobalMemoryManager::TotalDeallocations );
        SKL_IFMEMORYSTATS( ASSERT_TRUE( AllocationsAfter - AllocationsBefore == 1 ) );
        SKL_IFMEMORYSTATS( ASSERT_TRUE( DeallocationsAfter - DeallocationsBefore == 1 ) );

        ASSERT_TRUE( 5 == b );
    }

    TEST( MManagementTests, MakeUniqueArray )
    {
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
    }

    TEST( MManagementTests, MakeUniqueArrayWithNoDestructAndConstruct )
    {
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
    }

    TEST( MManagementTests, MakeUniqueArrayWithNoDestructButConstruct )
    {
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
    }

    TEST( MManagementTests, MakeShared )
    {
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
    }

    TEST( MManagementTests, MakeSharedNoDestructAndConstruct )
    {
        int b = 5;
     
        SKL_IFMEMORYSTATS( const uint64_t AllocationsBefore = SKL::SkylakeGlobalMemoryManager::TotalAllocations );
        SKL_IFMEMORYSTATS( const uint64_t DeallocationsBefore = SKL::SkylakeGlobalMemoryManager::TotalDeallocations );

        {
            auto SharedItem = SKL::MakeSharedNoDestruct<MyType>( &b );
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
    }

    TEST( MManagementTests, MakeSharedNoDestructButWithConstruct )
    {
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
    }

    TEST( MManagementTests, MakeSharedArray )
    {
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
    }

    TEST( MManagementTests, MakeSharedArrayNoDestructAndConstruct )
    {
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
    }

    TEST( MManagementTests, MakeSharedArrayNoDestructButWithConstruct )
    {
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
    }
}

int main( int argc, char** argv )
{
    testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS( );
}