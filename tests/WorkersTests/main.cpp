#include <gtest/gtest.h>

#include <SkylakeLib.h>
#include <ApplicationSetup.h>

namespace WorkersTests
{
    struct MyType { 
        void operator()( SKL::Worker& /*InWorker*/, SKL::WorkerGroup& /*InGroup*/ ) noexcept {}
        
        int a;
    };

    struct Test_Fixture_____1: public ::testing::Test, TestApplication
    {
         Test_Fixture_____1()
                : ::testing::Test(),
                  TestApplication( L"WORKERSTESTS_TESTS_APP" ) {}      

        // Start sequence
        bool OnAddServices() noexcept override
        { 
            SKLL_TRACE();

            EXPECT_TRUE( 1 == ++SequenceCount );
            
            return true; 
        }
        bool OnBeforeStartServer() noexcept override
        {
            SKLL_TRACE();
            
            EXPECT_TRUE( 2 == ++SequenceCount );
            
            if( false == TestApplication::OnBeforeStartServer() )
            {
                return false;
            }
            
            return true;
        }
        bool OnWorkerStarted( SKL::Worker& InWorker, SKL::WorkerGroup& Group ) noexcept override
        {
            SKLL_TRACE();
            
            EXPECT_TRUE( 3 == ++SequenceCount );
            
            if( false == TestApplication::OnWorkerStarted( InWorker, Group ) )
            {
                return false;
            }

            return true;
        }
        bool OnAllWorkersStarted( SKL::WorkerGroup& Group ) noexcept override
        {
            SKLL_TRACE();
            
            EXPECT_TRUE( 4 == ++SequenceCount );
            
            if( false == TestApplication::OnAllWorkersStarted( Group ) )
            {
                return false;
            }
            
            return true;
        }
        bool OnWorkerGroupStarted( SKL::WorkerGroup& Group ) noexcept override
        {
            SKLL_TRACE();
            
            EXPECT_TRUE( 5 == ++SequenceCount );
            
            if( false == TestApplication::OnWorkerGroupStarted( Group ) )
            {
                return false;
            }
            
            return true;
        }
        bool OnAllWorkerGroupsStarted() noexcept override
        {
            SKLL_TRACE();
            
            EXPECT_TRUE( 6 == ++SequenceCount );
            
            if( false == TestApplication::OnAllWorkerGroupsStarted() )
            {
                return false;
            }
            
            return true;
        }
        bool OnServerStarted() noexcept override
        {
            SKLL_TRACE();
            
            EXPECT_TRUE( 7 == ++SequenceCount );
            
            if( false == TestApplication::OnServerStarted() )
            {
                return false;
            }
            
            StartLath.arrive_and_wait();

            return true;
        }

        // Stop sequence
        bool OnBeforeStopServer() noexcept override
        {
            SKLL_TRACE();
            
            EXPECT_TRUE( 8 == ++SequenceCount );
            
            if( false == TestApplication::OnBeforeStopServer() )
            {
                return false;
            }
            
            return true;
        }
        void OnAllServiceStopped() noexcept override
        {
            SKLL_TRACE();

            EXPECT_TRUE( 9 == ++SequenceCount );
            
            TestApplication::OnAllServiceStopped();
        }
        bool OnWorkerStopped( SKL::Worker& InWorker, SKL::WorkerGroup& Group ) noexcept override
        {
            SKLL_TRACE();
            
            EXPECT_TRUE( 10 == ++SequenceCount );
            
            if( false == TestApplication::OnWorkerStopped( InWorker, Group ) )
            {
                return false;
            }
            
            return true;
        }
        bool OnAllWorkersStopped( SKL::WorkerGroup& Group ) noexcept override
        {
            SKLL_TRACE();
            
            EXPECT_TRUE( 11 == ++SequenceCount );
            
            if( false == TestApplication::OnAllWorkersStopped( Group ) )
            {
                return false;
            }
            
            return true;
        }
        bool OnWorkerGroupStopped( SKL::WorkerGroup& Group ) noexcept override
        {
            SKLL_TRACE();
            
            EXPECT_TRUE( 12 == ++SequenceCount );
            
            if( false == TestApplication::OnWorkerGroupStopped( Group ) )
            {
                return false;
            }
            
            return true;
        }
        bool OnAllWorkerGroupsStopped() noexcept override
        {
            SKLL_TRACE();
            
            EXPECT_TRUE( 13 == ++SequenceCount );
            
            if( false == TestApplication::OnAllWorkerGroupsStopped() )
            {
                return false;
            }
            
            return true;
        }
        bool OnServerStopped() noexcept override
        {
            SKLL_TRACE();
            
            EXPECT_TRUE( 14 == ++SequenceCount );
            
            if( false == TestApplication::OnServerStopped() )
            {
                return false;
            }
            
            return true;
        }
        bool OnAfterServerStopped() noexcept override
        {
            SKLL_TRACE();
            
            EXPECT_TRUE( 15 == ++SequenceCount );
            
            if( false == TestApplication::OnAfterServerStopped() )
            {
                return false;
            }
            
            return true;
        }
        
