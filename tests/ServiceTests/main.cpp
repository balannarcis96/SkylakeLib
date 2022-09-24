#include <gtest/gtest.h>

#include <SkylakeLib.h>
#include <ApplicationSetup.h>

namespace ServicesTests
{
    class SimpleService_TestsFixture : public ::testing::Test, public TestApplication
    {
    public:
        class MySimpleService: public SKL::SimpleService
        {
        public:
            MySimpleService() noexcept: SKL::SimpleService{ 1 }{ }

        protected:
            SKL::RStatus Initialize() noexcept override
            {
                SKL_ASSERT_ALLWAYS( 1 == ++SeqCounter );
                return SKL::RSuccess;
            }

            void OnServerStarted() noexcept override
            {
                SKL_ASSERT_ALLWAYS( 2 == ++SeqCounter );

                SKL::DeferTask([ this ]( SKL::ITask* ) noexcept -> void 
                {
                    GetServerInstance().SignalToStop();
                });
            }
    
            void OnServerStopped() noexcept override
            {
                SKL_ASSERT_ALLWAYS( 4 == ++SeqCounter );
            }
    
            void OnServerStopSignaled() noexcept override
            {
                SKL_ASSERT_ALLWAYS( 3 == ++SeqCounter );
            }

            int32_t SeqCounter{ 0 };
        };

        SimpleService_TestsFixture()
            : ::testing::Test(),
              TestApplication( L"AOD_TESTS_APP" ) {}      
  
        bool OnAddServices() noexcept override
        {
            SKL_ASSERT_ALLWAYS( true == AddService( new MySimpleService() ) );
            return true;
        }

        void SetUp() override
        {
            ASSERT_TRUE( SKL::RSuccess == SKL::Skylake_InitializeLibrary( 0, nullptr, nullptr ) );
        }

        void TearDown() override
        {
            ASSERT_TRUE( SKL::RSuccess == SKL::Skylake_TerminateLibrary() );
        }
    };

    class AODService_TestsFixture : public ::testing::Test, public TestApplication
    {
    public:
        class MyService: public SKL::AODService
        {
        public:
            MyService() noexcept: SKL::AODService{ 1 }{ }

        protected:
            SKL::RStatus Initialize() noexcept override
            {
                SKL_ASSERT_ALLWAYS( 1 == ++SeqCounter );
                return SKL::RSuccess;
            }

            void OnServerStarted() noexcept override
            {
                SKL_ASSERT_ALLWAYS( 2 == ++SeqCounter );

                SKL::DeferTask([ this ]( SKL::ITask* ) noexcept -> void 
                {
                    DoAsync([ this ]() noexcept -> void 
                    {
                        GetServerInstance().SignalToStop();
                    } );
                });
            }
    
            void OnServerStopped() noexcept override
            {
                SKL_ASSERT_ALLWAYS( 4 == ++SeqCounter );
            }
    
            void OnServerStopSignaled() noexcept override
            {
                SKL_ASSERT_ALLWAYS( 3 == ++SeqCounter );
            }

            int32_t SeqCounter{ 0 };
        };

        AODService_TestsFixture()
            : ::testing::Test(),
              TestApplication( L"AOD_TESTS_APP" ) {}      
  
        bool OnAddServices() noexcept override
        {
            SKL_ASSERT_ALLWAYS( true == AddService( new MyService() ) );
            return true;
        }

        void SetUp() override
        {
            ASSERT_TRUE( SKL::RSuccess == SKL::Skylake_InitializeLibrary( 0, nullptr, nullptr ) );
        }

        void TearDown() override
        {
            ASSERT_TRUE( SKL::RSuccess == SKL::Skylake_TerminateLibrary() );
        }
    };

    class ActiveService_TestsFixture : public ::testing::Test, public TestApplication
    {
    public:
        class MyService: public SKL::ActiveService
        {
        public:
            MyService() noexcept: SKL::ActiveService{ 1 }{ }

        protected:
            SKL::RStatus Initialize() noexcept override
            {
                SKL_ASSERT_ALLWAYS( 1 == ++SeqCounter );
                return SKL::RSuccess;
            }

            void OnServerStarted() noexcept override
            {
                SKL_ASSERT_ALLWAYS( 2 == ++SeqCounter );
            }
    
            void OnServerStopped() noexcept override
            {
                //SKL_ASSERT_ALLWAYS( 4 == ++SeqCounter );
            }
    
            void OnServerStopSignaled() noexcept override
            {
                //SKL_ASSERT_ALLWAYS( 3 == ++SeqCounter );
            }

            void OnTick() noexcept override
            {
                if( ++SeqCounter == 10000 )
                {
                    GetServerInstance().SignalToStop();
                }
            }

            int32_t SeqCounter{ 0 };
        };

        ActiveService_TestsFixture()
            : ::testing::Test(),
              TestApplication( L"AOD_TESTS_APP" ) {}      
  
