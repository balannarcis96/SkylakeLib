#include <gtest/gtest.h>

#include <SkylakeLib.h>
#include <ApplicationSetup.h>

namespace AODTests
{
    class AODStandaloneFixture: public ::testing::Test
    {
    public:
        AODStandaloneFixture()
            :ServerInstanceConfig{ L"TEMP" } {}

        void SetUp() override
        {
            ASSERT_TRUE( RSuccess == SKL::Skylake_InitializeLibrary( 0, nullptr, nullptr ) );

            ServerInstanceConfig.AddNewGroup( SKL::WorkerGroupTag{  .Id = 1, .WorkersCount = 1, .bHandlesTasks = true, .Name = L"TEMP" } );        
            ASSERT_TRUE( RSuccess == ServerInstance.Initialize( std::move( ServerInstanceConfig ) ) );

            ASSERT_TRUE( RSuccess == SKL::ServerInstanceTLSContext::Create( &ServerInstance, SKL::WorkerGroupTag{ .Id = 1, .WorkersCount = 1, .bHandlesTasks = true, .Name = L"TEMP" } ) );
            ASSERT_TRUE( RSuccess == SKL::AODTLSContext::Create( &ServerInstance, SKL::WorkerGroupTag{ .Id = 1, .WorkersCount = 1, .bHandlesTasks = true, .Name = L"TEMP" } ) );
        }

        void TearDown() override
        {
            ServerInstance.SignalToStop( true );
            ServerInstance.JoinAllGroups();

            SKL::AODTLSContext::Destroy();
            SKL::ServerInstanceTLSContext::Destroy();

            ASSERT_TRUE( RSuccess == SKL::Skylake_TerminateLibrary() );
        }

        SKL::ServerInstanceConfig::ServerInstanceConfig ServerInstanceConfig;
        SKL::ServerInstance                             ServerInstance;
    };

    class AODTestsFixture : public ::testing::Test, public TestApplication
    {
    public:
        AODTestsFixture()
            : ::testing::Test(),
              TestApplication( L"AOD_TESTS_APP" ) {}      
  
        void SetUp() override
        {
            ASSERT_TRUE( RSuccess == SKL::Skylake_InitializeLibrary( 0, nullptr, nullptr ) );
        }

        void TearDown() override
        {
            ASSERT_TRUE( RSuccess == SKL::Skylake_TerminateLibrary() );
        }

        bool Initialize() override
        {
            return true;                        
        }

        int asdasd = 0;
    };