        void OnServiceStopped( SKL::IService* InService, SKL::RStatus InStatus ) noexcept override
        {
            SKLL_TRACE();
            TestApplication::OnServiceStopped( InService, InStatus );
        }

        void SetUp() override
        {
            ASSERT_TRUE( SKL::RSuccess == SKL::Skylake_InitializeLibrary( 0, nullptr, nullptr ) );
        }
        void TearDown() override
        {
            ASSERT_TRUE( SKL::RSuccess == SKL::Skylake_TerminateLibrary() );
        }

        uint32_t SequenceCount{ 0 };
        std::latch StartLath{ 2 };
    };
    
    struct Test_Fixture_____2: public ::testing::Test, TestApplication
    {
         Test_Fixture_____2()
                : ::testing::Test(),
                  TestApplication( L"WORKERSTESTS_TESTS_APP" ) {}      

        // Start sequence
        bool OnAddServices() noexcept override
        { 
            SKLL_TRACE();

            EXPECT_TRUE( 1 == ( SequenceCount.increment() + 1 ) );
            
            return true; 
        }
        bool OnBeforeStartServer() noexcept override
        {
            SKLL_TRACE();
            
            EXPECT_TRUE( 2 == ( SequenceCount.increment() + 1 ) );
            
            if( false == TestApplication::OnBeforeStartServer() )
            {
                return false;
            }
            
            return true;
        }
        bool OnWorkerStarted( SKL::Worker& InWorker, SKL::WorkerGroup& Group ) noexcept override
        {
            SKLL_TRACE();
            
            EXPECT_TRUE( 3 <= ( SequenceCount.increment() + 1 ) );
            
            if( false == TestApplication::OnWorkerStarted( InWorker, Group ) )
            {
                return false;
            }

            return true;
        }
        bool OnAllWorkersStarted( SKL::WorkerGroup& Group ) noexcept override
        {
            SKLL_TRACE();
            
            EXPECT_TRUE( 19 == ( SequenceCount.increment() + 1 ) );
            
            if( false == TestApplication::OnAllWorkersStarted( Group ) )
            {
                return false;
            }
            
            return true;
        }
        bool OnWorkerGroupStarted( SKL::WorkerGroup& Group ) noexcept override
        {
            SKLL_TRACE();
            
            EXPECT_TRUE( 20 == ( SequenceCount.increment() + 1 ) );
            
            if( false == TestApplication::OnWorkerGroupStarted( Group ) )
            {
                return false;
            }
            
            return true;
        }
        bool OnAllWorkerGroupsStarted() noexcept override
        {
            SKLL_TRACE();
            
            EXPECT_TRUE( 21 == ( SequenceCount.increment() + 1 ) );
            
            if( false == TestApplication::OnAllWorkerGroupsStarted() )
            {
                return false;
            }
            
            return true;
        }
        bool OnServerStarted() noexcept override
        {
            SKLL_TRACE();
            
            EXPECT_TRUE( 22 == ( SequenceCount.increment() + 1 ) );
            
            if( false == TestApplication::OnServerStarted() )
            {
                return false;
            }
            
            StartLath.arrive_and_wait();

            return true;
        }

