#include <gtest/gtest.h>

#include <SkylakeLib.h>
#include <ApplicationSetup.h>

namespace AODTests
{
    class SimpleService_TestsFixture : public ::testing::Test, public TestApplication
    {
    public:
        class MySimpleService: public SKL::SimpleService
        {
        public:
            MySimpleService() noexcept: SKL::SimpleService{ 1 }{ }

        protected:
            RStatus Initialize() noexcept override
            {
                SKL_ASSERT_ALLWAYS( 1 == ++SeqCounter );
                return RSuccess;
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
            ASSERT_TRUE( RSuccess == SKL::Skylake_InitializeLibrary( 0, nullptr, nullptr ) );
        }

        void TearDown() override
        {
            ASSERT_TRUE( RSuccess == SKL::Skylake_TerminateLibrary() );
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
            RStatus Initialize() noexcept override
            {
                SKL_ASSERT_ALLWAYS( 1 == ++SeqCounter );
                return RSuccess;
            }

            void OnServerStarted() noexcept override
            {
                SKL_ASSERT_ALLWAYS( 2 == ++SeqCounter );

                DoAsync([ this ]() noexcept -> void 
                {
                    GetServerInstance().SignalToStop();
                } );
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
            ASSERT_TRUE( RSuccess == SKL::Skylake_InitializeLibrary( 0, nullptr, nullptr ) );
        }

        void TearDown() override
        {
            ASSERT_TRUE( RSuccess == SKL::Skylake_TerminateLibrary() );
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
            .Name                            = L"SimpleService_BasicAPI_ACTIVE"
        }, []( SKL::Worker& , SKL::WorkerGroup& ) noexcept -> void {} ) );

        ASSERT_TRUE( true == Start( true ) );
        JoinAllGroups();

        const auto TotalAllocationsAfter{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsAfter{ SKL::GlobalMemoryManager::TotalDeallocations.load() };
        ASSERT_TRUE( TotalAllocationsBefore + 1 == TotalAllocationsAfter );
        ASSERT_TRUE( TotalDeallocationsBefore + 1 == TotalDeallocationsAfter );
    }
}

int main( int argc, char** argv )
{
    testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS( );
}