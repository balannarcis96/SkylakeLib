#include <gtest/gtest.h>

#include <SkylakeLib.h>

namespace MManagementTests
{
    struct MyType
    {
        MyType( int* a ) noexcept : a { a } {} 
        ~MyType() noexcept { 
            *a = 23;
        }

        int *a;
    };

    TEST( MManagementTests, Init )
    {
        //SKL::MemoryManager::Preallocate();
        //SKL::MemoryManager::LogStatistics();
        
        auto AllocResult = SKL::MemoryManager::Allocate<24>();
        ASSERT_TRUE( true == AllocResult.IsValid() );

        SKL::MemoryManager::Deallocate( AllocResult );
        ASSERT_TRUE( false == AllocResult.IsValid() );
    }

    TEST( MManagementTests, MakeUnique )
    {
        int b = 5;
    
        {
            auto UniqueItem = SKL::MakeUnique<MyType>( &b );
            ASSERT_TRUE( nullptr != UniqueItem.get() );
            ASSERT_TRUE( 5 == *UniqueItem->a );
        }

        ASSERT_TRUE( 23 == b );
    }

    TEST( MManagementTests, MakeUniqueNoDeconstruct )
    {
        int b = 5;
    
        {
            auto UniqueItem = SKL::MakeUniqueNoDeconstruct<MyType>( &b );
            ASSERT_TRUE( nullptr != UniqueItem.get() );
            ASSERT_TRUE( 5 == *UniqueItem->a );
        }

        ASSERT_TRUE( 5 == b );
    }

    TEST( MManagementTests, MakeShared )
    {
        int b = 5;
     
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

        ASSERT_TRUE( 23 == b );
    }
}

int main( int argc, char** argv )
{
    testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS( );
}