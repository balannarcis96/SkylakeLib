#include <gtest/gtest.h>

#include <SkylakeLib.h>

namespace ObjectPoolTestsSuite
{
    struct MyType
    {
        int a { 0 };
    };

    TEST( ObjectPoolTestsSuite, ObjectPool_NoThreads_Test )
    {
        using TMyThreadPool = SKL::ObjectPool<MyType, 1024, true>;
     
        //Preallocate all 1024 items
        TMyThreadPool::Preallocate();   
        
        auto* FirstItem = TMyThreadPool::Debug_ProbeAt( 0 );

        ASSERT_TRUE( nullptr != FirstItem );

        auto* NewObj = TMyThreadPool::Allocate();
        
        ASSERT_TRUE( nullptr != NewObj );
        ASSERT_TRUE( FirstItem == NewObj );
        ASSERT_TRUE( nullptr == TMyThreadPool::Debug_ProbeAt( 0 ) );
    }

    TEST( ObjectPoolTestsSuite, ObjectPool_MultiThreads_SpinLock_Test )
    {
        using TMyThreadPool = SKL::ObjectPool<MyType, 1024>;
     
        //Preallocate all 1024 items
        TMyThreadPool::Preallocate();   
        
        std::jthread Thread1 { []() -> void
        {   
            for( int i =0; i < 50; ++i )
            {
                auto * NewItem { TMyThreadPool::Allocate() };
                ASSERT_TRUE( nullptr != NewItem );
        
                TCLOCK_SLEEP_FOR_MICROS( 250 );
        
                TMyThreadPool::Deallocate( NewItem );
            }
        } };

        std::jthread Thread2 { []() -> void
        {   
            for( int i =0; i < 20; ++i )
            {
                auto * NewItem { TMyThreadPool::Allocate() };
                ASSERT_TRUE( nullptr != NewItem );
        
                TCLOCK_SLEEP_FOR_MICROS( 250 );
        
                TMyThreadPool::Deallocate( NewItem );
            }
        } };

        for( int i =0; i < 20; ++i )
        {
            auto * NewItem { TMyThreadPool::Allocate() };
            ASSERT_TRUE( nullptr != NewItem );
        
            TCLOCK_SLEEP_FOR_MICROS( 250 );
        
            TMyThreadPool::Deallocate( NewItem );
        }

        ASSERT_TRUE( nullptr != TMyThreadPool::Debug_ProbeAt( 0 ) );
    }
}

int main( int argc, char** argv )
{
    testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS( );
}