        // Stop sequence
        bool OnBeforeStopServer() noexcept override
        {
            SKLL_TRACE();
            
            EXPECT_TRUE( 23 == ( SequenceCount.increment() + 1 ) );
            
            if( false == TestApplication::OnBeforeStopServer() )
            {
                return false;
            }
            
            return true;
        }
        void OnAllServiceStopped() noexcept override
        {
            SKLL_TRACE();

            EXPECT_TRUE( 24 == ( SequenceCount.increment() + 1 ) );
            
            TestApplication::OnAllServiceStopped();
        }
        bool OnWorkerStopped( SKL::Worker& InWorker, SKL::WorkerGroup& Group ) noexcept override
        {
            SKLL_TRACE();
            
            EXPECT_TRUE( 25 <= ( SequenceCount.increment() + 1 ) );
            
            if( false == TestApplication::OnWorkerStopped( InWorker, Group ) )
            {
                return false;
            }
            
            return true;
        }
        bool OnAllWorkersStopped( SKL::WorkerGroup& Group ) noexcept override
        {
            SKLL_TRACE();
            
            EXPECT_EQ( 41U , ( SequenceCount.increment() + 1U ) );
            
            if( false == TestApplication::OnAllWorkersStopped( Group ) )
            {
                return false;
            }
            
            return true;
        }
        bool OnWorkerGroupStopped( SKL::WorkerGroup& Group ) noexcept override
        {
            SKLL_TRACE();
            
            EXPECT_TRUE( 42 == ( SequenceCount.increment() + 1 ) );
            
            if( false == TestApplication::OnWorkerGroupStopped( Group ) )
            {
                return false;
            }
            
            return true;
        }
        bool OnAllWorkerGroupsStopped() noexcept override
        {
            SKLL_TRACE();
            
            EXPECT_TRUE( 43 == ( SequenceCount.increment() + 1 ) );
            
            if( false == TestApplication::OnAllWorkerGroupsStopped() )
            {
                return false;
            }
            
            return true;
        }
        bool OnServerStopped() noexcept override
        {
            SKLL_TRACE();
            
            EXPECT_TRUE( 44 == ( SequenceCount.increment() + 1 ) );
            
            if( false == TestApplication::OnServerStopped() )
            {
                return false;
            }
            
            return true;
        }
        bool OnAfterServerStopped() noexcept override
        {
            SKLL_TRACE();
            
            EXPECT_TRUE( 45 == ( SequenceCount.increment() + 1 ) );
            
            if( false == TestApplication::OnAfterServerStopped() )
            {
                return false;
            }
            
            return true;
        }
        
        void OnServiceStopped( SKL::IService* InService, SKL::RStatus InStatus ) noexcept override
        {
            SKLL_TRACE();
            TestApplication::OnServiceStopped( InService, InStatus );
        }

        void SetUp() override
        {
            ASSERT_TRUE( SKL::RSuccess == SKL::Skylake_InitializeLibrary( 0, nullptr, nullptr ) );
        }
        void TearDown() override
        {
            ASSERT_TRUE( SKL::RSuccess == SKL::Skylake_TerminateLibrary() );
        }

        std::synced_value<uint32_t> SequenceCount{ 0 };
        std::latch StartLath{ 2 };
    };
    
    struct Test_Fixture_____3: public ::testing::Test, TestApplication
    {
         Test_Fixture_____3()
                : ::testing::Test(),
                  TestApplication( L"WORKERSTESTS_TESTS_APP" ) {}      