        bool OnAddServices() noexcept override
        {
            SKL_ASSERT_ALLWAYS( true == AddService( new MyService() ) );
            return true;
        }

        void SetUp() override
        {
            ASSERT_TRUE( SKL::RSuccess == SKL::Skylake_InitializeLibrary( 0, nullptr, nullptr ) );
        }

        void TearDown() override
        {
            ASSERT_TRUE( SKL::RSuccess == SKL::Skylake_TerminateLibrary() );
        }
    };

    class WorkerService_TestsFixture : public ::testing::Test, public TestApplication
    {
    public:
        using TLSCounter = SKL::TLSValue<uint32_t, 0, WorkerService_TestsFixture>;
        static constexpr uint32_t CWorkersCount{ 2 };
        static constexpr int32_t CIterCount{ 100 };
        static constexpr uint32_t CTickRate{ 210 };

        class MyService: public SKL::WorkerService
        {
        public:
            MyService() noexcept: SKL::WorkerService{ 1 }{ }

        protected:
            SKL::RStatus Initialize() noexcept override
            {
                SKL_ASSERT_ALLWAYS( 1 == ++SeqCounter );
                return SKL::RSuccess;
            }

            void OnServerStarted() noexcept override
            {
                SKL_ASSERT_ALLWAYS( 2 == ++SeqCounter );
            }
    
            void OnServerStopped() noexcept override
            {
                SKL_ASSERT_ALLWAYS( 4 == ++SeqCounter );
                SKL_ASSERT_ALLWAYS( CWorkersCount == DoneCount.load_acquire() );
            }
    
            void OnServerStopSignaled() noexcept override
            {
                SKL_ASSERT_ALLWAYS( 3 == ++SeqCounter );
                SKL_ASSERT_ALLWAYS( CWorkersCount == DoneCount.load_acquire() );
            }

            //! [Callback] Each time a worker started
            void OnWorkerStarted( SKL::Worker& InWorker, SKL::WorkerGroup& InWorkerGroup ) noexcept override
            {
                TLSCounter::SetValue( 0 );
            }

            //! [Callback] Each time a worker stopped
            void OnWorkerStopped( SKL::Worker& InWorker, SKL::WorkerGroup& InWorkerGroup ) noexcept override
            {
                const auto Value{ TLSCounter::GetValue() };
                if ( CIterCount != Value )
                {
                    SKL_BREAK();
                }

                SKL_ASSERT_ALLWAYS( CIterCount == Value );
                TLSCounter::SetValue( 0 );
            }

            //! [Callback] Tick for each active worker
            void OnTickWorker( SKL::Worker& InWorker, SKL::WorkerGroup& InWorkerGroup ) noexcept override
            {
                const auto LastValue{ TLSCounter::GetValue() };

                std::this_thread::yield();
                std::this_thread::yield();

                SKL_ASSERT_ALLWAYS( LastValue == TLSCounter::GetValue() );

                if( CIterCount == LastValue + 1 )
                {
                    TLSCounter::SetValue( LastValue + 1 );
                    if( CWorkersCount == DoneCount.increment() + 1 )
                    {
                        GetServerInstance().SignalToStop( true );
                    }
                }
                else
                {
                    SKL_ASSERT_ALLWAYS( LastValue + 1 <= CIterCount );
                    TLSCounter::SetValue( LastValue + 1 );
                }

            }

            int32_t                     SeqCounter{ 0 };
            std::synced_value<uint32_t> DoneCount { 0 };
        };

        WorkerService_TestsFixture()
            : ::testing::Test(),
              TestApplication( L"AOD_TESTS_APP" ) {}      
  
        bool OnAddServices() noexcept override
        {
            SKL_ASSERT_ALLWAYS( true == AddService( new MyService() ) );
            return true;
        }

        void SetUp() override
        {
            ASSERT_TRUE( SKL::RSuccess == SKL::Skylake_InitializeLibrary( 0, nullptr, nullptr ) );
        }

        void TearDown() override
        {
            ASSERT_TRUE( SKL::RSuccess == SKL::Skylake_TerminateLibrary() );
        }
    };

