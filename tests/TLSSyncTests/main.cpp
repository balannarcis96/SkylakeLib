#include <gtest/gtest.h>

#include <SkylakeLib.h>
#include <ApplicationSetup.h>

namespace TLSSyncTests
{
    class TLSSyncTestsFixture : public ::testing::Test, public TestApplication
    {
    public:
        static constexpr uint32_t WorkersCount{ 4 };
        using TLSCustomVal = SKL::TLSValue<int32_t, 0, TLSSyncTestsFixture>;

        struct MyService : SKL::WorkerService
        {
            MyService() noexcept : SKL::WorkerService{ 1 } {}

            RStatus Initialize() noexcept override 
            {
                return RSuccess;
            }

            void OnServerStarted() noexcept override 
            {
                
            }

            void OnServerStopped() noexcept override 
            {
            
            }

            void OnServerStopSignaled() noexcept override 
            {
            
            }

            void OnWorkerStarted( SKL::Worker& InWorker, SKL::WorkerGroup& InWorkerGroup ) noexcept override
            {
                TLSCustomVal::SetValue( 0 );
            }

            void OnWorkerStopped( SKL::Worker& InWorker, SKL::WorkerGroup& InWorkerGroup ) noexcept override
            {
                SKL_ASSERT_ALLWAYS( WorkersCount == Counter.load_relaxed() );
            }

            void OnTickWorker( SKL::Worker& InWorker, SKL::WorkerGroup& InWorkerGroup ) noexcept override
            {
                if( 1 == TLSCustomVal::GetValue() )
                {
                    if( WorkersCount + 1 == Counter.increment() + 1 )
                    {
                        GetServerInstance().SignalToStop( true );
                    }

                    TLSCustomVal::SetValue( 2 );
                }
            }

            std::relaxed_value<uint32_t> Counter{ 0 };
        };

        TLSSyncTestsFixture()
            : ::testing::Test(),
              TestApplication( L"TLSSYNC_TESTS_APP" ) {}      
  
        bool OnServerStarted() noexcept override
        {
            if( false == TestApplication::OnServerStarted() )
            {
                return false;
            }

            SyncTSLOnAllWorkerGroups( [ this ]( SKL::Worker& InWorker, SKL::WorkerGroup& InGroup, bool bIsFinalzation ) noexcept -> void 
            {
                if( false == bIsFinalzation )
                {
                    TLSCustomVal::SetValue( 1 );
                }   
                else
                {
                    auto* Service{ GetWorkerServiceById<MyService>( 1 ) };
                    EXPECT_TRUE( nullptr != Service );

                    if( WorkersCount + 1 == Service->Counter.increment() + 1 )
                    {
                        SignalToStop( true );
                    }
                }
            } );

            return true;
        }

        bool OnAddServices() noexcept override
        {
            if( false == TestApplication::OnAddServices() )
            {
                return false;
            }

            AddService( new MyService() );

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

    TEST_F( TLSSyncTestsFixture, TLSSync_WorkerGroup )
    {
        const auto TotalAllocationsBefore{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsBefore{ SKL::GlobalMemoryManager::TotalDeallocations.load() };

        ASSERT_TRUE( true == AddNewWorkerGroup( SKL::WorkerGroupTag {
            .TickRate                        = 160, 
            .SyncTLSTickRate                 = 0,
            .Id                              = 1,
            .WorkersCount                    = 16,
            .bIsActive                       = true,
            .bHandlesTasks                   = true,
            .bSupportsAOD                    = true,
            .bHandlesTimerTasks              = true,
            .bSupportsTLSSync                = false,
            .bHasThreadLocalMemoryManager    = true,
            .bPreallocateAllThreadLocalPools = false,
            .bSupportesTCPAsyncAcceptors     = false,
            .bCallTickHandler                = true,
            .Name                            = L"AODOBJECTSINGLETHREAD_GROUP"
        }, []( SKL::Worker& InWorker, SKL::WorkerGroup& InGroup ) mutable noexcept -> void {  } ) );

        ASSERT_TRUE( true == Start( true ) );

        const auto TotalAllocationsAfter{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsAfter{ SKL::GlobalMemoryManager::TotalDeallocations.load() };
        const auto CustomSizeAllocations{ SKL::GlobalMemoryManager::CustomSizeAllocations.load() };
        const auto CustomSizeDeallocations{ SKL::GlobalMemoryManager::CustomSizeDeallocations.load() };
        ASSERT_TRUE( TotalAllocationsBefore + 1 == TotalAllocationsAfter );
        ASSERT_TRUE( TotalDeallocationsBefore + 1 == TotalDeallocationsAfter );
    }
}

int main( int argc, char** argv )
{
    testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS( );
}