        // Start sequence
        bool OnAddServices() noexcept override
        { 
            SKLL_TRACE();

            EXPECT_TRUE( 0 == SequenceCount.increment() );
            
            return true; 
        }
        bool OnBeforeStartServer() noexcept override
        {
            SKLL_TRACE();
            
            EXPECT_TRUE( 1 == SequenceCount.increment() );
            
            if( false == TestApplication::OnBeforeStartServer() )
            {
                return false;
            }
            
            return true;
        }
        bool OnWorkerStarted( SKL::Worker& InWorker, SKL::WorkerGroup& Group ) noexcept override
        {
            SKLL_TRACE();
            
            EXPECT_TRUE( 2 <= SequenceCount.increment() );
            
            if( false == TestApplication::OnWorkerStarted( InWorker, Group ) )
            {
                return false;
            }

            return true;
        }
        bool OnAllWorkersStarted( SKL::WorkerGroup& Group ) noexcept override
        {
            SKLL_TRACE();
            
            SequenceCount.increment();
            
            for( auto& Worker : Group.GetWorkers() )
            {
                if( nullptr == Worker ){ continue; }
                EXPECT_TRUE( true == Worker->GetIsRunning() );
            }

            if( false == TestApplication::OnAllWorkersStarted( Group ) )
            {
                return false;
            }
            
            return true;
        }
        bool OnWorkerGroupStarted( SKL::WorkerGroup& Group ) noexcept override
        {
            SKLL_TRACE();
            
            SequenceCount.increment();
            
            for( auto& Worker : Group.GetWorkers() )
            {
                if( nullptr == Worker ){ continue; }
                EXPECT_TRUE( true == Worker->GetIsRunning() );
            }

            if( false == TestApplication::OnWorkerGroupStarted( Group ) )
            {
                return false;
            }
            
            return true;
        }
        bool OnAllWorkerGroupsStarted() noexcept override
        {
            SKLL_TRACE();
            
            EXPECT_TRUE( 38 == SequenceCount.increment() );
            
            for( auto* WorkerGroup : GetAllWorkerGroups() )
            {
                if( nullptr == WorkerGroup ){ continue; }
                EXPECT_TRUE( true == WorkerGroup->IsRunning() );
            }

            if( false == TestApplication::OnAllWorkerGroupsStarted() )
            {
                return false;
            }
            
            return true;
        }
        bool OnServerStarted() noexcept override
        {
            SKLL_TRACE();
            
            EXPECT_TRUE( 39 == SequenceCount.increment() );

            if( false == TestApplication::OnServerStarted() )
            {
                return false;
            }
            
            StartLath.arrive_and_wait();

            return true;
        }

        // Stop sequence
        bool OnBeforeStopServer() noexcept override
        {
            SKLL_TRACE();
            
            EXPECT_TRUE( 40 == SequenceCount.increment() );
            
            if( false == TestApplication::OnBeforeStopServer() )
            {
                return false;
            }
            
            return true;
        }
        void OnAllServiceStopped() noexcept override
        {
            SKLL_TRACE();

            EXPECT_TRUE( 41 == SequenceCount.increment() );
            
            TestApplication::OnAllServiceStopped();
        }
        bool OnWorkerStopped( SKL::Worker& InWorker, SKL::WorkerGroup& Group ) noexcept override
        {
            SKLL_TRACE();
            
            SequenceCount.increment();
            EXPECT_TRUE( false == InWorker.GetIsRunning() );
            
            if( false == TestApplication::OnWorkerStopped( InWorker, Group ) )
            {
                return false;
            }
            
            return true;
        }
        bool OnAllWorkersStopped( SKL::WorkerGroup& Group ) noexcept override
        {
            SKLL_TRACE();
            
            SequenceCount.increment();

            for( const auto& Worker : Group.GetWorkers() )
            {
                if( nullptr == Worker ){ continue; }
                EXPECT_TRUE( false == Worker->GetIsRunning() );
            }

            if( false == TestApplication::OnAllWorkersStopped( Group ) )
            {
                return false;
            }
            
            return true;
        }
        bool OnWorkerGroupStopped( SKL::WorkerGroup& Group ) noexcept override
        {
            SKLL_TRACE();
            
            SequenceCount.increment();

            for( auto& Worker : Group.GetWorkers() )
            {
                if( nullptr == Worker ){ continue; }
                EXPECT_TRUE( false == Worker->GetIsRunning() );
            }

            if( false == TestApplication::OnWorkerGroupStopped( Group ) )
            {
                return false;
            }
            
            return true;
        }
        bool OnAllWorkerGroupsStopped() noexcept override
        {
            SKLL_TRACE();
            
            EXPECT_TRUE( 78 == SequenceCount.increment() );
            
            for( const auto* WorkerGroup : GetAllWorkerGroups() )
            {
                if( nullptr == WorkerGroup ){ continue; }
                EXPECT_TRUE( false == WorkerGroup->IsRunning() );
            }

            if( false == TestApplication::OnAllWorkerGroupsStopped() )
            {
                return false;
            }
            
            return true;
        }
        bool OnServerStopped() noexcept override
        {
            SKLL_TRACE();
            
            EXPECT_TRUE( 79 == SequenceCount.increment() );
            
            for( auto* WorkerGroup : GetAllWorkerGroups() )
            {
                if( nullptr == WorkerGroup ){ continue; }
                EXPECT_TRUE( false == WorkerGroup->IsRunning() );
            }

            if( false == TestApplication::OnServerStopped() )
            {
                return false;
            }
            
            return true;
        }
        bool OnAfterServerStopped() noexcept override
        {
            SKLL_TRACE();
            
            EXPECT_TRUE( 80 == SequenceCount.increment() );
            
            for( auto& WorkerGroup : GetAllWorkerGroups() )
            {
                if( nullptr == WorkerGroup ){ continue; }
                EXPECT_TRUE( false == WorkerGroup->IsRunning() );
            }

            if( false == TestApplication::OnAfterServerStopped() )
            {
                return false;
            }
            
            return true;
        }
        