    TEST_F( SimpleService_TestsFixture, SimpleService_BasicAPI )
    {
        const auto TotalAllocationsBefore{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsBefore{ SKL::GlobalMemoryManager::TotalDeallocations.load() };

        ASSERT_TRUE( true == AddNewWorkerGroup( SKL::WorkerGroupTag {
            .TickRate                        = 30, 
            .SyncTLSTickRate                 = 0,
            .Id                              = 1,
            .WorkersCount                    = 2,
            .bIsActive                       = true,
            .bHandlesTasks                   = false,
            .bSupportsAOD                    = true,
            .bHandlesTimerTasks              = true,
            .bSupportsTLSSync                = false,
            .bHasThreadLocalMemoryManager    = true,
            .bPreallocateAllThreadLocalPools = false,
            .bSupportesTCPAsyncAcceptors     = false,
            .bCallTickHandler                = false,
            .Name                            = L"SimpleService_BasicAPI_ACTIVE"
        }, []( SKL::Worker& , SKL::WorkerGroup& ) noexcept -> void {} ) );

        ASSERT_TRUE( true == Start( true ) );
        JoinAllGroups();

        const auto TotalAllocationsAfter{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsAfter{ SKL::GlobalMemoryManager::TotalDeallocations.load() };
        ASSERT_TRUE( TotalAllocationsBefore + 1 == TotalAllocationsAfter );
        ASSERT_TRUE( TotalDeallocationsBefore + 1 == TotalDeallocationsAfter );
    }

    TEST_F( AODService_TestsFixture, AODService_BasicAPI )
    {
        const auto TotalAllocationsBefore{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsBefore{ SKL::GlobalMemoryManager::TotalDeallocations.load() };

        ASSERT_TRUE( true == AddNewWorkerGroup( SKL::WorkerGroupTag {
            .TickRate                        = 30, 
            .SyncTLSTickRate                 = 0,
            .Id                              = 1,
            .WorkersCount                    = 2,
            .bIsActive                       = true,
            .bHandlesTasks                   = false,
            .bSupportsAOD                    = true,
            .bHandlesTimerTasks              = true,
            .bSupportsTLSSync                = false,
            .bHasThreadLocalMemoryManager    = true,
            .bPreallocateAllThreadLocalPools = false,
            .bSupportesTCPAsyncAcceptors     = false,
            .bCallTickHandler                = false,
            .Name                            = L"AODService_BasicAPI_ACTIVE"
        }, []( SKL::Worker& , SKL::WorkerGroup& ) noexcept -> void {} ) );

        ASSERT_TRUE( true == Start( true ) );
        JoinAllGroups();

        const auto TotalAllocationsAfter{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsAfter{ SKL::GlobalMemoryManager::TotalDeallocations.load() };
        ASSERT_TRUE( TotalAllocationsBefore + 2 == TotalAllocationsAfter );
        ASSERT_TRUE( TotalDeallocationsBefore + 2 == TotalDeallocationsAfter );
    }

    TEST_F( ActiveService_TestsFixture, ActiveService_BasicAPI )
    {
        const auto TotalAllocationsBefore{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsBefore{ SKL::GlobalMemoryManager::TotalDeallocations.load() };

        ASSERT_TRUE( true == AddNewWorkerGroup( SKL::WorkerGroupTag {
            .TickRate                        = 60, 
            .SyncTLSTickRate                 = 0,
            .Id                              = 1,
            .WorkersCount                    = 2,
            .bIsActive                       = true,
            .bHandlesTasks                   = false,
            .bSupportsAOD                    = true,
            .bHandlesTimerTasks              = true,
            .bSupportsTLSSync                = false,
            .bHasThreadLocalMemoryManager    = true,
            .bPreallocateAllThreadLocalPools = false,
            .bSupportesTCPAsyncAcceptors     = false,
            .bCallTickHandler                = false,
            .Name                            = L"ActiveService_BasicAPI_ACTIVE"
        }, []( SKL::Worker& , SKL::WorkerGroup& ) noexcept -> void {} ) );

        ASSERT_TRUE( true == Start( true ) );
        JoinAllGroups();

        const auto TotalAllocationsAfter{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsAfter{ SKL::GlobalMemoryManager::TotalDeallocations.load() };
        ASSERT_TRUE( TotalAllocationsBefore + 1 == TotalAllocationsAfter );
        ASSERT_TRUE( TotalDeallocationsBefore + 1 == TotalDeallocationsAfter );
    }

    TEST_F( WorkerService_TestsFixture, WorkerService_BasicAPI )
    {
        const auto TotalAllocationsBefore{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsBefore{ SKL::GlobalMemoryManager::TotalDeallocations.load() };

        ASSERT_TRUE( true == AddNewWorkerGroup( SKL::WorkerGroupTag {
            .TickRate                        = CTickRate, 
            .SyncTLSTickRate                 = 0,
            .Id                              = 1,
            .WorkersCount                    = CWorkersCount,
            .bIsActive                       = true,
            .bHandlesTasks                   = false,
            .bSupportsAOD                    = true,
            .bHandlesTimerTasks              = true,
            .bSupportsTLSSync                = false,
            .bHasThreadLocalMemoryManager    = true,
            .bPreallocateAllThreadLocalPools = false,
            .bSupportesTCPAsyncAcceptors     = false,
            .bCallTickHandler                = false,
            .Name                            = L"ActiveService_BasicAPI_ACTIVE"
        }, []( SKL::Worker& , SKL::WorkerGroup& ) noexcept -> void {} ) );

        ASSERT_TRUE( true == Start( true ) );
        JoinAllGroups();

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