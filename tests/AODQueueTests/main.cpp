#include <gtest/gtest.h>

#include <SkylakeLib.h>

namespace AODQueueTests
{
    TEST( AODQueueTests, SameThread )
    {
        constexpr uint32_t           IterCount { 10000 };
        SKL::AODTaskQueue            Queue;
        using TaskType = SKL::AODSharedObjectTask<32>;

        for( uint32_t i = 0; i < IterCount; ++i )
        {
            const auto NewTask{ new TaskType() };
            ASSERT_TRUE( nullptr == NewTask->Next );

            Queue.Push( NewTask );
        }

        for( uint32_t i = 0 ; i < IterCount; ++ i )
        {
            int32_t FailCount { 0 };

            while( 16 > FailCount )
            {
                const auto* Task{ Queue.Pop() };
                if( nullptr != Task )
                {
                    delete Task;
                    break;
                }
                
                ++FailCount;

            }
        
            ASSERT_TRUE( 16 > FailCount );
        }
    }
    
    TEST( AODQueueTests, TwoThreads_Produce_Then_Consume )
    {
        constexpr uint32_t           IterCount { 10000 };
        SKL::AODTaskQueue            Queue;
        using TaskType = SKL::AODSharedObjectTask<32>;
        std::latch                   Sync{ 2 };

        std::jthread Producer{ [ &Sync, &Queue ]() 
        {
            for( uint32_t i = 0; i < IterCount; ++i )
            {
                const auto NewTask{ new TaskType() };
                ASSERT_TRUE( nullptr == NewTask->Next );

                Queue.Push( NewTask );
            }

            Sync.arrive_and_wait();
        } };
        
        Sync.arrive_and_wait();

        for( uint32_t i = 0 ; i < IterCount; ++ i )
        {
            const auto* Task{ Queue.Pop() };
            ASSERT_TRUE( nullptr != Task );
            delete Task;
        }
    }
    
    TEST( AODQueueTests, TwoThreads_Produce_And_Consume )
    {
        constexpr uint32_t           IterCount { 10000 };
        SKL::AODTaskQueue            Queue;
        using TaskType = SKL::AODSharedObjectTask<32>;
        std::latch                   Sync{ 2 };

        std::jthread Producer{ [ &Sync, &Queue ]() 
        {
            Sync.arrive_and_wait();

            for( uint32_t i = 0; i < IterCount; ++i )
            {
                const auto NewTask{ new TaskType() };
                ASSERT_TRUE( nullptr == NewTask->Next );

                Queue.Push( NewTask );
            }
        } };
        
        Sync.arrive_and_wait();
        for( uint32_t i = 0 ; i < IterCount; ++ i )
        {
            int32_t FailCount { 0 };

            while( 1024 > FailCount )
            {
                const auto* Task{ Queue.Pop() };
                if( nullptr != Task )
                {
                    break;
                }
                
                ++FailCount;
                std::this_thread::yield();
            }
        
            ASSERT_TRUE( 1024 > FailCount );
        }
    }

    TEST( AODQueueTests, MultipleProducers_SingleConsummer_SyncStart )
    {
        constexpr uint32_t           IterCount { 10000 };
        constexpr uint32_t           ProducersCount { 16 };
        SKL::AODTaskQueue            Queue;
        using TaskType = SKL::AODSharedObjectTask<32>;
        std::latch                   Sync{ ProducersCount + 1 };

        std::jthread Producers[ ProducersCount ];

        for( uint32_t i = 0; i < ProducersCount; ++i )
        {
            Producers[i] = std::jthread{ [ &Sync, &Queue ]()
            {
                Sync.arrive_and_wait();

                for( uint32_t i = 0; i < IterCount; ++i )
                {
                    const auto NewTask{ new TaskType() };
                    ASSERT_TRUE( nullptr == NewTask->Next );

                    Queue.Push( NewTask );
                }
            } };
        }
        
        Sync.arrive_and_wait();
        for( uint32_t i = 0 ; i < ProducersCount * IterCount; ++ i )
        {
            int32_t FailCount { 0 };

            while( 1024 > FailCount )
            {
                const auto* Task{ Queue.Pop() };
                if( nullptr != Task )
                {
                    break;
                }
                
                ++FailCount;
                std::this_thread::yield();
            }
        
            ASSERT_TRUE( 1024 > FailCount );
        }
    }
    
    TEST( AODQueueTests, MultipleProducers_SingleConsummer )
    {
        constexpr uint32_t           IterCount { 10000 };
        constexpr uint32_t           ProducersCount { 16 };
        SKL::AODTaskQueue            Queue;
        using TaskType = SKL::AODSharedObjectTask<32>;

        std::jthread Producers[ ProducersCount ];

        for( uint32_t i = 0; i < ProducersCount; ++i )
        {
            Producers[i] = std::jthread{ [ &Queue ]()
            {
                for( uint32_t i = 0; i < IterCount; ++i )
                {
                    const auto NewTask{ new TaskType() };
                    ASSERT_TRUE( nullptr == NewTask->Next );

                    Queue.Push( NewTask );
                }
            } };
        }
        
        for( uint32_t i = 0 ; i < ProducersCount * IterCount; ++ i )
        {
            int32_t FailCount { 0 };

            while( 1024 > FailCount )
            {
                const auto* Task{ Queue.Pop() };
                if( nullptr != Task )
                {
                    break;
                }
                
                ++FailCount;
                std::this_thread::yield();
            }
        
            ASSERT_TRUE( 1024 > FailCount );
        }
    }
}

int main( int argc, char** argv )
{
    testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS( );
}