        void OnServiceStopped( SKL::IService* InService, SKL::RStatus InStatus ) noexcept override
        {
            SKLL_TRACE();
            TestApplication::OnServiceStopped( InService, InStatus );
        }

        void SetUp() override
        {
            ASSERT_TRUE( SKL::RSuccess == SKL::Skylake_InitializeLibrary( 0, nullptr, nullptr ) );
        }
        void TearDown() override
        {
            ASSERT_TRUE( SKL::RSuccess == SKL::Skylake_TerminateLibrary() );
        }

        std::relaxed_value<uint32_t> SequenceCount{ 0 };
        std::latch StartLath{ 2 };
    };

    TEST( WorkersTestsSuite, Main_Test__Case )
    {
        SKL::Skylake_InitializeLibrary( 0, nullptr, nullptr );

        {
            SKL::ServerInstance Manager;

            SKL::ServerInstanceConfig::ServerInstanceConfig Config { L"TEST_APPLICATION" };

            SKL::WorkerGroupTag Tag1
            {
                .TickRate        = 5, 
                .SyncTLSTickRate = 0,
                .Id              = 1,
                .WorkersCount    = 1,
                .Name            = L"FRONT_END_GROUP"
            };
            Tag1.bIsActive = true;
            Tag1.bEnableAsyncIO = true;
            Tag1.bCallTickHandler = true;

            SKL::ServerInstanceConfig::WorkerGroupConfig Group1{ Tag1 };
            ASSERT_TRUE( true == Group1.Validate() );

            Group1.SetWorkerStartHandler( []( SKL::Worker& /*InWorker*/, SKL::WorkerGroup& /*InGroup*/ ) mutable noexcept -> bool {
                SKLL_INF( "Worker Group1 WORKER STARTED!" );
                return true;
            } ); 

            Group1.SetWorkerTickHandler( [ &Manager ]( SKL::Worker& Worker, SKL::WorkerGroup& /*Group*/ ) mutable noexcept -> void
            {
                SKLL_INF( "Worker Group1 Tick()" );

                if( Worker.GetAliveTime() < 1000 )
                {
                    return ;
                }

                ( void )Manager.GetWorkerGroupById( 2 )->Defer( [ &Manager ]( SKL::ITask* /*Self*/ ) noexcept {
                    SKLL_INF( "From TASK" );
                    Manager.SignalToStop();
                } );
            
                TCLOCK_SLEEP_FOR_MILLIS( 50 );

                return ;
            } );
            Config.AddNewGroup( std::move( Group1 ) );
            
            SKL::WorkerGroupTag Tag2
            {
                .TickRate        = 5, 
                .SyncTLSTickRate = 0,
                .Id              = 2,
                .WorkersCount    = 2,
                .Name            = L"BACK_END_GROUP"
            };
            Tag2.bEnableAsyncIO = true;
            Tag2.bCallTickHandler = true;

            SKL::ServerInstanceConfig::WorkerGroupConfig Group2{ Tag2 };
            Group2.SetWorkerTickHandler( [ /*&Manager*/ ]( SKL::Worker& /*Worker*/, SKL::WorkerGroup& /*Group*/ ) mutable noexcept -> void
            {
                SKLL_INF( "Worker Group2 Tick()" );
            } );
            Config.AddNewGroup( std::move( Group2 ) );

            auto Result = Manager.Initialize( std::move( Config ) );
            ASSERT_TRUE( SKL::RSuccess == Result );

            auto* QueryResult = Manager.GetWorkerGroupById( 1 );
            ASSERT_TRUE( nullptr != QueryResult );
            ASSERT_TRUE( 1 == QueryResult->GetTag().Id );

            QueryResult = Manager.GetWorkerGroupById( 2 );
            ASSERT_TRUE( nullptr != QueryResult );
            ASSERT_TRUE( 2 == QueryResult->GetTag().Id );

            ( void )Manager.StartServer();
        }

        SKL::Skylake_TerminateLibrary();
    }