    TEST_F( AODStandaloneFixture, AODObjectSingleThread )
    {
        struct MyObject : SKL::AODObject
        {
            int a { 0 };
        };
        
        auto obj = SKL::MakeShared<MyObject>();

        ASSERT_TRUE( nullptr != obj.get() );
        ASSERT_TRUE( 0 == obj->a );
        const auto TotalAllocationsBefore{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsBefore{ SKL::GlobalMemoryManager::TotalDeallocations.load() };
        
        obj->DoAsync( []( SKL::AODObject& Obj ) noexcept -> void
        {
            auto& Self = reinterpret_cast<MyObject&>( Obj );
        
            Self.a = 55;
        } );

        ASSERT_TRUE( 55 == obj->a );

        SKL::AODTLSContext::Destroy();

        const auto TotalAllocationsAfter{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsAfter{ SKL::GlobalMemoryManager::TotalDeallocations.load() };
        ASSERT_TRUE( TotalAllocationsBefore + 1 == TotalAllocationsAfter );
        ASSERT_TRUE( TotalDeallocationsBefore + 1 == TotalDeallocationsAfter );
    }

    TEST_F( AODStandaloneFixture, AODObjectSingleThread_MultipleCalls )
    {
        struct MyObject : SKL::AODObject
        {
            int a { 0 };
        };
        
        auto obj = SKL::MakeShared<MyObject>();

        ASSERT_TRUE( nullptr != obj.get() );
        ASSERT_TRUE( 0 == obj->a );
        const auto TotalAllocationsBefore{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsBefore{ SKL::GlobalMemoryManager::TotalDeallocations.load() };
        
        for( int i = 0; i < 50; ++i )
        {
            obj->DoAsync( [i]( SKL::AODObject& Obj ) noexcept -> void
            {
                auto& Self = reinterpret_cast<MyObject&>( Obj );
        
                Self.a = i;
            } );
        }

        ASSERT_TRUE( 49 == obj->a );

        SKL::AODTLSContext::Destroy();

        const auto TotalAllocationsAfter{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsAfter{ SKL::GlobalMemoryManager::TotalDeallocations.load() };
        ASSERT_TRUE( TotalAllocationsBefore + 50 == TotalAllocationsAfter );
        ASSERT_TRUE( TotalDeallocationsBefore + 50 == TotalDeallocationsAfter );
    }

    TEST_F( AODTestsFixture, AODObjectMultipleWorkers )
    {
        struct MyObject : SKL::AODObject
        {
            uint64_t a { 0 };
        };
        
        auto obj = SKL::MakeShared<MyObject>();
        ASSERT_TRUE( nullptr != obj.get() );
        ASSERT_TRUE( 0 == obj->a );
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
        }, [ Ptr = obj.get() ]( SKL::Worker& InWorker, SKL::WorkerGroup& InGroup ) mutable noexcept -> void
        {
            for( uint64_t i = 0; i < 100000; ++i )
            {
                Ptr->DoAsync( []( SKL::AODObject& InObj ) noexcept -> void
                {
                    auto& Self = reinterpret_cast<MyObject&>( InObj );
                    ++Self.a;
                } );
            }

            InGroup.GetServerInstance()->SignalToStop( true );
        } ) );

        ASSERT_TRUE( true == Start( true ) );

        ASSERT_TRUE( 16 * 100000 == obj->a );
        const auto TotalAllocationsAfter{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsAfter{ SKL::GlobalMemoryManager::TotalDeallocations.load() };
        const auto CustomSizeAllocations{ SKL::GlobalMemoryManager::CustomSizeAllocations.load() };
        const auto CustomSizeDeallocations{ SKL::GlobalMemoryManager::CustomSizeDeallocations.load() };
        ASSERT_TRUE( TotalAllocationsBefore + 16 * 100000 == TotalAllocationsAfter );
        ASSERT_TRUE( TotalDeallocationsBefore + 16 * 100000 == TotalDeallocationsAfter );
    }

    TEST_F( AODTestsFixture, AODObjectMultipleWorkers_OneDeferedTask )
    {
        struct MyObject : SKL::AODObject
        {
            std::relaxed_value<int32_t> bShouldStop{ FALSE };
        };
        
        auto obj = SKL::MakeShared<MyObject>();
        ASSERT_TRUE( nullptr != obj.get() );
        ASSERT_TRUE( FALSE == obj->bShouldStop.load_acquire() );
        const auto TotalAllocationsBefore{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsBefore{ SKL::GlobalMemoryManager::TotalDeallocations.load() };
        std::relaxed_value<int32_t> bHasCreatedTask{ FALSE };

        auto OnTick = [ Ptr = obj.get(), &bHasCreatedTask ]( SKL::Worker& InWorker, SKL::WorkerGroup& InGroup ) mutable noexcept -> void
        {
            if( true == InWorker.IsMaster() && FALSE == bHasCreatedTask.load_acquire() )
            {
                bHasCreatedTask.exchange( TRUE );

                ASSERT_TRUE( RSuccess == Ptr->DoAsyncAfter( 1000, []( SKL::AODObject& InObject ) noexcept -> void 
                {
                    auto& Self = reinterpret_cast<MyObject&>( InObject );
                    SKL_INF( "################# stop #################" );
                    Self.bShouldStop.exchange( TRUE );
                } ) );
            }

            if( TRUE == Ptr->bShouldStop.exchange( FALSE ) )
            {
                InGroup.GetServerInstance()->SignalToStop( true );
            }
        } ;

        ASSERT_TRUE( true == AddNewWorkerGroup( SKL::WorkerGroupTag {
            .TickRate                        = 60, 
            .SyncTLSTickRate                 = 0,
            .Id                              = 1,
            .WorkersCount                    = 4,
            .bIsActive                       = true,
            .bHandlesTasks                   = false,
            .bSupportsAOD                    = true,
            .bHandlesTimerTasks              = true,
            .bSupportsTLSSync                = false,
            .bHasThreadLocalMemoryManager    = true,
            .bPreallocateAllThreadLocalPools = false,
            .bSupportesTCPAsyncAcceptors     = false,
            .bCallTickHandler                = true,
            .Name                            = L"AODOBJECTSINGLETHREAD_GROUP"
        }, std::move( OnTick ) ) );

        ASSERT_TRUE( true == Start( true ) );

        ASSERT_TRUE( FALSE == obj->bShouldStop.load_acquire() );
        const auto TotalAllocationsAfter{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsAfter{ SKL::GlobalMemoryManager::TotalDeallocations.load() };
        ASSERT_TRUE( TotalAllocationsBefore + 1 == TotalAllocationsAfter );
        ASSERT_TRUE( TotalDeallocationsBefore + 1 == TotalDeallocationsAfter );
    }

    TEST_F( AODTestsFixture, AODObjectMultipleWorkers_MultipleDeferedTasks )
    {
        struct MyObject : SKL::AODObject
        {
            std::relaxed_value<int32_t> bShouldStop{ 10000 };
        };
        
        auto obj = SKL::MakeShared<MyObject>();
        ASSERT_TRUE( nullptr != obj.get() );
        ASSERT_TRUE( 10000 == obj->bShouldStop );
        const auto TotalAllocationsBefore{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsBefore{ SKL::GlobalMemoryManager::TotalDeallocations.load() };
        bool bHasCreatedTask{ false };

        auto OnTick = [ Ptr = obj.get(), &bHasCreatedTask ]( SKL::Worker& InWorker, SKL::WorkerGroup& InGroup ) mutable noexcept -> void
        {
            if( true == InWorker.IsMaster() && false == bHasCreatedTask )
            {
                bHasCreatedTask = true;

                for( auto i = 0; i < 10000; ++i )
                {
                    ASSERT_TRUE( RSuccess == Ptr->DoAsyncAfter( 10, []( SKL::AODObject& InObject ) noexcept -> void 
                    {
                        auto& Self = reinterpret_cast<MyObject&>( InObject );
                        if( 1 == Self.bShouldStop.decrement() )
                        {
                            SKL_INF( "############# LAST DECREMENT #############" );
                        }
                    } ) );
                }
            }       

            if( 0 == Ptr->bShouldStop.load_acquire() )
            {
                InGroup.GetServerInstance()->SignalToStop( true );
            }
        } ;

        ASSERT_TRUE( true == AddNewWorkerGroup( SKL::WorkerGroupTag {
            .TickRate                        = 60, 
            .SyncTLSTickRate                 = 0,
            .Id                              = 1,
            .WorkersCount                    = 4,
            .bIsActive                       = true,
            .bHandlesTasks                   = false,
            .bSupportsAOD                    = true,
            .bHandlesTimerTasks              = true,
            .bSupportsTLSSync                = false,
            .bHasThreadLocalMemoryManager    = true,
            .bPreallocateAllThreadLocalPools = false,
            .bSupportesTCPAsyncAcceptors     = false,
            .bCallTickHandler                = true,
            .Name                            = L"AODOBJECTSINGLETHREAD_GROUP"
        }, std::move( OnTick ) ) );

        ASSERT_TRUE( true == Start( true ) );

        ServerInstance.JoinAllGroups();
        ASSERT_TRUE( FALSE == obj->bShouldStop );
        const auto TotalAllocationsAfter{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsAfter{ SKL::GlobalMemoryManager::TotalDeallocations.load() };
        ASSERT_TRUE( TotalAllocationsBefore + 10000 == TotalAllocationsAfter );
        ASSERT_TRUE( TotalDeallocationsBefore + 10000 == TotalDeallocationsAfter );
    }
}

int main( int argc, char** argv )
{
    testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS( );
}