    TEST_F( Test_Fixture_____1, FullFlow_OneReactiveWorker )
    {
        const auto TotalAllocationsBefore  { SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsBefore{ SKL::GlobalMemoryManager::TotalDeallocations.load() };

        SKL::WorkerGroupTag Tag{
            .TickRate        = 24, 
            .SyncTLSTickRate = 0,
            .Id              = 1,
            .WorkersCount    = 1,
            .Name            = L"AODObjectMultipleWorkers_MultipleDeferedTasks_REACTIVE"
        };
        Tag.bEnableAsyncIO = true;

        ASSERT_TRUE( true == AddNewWorkerGroup( Tag, []( SKL::Worker& /*Worker*/, SKL::WorkerGroup& /*Group*/ ) noexcept -> void {} ) );
        ASSERT_TRUE( true == Start( false ) );

        StartLath.arrive_and_wait();

        Stop();

        const auto TotalAllocationsAfter{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsAfter{ SKL::GlobalMemoryManager::TotalDeallocations.load() };
        ASSERT_TRUE( TotalAllocationsBefore == TotalAllocationsAfter );
        ASSERT_TRUE( TotalDeallocationsBefore == TotalDeallocationsAfter );
    }
    
    TEST_F( Test_Fixture_____2, FullFlow_MultipleReactiveWorkers )
    {
        const auto TotalAllocationsBefore  { SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsBefore{ SKL::GlobalMemoryManager::TotalDeallocations.load() };
        
        SKL::WorkerGroupTag Tag{
            .TickRate        = 24, 
            .SyncTLSTickRate = 0,
            .Id              = 1,
            .WorkersCount    = 16,
            .Name            = L"AODObjectMultipleWorkers_MultipleDeferedTasks_REACTIVE"
        };
        Tag.bEnableAsyncIO = true;

        ASSERT_TRUE( true == AddNewWorkerGroup( Tag, []( SKL::Worker& /*Worker*/, SKL::WorkerGroup& /*Group*/ ) noexcept -> void {} ) );
        ASSERT_TRUE( true == Start( false ) );

        StartLath.arrive_and_wait();

        Stop();

        const auto TotalAllocationsAfter{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsAfter{ SKL::GlobalMemoryManager::TotalDeallocations.load() };
        ASSERT_TRUE( TotalAllocationsBefore == TotalAllocationsAfter );
        ASSERT_TRUE( TotalDeallocationsBefore == TotalDeallocationsAfter );
    }
    
    TEST_F( Test_Fixture_____3, FullFlow_MultipleReactiveAndActiveWorkers )
    {
        const auto TotalAllocationsBefore  { SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsBefore{ SKL::GlobalMemoryManager::TotalDeallocations.load() };
        
        SKL::WorkerGroupTag Tag1{
            .TickRate        = 24, 
            .SyncTLSTickRate = 0,
            .Id              = 1,
            .WorkersCount    = 16,
            .Name            = L"REACTIVE"
        };
        Tag1.bEnableAsyncIO = true;

        ASSERT_TRUE( true == AddNewWorkerGroup( Tag1, []( SKL::Worker& /*Worker*/, SKL::WorkerGroup& /*Group*/ ) noexcept -> void {} ) );
        
        SKL::WorkerGroupTag Tag2{
            .TickRate        = 24, 
            .SyncTLSTickRate = 0,
            .Id              = 1,
            .WorkersCount    = 16,
            .Name            = L"ACTIVE"
        };
        Tag2.bIsActive = true;

        ASSERT_TRUE( true == AddNewWorkerGroup( Tag2, []( SKL::Worker& /*Worker*/, SKL::WorkerGroup& /*Group*/ ) noexcept -> void {} ) );
        ASSERT_TRUE( true == Start( false ) );

        StartLath.arrive_and_wait();

        Stop();

        const auto TotalAllocationsAfter{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsAfter{ SKL::GlobalMemoryManager::TotalDeallocations.load() };
        ASSERT_TRUE( TotalAllocationsBefore == TotalAllocationsAfter );
        ASSERT_TRUE( TotalDeallocationsBefore == TotalDeallocationsAfter );
    }
}

int main( int argc, char** argv )
{
    testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS( );
}
