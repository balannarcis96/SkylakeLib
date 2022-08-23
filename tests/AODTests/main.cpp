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

            ASSERT_TRUE( RSuccess == SKL::ServerInstanceTLSContext::Create( &ServerInstance, SKL::WorkerGroupTag{ 
                .Id = 1, 
                .WorkersCount = 1, 
                .bHandlesTasks = true, 
                .Name = L"TEMP" 
            } ) );
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
    };

    class AODTestsFixture2 : public ::testing::Test, public TestApplication
    {
    public:
        static constexpr uint64_t IterCount{ 10000 };
        static inline std::relaxed_value<uint64_t> Counter{ IterCount };

        AODTestsFixture2()
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

        bool OnAllWorkersStarted( SKL::WorkerGroup& InGroup ) noexcept override
        {
            if ( false == TestApplication::OnAllWorkersStarted( InGroup ) )
            {
                return false;
            }

            if( InGroup.GetTag().Id == 1 )
            {
                SKL_ASSERT_ALLWAYS( true == InGroup.GetTag().bHandlesTasks );
                InGroup.Deferre( [ this ]() noexcept -> void 
                {
                    for( uint64_t i = 0; i < IterCount; ++i )
                    {
                        SKL::DeferTask( [ this ]() noexcept -> void 
                        {
                            Counter.decrement();
                        } );
                    }
                } );
            }

            return true;
        }
    };

    class AODTestsFixture3 : public ::testing::Test, public TestApplication
    {
    public:
        static constexpr uint64_t IterCount{ 10000 };
        static inline std::atomic<int64_t> Counter{ IterCount };

        AODTestsFixture3()
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

        bool OnAllWorkersStarted( SKL::WorkerGroup& InGroup ) noexcept override
        {
            if ( false == TestApplication::OnAllWorkersStarted( InGroup ) )
            {
                return false;
            }

            SKL_ASSERT_ALLWAYS( true == InGroup.GetTag().bHandlesTasks );
            for( uint64_t i = 0; i < IterCount; ++i )
            {
                int32_t ExecCounter = 0;
                InGroup.Deferre( [ this, ExecCounter ]() mutable noexcept -> void 
                {
                    SKL_ASSERT_ALLWAYS( 0 == ExecCounter++ );

                    const auto NewCounter{ Counter.fetch_sub( 1 ) };
                    if ( 1 == NewCounter )
                    {
                        SignalToStop( true );
                    }
                } );
            }

            return true;
        }
    };

    class AODTestsFixture4 : public ::testing::Test, public TestApplication
    {
    public:
        static constexpr uint64_t IterCount{ 10000 };

        struct MyObject : SKL::AODObject
        {
            int Counter { IterCount };

            MyObject() noexcept = default;
            ~MyObject() noexcept
            {
                SKL_ASSERT_ALLWAYS( 0 == Counter );
                SKL_INF( "AODTestsFixture4::MyObject::~MyObject()" );
            }
        };
        
        AODTestsFixture4()
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

        bool OnAllWorkersStarted( SKL::WorkerGroup& InGroup ) noexcept override
        {
            if ( false == TestApplication::OnAllWorkersStarted( InGroup ) )
            {
                return false;
            }

            if( false == InGroup.GetTag().bIsActive )
            {
                auto obj = SKL::MakeShared<MyObject>();

                SKL_ASSERT_ALLWAYS( true == InGroup.GetTag().bHandlesTasks );
                for( uint64_t i = 0; i < IterCount; ++i )
                {
                    SKL_ASSERT_ALLWAYS( RSuccess == obj->DoAsyncAfter( 5, [ &InGroup ]( SKL::AODObject& InObj ) noexcept -> void 
                    {
                        auto& Self = reinterpret_cast<MyObject&>( InObj );
                        if( 0 == --Self.Counter )           
                        {
                            InGroup.GetServerInstance()->SignalToStop( true );
                        }
                    } ) );
                }

                obj.reset();
            }

            return true;
        }
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

    TEST_F( AODTestsFixture, AODObjectMultipleSymetricWorkers )
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

        constexpr uint64_t IterCount{ 10000 };

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
            for( uint64_t i = 0; i < IterCount; ++i )
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

        ASSERT_TRUE( 16 * IterCount == obj->a );
        const auto TotalAllocationsAfter{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsAfter{ SKL::GlobalMemoryManager::TotalDeallocations.load() };
        const auto CustomSizeAllocations{ SKL::GlobalMemoryManager::CustomSizeAllocations.load() };
        const auto CustomSizeDeallocations{ SKL::GlobalMemoryManager::CustomSizeDeallocations.load() };
        ASSERT_TRUE( TotalAllocationsBefore + 16 * IterCount == TotalAllocationsAfter );
        ASSERT_TRUE( TotalDeallocationsBefore + 16 * IterCount == TotalDeallocationsAfter );
    }

    TEST_F( AODTestsFixture, AODObjectMultipleSymetricWorkers_OneDeferedTask )
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

    TEST_F( AODTestsFixture, AODObjectMultipleSymetricWorkers_MultipleDeferedTasks )
    {
        struct MyObject : SKL::AODObject
        {
            std::relaxed_value<int32_t> Counter{ 2000 };
        };
        
        auto obj = SKL::MakeShared<MyObject>();
        ASSERT_TRUE( nullptr != obj.get() );
        ASSERT_TRUE( 2000 == obj->Counter );
        const auto TotalAllocationsBefore{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsBefore{ SKL::GlobalMemoryManager::TotalDeallocations.load() };
        bool bHasCreatedTask{ false };

        auto OnTick = [ Ptr = obj.get(), &bHasCreatedTask ]( SKL::Worker& InWorker, SKL::WorkerGroup& InGroup ) mutable noexcept -> void
        {
            if( true == InWorker.IsMaster() && false == bHasCreatedTask )
            {
                bHasCreatedTask = true;

                for( auto i = 0; i < 2000; ++i )
                {
                    ASSERT_TRUE( RSuccess == Ptr->DoAsyncAfter( 5, []( SKL::AODObject& InObject ) noexcept -> void 
                    {
                        auto& Self = reinterpret_cast<MyObject&>( InObject );
                        if( 1 == Self.Counter.decrement() )
                        {
                            SKL_INF( "############# LAST DECREMENT #############" );
                        }
                    } ) );
                }
            }       

            if( 0 == Ptr->Counter.load_acquire() )
            {
                InGroup.GetServerInstance()->SignalToStop( true );
            }
        };

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
            .Name                            = L"AODObjectMultipleSymetricWorkers_MultipleDeferedTasks_ACTIVE"
        }, std::move( OnTick ) ) );

        ASSERT_TRUE( true == Start( true ) );

        JoinAllGroups();
        ASSERT_TRUE( 0 == obj->Counter );
        const auto TotalAllocationsAfter{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsAfter{ SKL::GlobalMemoryManager::TotalDeallocations.load() };
        ASSERT_TRUE( TotalAllocationsBefore + 2000 == TotalAllocationsAfter );
        ASSERT_TRUE( TotalDeallocationsBefore + 2000 == TotalDeallocationsAfter );
    }

    TEST_F( AODTestsFixture, AODObjectReactiveAndActiveWorkers_ShutdownNotice )
    {
        const auto TotalAllocationsBefore{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsBefore{ SKL::GlobalMemoryManager::TotalDeallocations.load() };
        bool bHasCreatedTask{ false };

        auto OnTick = [ &bHasCreatedTask ]( SKL::Worker& InWorker, SKL::WorkerGroup& InGroup ) mutable noexcept -> void
        {
            if( true == InWorker.IsMaster() && false == bHasCreatedTask )
            {
                bHasCreatedTask = true;

                SKL::DeferTask( [ &InGroup ]() noexcept -> void 
                {
                    SKL_INF( "FROM TASK" );
                    InGroup.GetServerInstance()->SignalToStop( true );
                } );
            }
        };

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
            .bCallTickHandler                = true,
            .Name                            = L"AODObjectMultipleWorkers_MultipleDeferedTasks_ACTIVE"
        }, std::move( OnTick ) ) );

        ASSERT_TRUE( true == Start( true ) );
        JoinAllGroups();

        const auto TotalAllocationsAfter{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsAfter{ SKL::GlobalMemoryManager::TotalDeallocations.load() };
        ASSERT_TRUE( TotalAllocationsBefore + 1 == TotalAllocationsAfter );
        ASSERT_TRUE( TotalDeallocationsBefore + 1 == TotalDeallocationsAfter );
    }

    TEST_F( AODTestsFixture, AODObjectReactiveAndActiveWorkers_HeavyGlobalDefer )
    {
        const auto TotalAllocationsBefore{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsBefore{ SKL::GlobalMemoryManager::TotalDeallocations.load() };
        bool bHasCreatedTask{ false };

        constexpr uint64_t IterCount{ 10000 };

        alignas( SKL_CACHE_LINE_SIZE ) std::relaxed_value<uint64_t> Counter{ IterCount };

        auto OnTick = [ &bHasCreatedTask, &Counter ]( SKL::Worker& InWorker, SKL::WorkerGroup& InGroup ) mutable noexcept -> void
        {
            if( true == InWorker.IsMaster() && false == bHasCreatedTask )
            {
                bHasCreatedTask = true;

                for( uint64_t i = 0; i < IterCount; ++i )
                {
                    SKL::DeferTask( [ &Counter ]() noexcept -> void 
                    {
                        Counter.decrement();
                    } );
                }
            }

            if( 0 == Counter.load_relaxed() )
            {
                InGroup.GetServerInstance()->SignalToStop( true );
            }
        };
      
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
            .bCallTickHandler                = true,
            .Name                            = L"AODObjectReactiveAndActiveWorkers_HeavyGlobalDefer_ACTIVE"
        }, std::move( OnTick ) ) );

        ASSERT_TRUE( true == Start( true ) );
        JoinAllGroups();

        const auto TotalAllocationsAfter{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsAfter{ SKL::GlobalMemoryManager::TotalDeallocations.load() };
        ASSERT_TRUE( TotalAllocationsBefore + IterCount == TotalAllocationsAfter );
        ASSERT_TRUE( TotalDeallocationsBefore + IterCount == TotalDeallocationsAfter );
    }
    
    TEST_F( AODTestsFixture2, AODObjectReactiveAndActiveWorkers_HeaveyGlobalDeferFromReactive )
    {
        const auto TotalAllocationsBefore{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsBefore{ SKL::GlobalMemoryManager::TotalDeallocations.load() };
        bool bHasCreatedTask{ false };

        auto OnTick = []( SKL::Worker& InWorker, SKL::WorkerGroup& InGroup ) mutable noexcept -> void
        {
            if( 0 == Counter.load_relaxed() )
            {
                InGroup.GetServerInstance()->SignalToStop( true );
            }
        };

        ASSERT_TRUE( true == AddNewWorkerGroup( SKL::WorkerGroupTag {
            .TickRate                        = 24, 
            .SyncTLSTickRate                 = 0,
            .Id                              = 1,
            .WorkersCount                    = 2,
            .bIsActive                       = false,
            .bHandlesTasks                   = true,
            .bSupportsAOD                    = false,
            .bHandlesTimerTasks              = false,
            .bSupportsTLSSync                = false,
            .bHasThreadLocalMemoryManager    = true,
            .bPreallocateAllThreadLocalPools = false,
            .bSupportesTCPAsyncAcceptors     = false,
            .bCallTickHandler                = false,
            .Name                            = L"AODObjectMultipleWorkers_MultipleDeferedTasks_REACTIVE"
        }, []( SKL::Worker& InWorker, SKL::WorkerGroup& InGroup ) noexcept -> void {} ) );
        ASSERT_TRUE( true == AddNewWorkerGroup( SKL::WorkerGroupTag {
            .TickRate                        = 30, 
            .SyncTLSTickRate                 = 0,
            .Id                              = 2,
            .WorkersCount                    = 2,
            .bIsActive                       = true,
            .bHandlesTasks                   = false,
            .bSupportsAOD                    = true,
            .bHandlesTimerTasks              = true,
            .bSupportsTLSSync                = false,
            .bHasThreadLocalMemoryManager    = true,
            .bPreallocateAllThreadLocalPools = false,
            .bSupportesTCPAsyncAcceptors     = false,
            .bCallTickHandler                = true,
            .Name                            = L"AODObjectMultipleWorkers_MultipleDeferedTasks_ACTIVE"
        }, std::move( OnTick ) ) );

        ASSERT_TRUE( true == Start( true ) );

        JoinAllGroups();
        const auto TotalAllocationsAfter{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsAfter{ SKL::GlobalMemoryManager::TotalDeallocations.load() };
        ASSERT_TRUE( TotalAllocationsBefore + IterCount + 1 == TotalAllocationsAfter );
        ASSERT_TRUE( TotalDeallocationsBefore + IterCount + 1 == TotalDeallocationsAfter );
    }

    TEST_F( AODTestsFixture3, AODObjectReactiveAndActiveWorkers_HeaveyGlobalDeferFromReactiveOnly )
    {
        const auto TotalAllocationsBefore{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsBefore{ SKL::GlobalMemoryManager::TotalDeallocations.load() };

        ASSERT_TRUE( true == AddNewWorkerGroup( SKL::WorkerGroupTag {
            .TickRate                        = 24, 
            .SyncTLSTickRate                 = 0,
            .Id                              = 1,
            .WorkersCount                    = 4,
            .bIsActive                       = false,
            .bHandlesTasks                   = true,
            .bSupportsAOD                    = false,
            .bHandlesTimerTasks              = false,
            .bSupportsTLSSync                = false,
            .bHasThreadLocalMemoryManager    = true,
            .bPreallocateAllThreadLocalPools = false,
            .bSupportesTCPAsyncAcceptors     = false,
            .bCallTickHandler                = false,
            .Name                            = L"AODObjectMultipleWorkers_MultipleDeferedTasks_REACTIVE"
        }, []( SKL::Worker& InWorker, SKL::WorkerGroup& InGroup ) noexcept -> void {} ) );

        ASSERT_TRUE( true == Start( true ) );

        JoinAllGroups();
        const auto TotalAllocationsAfter{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsAfter{ SKL::GlobalMemoryManager::TotalDeallocations.load() };
        ASSERT_TRUE( TotalAllocationsBefore + IterCount == TotalAllocationsAfter );
        ASSERT_TRUE( TotalDeallocationsBefore + IterCount == TotalDeallocationsAfter );
    }

    TEST_F( AODTestsFixture4, AODObjectReactiveAndActiveWorkers_AODDeferredFromReactive )
    {
        const auto TotalAllocationsBefore{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsBefore{ SKL::GlobalMemoryManager::TotalDeallocations.load() };

        ASSERT_TRUE( true == AddNewWorkerGroup( SKL::WorkerGroupTag {
            .TickRate                        = 24, 
            .SyncTLSTickRate                 = 0,
            .Id                              = 1,
            .WorkersCount                    = 2,
            .bIsActive                       = false,
            .bHandlesTasks                   = true,
            .bSupportsAOD                    = true,
            .bHandlesTimerTasks              = false,
            .bSupportsTLSSync                = false,
            .bHasThreadLocalMemoryManager    = true,
            .bPreallocateAllThreadLocalPools = false,
            .bSupportesTCPAsyncAcceptors     = false,
            .bCallTickHandler                = false,
            .Name                            = L"AODObjectMultipleWorkers_MultipleDeferedTasks_REACTIVE"
        }, []( SKL::Worker& InWorker, SKL::WorkerGroup& InGroup ) noexcept -> void {} ) );
        ASSERT_TRUE( true == AddNewWorkerGroup( SKL::WorkerGroupTag {
            .TickRate                        = 30, 
            .SyncTLSTickRate                 = 0,
            .Id                              = 2,
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
            .Name                            = L"AODObjectMultipleWorkers_MultipleDeferedTasks_ACTIVE"
        }, []( SKL::Worker& InWorker, SKL::WorkerGroup& InGroup ) noexcept -> void {} ) );
        ASSERT_TRUE( true == Start( true ) );

        JoinAllGroups();
        const auto TotalAllocationsAfter{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsAfter{ SKL::GlobalMemoryManager::TotalDeallocations.load() };
        ASSERT_TRUE( TotalAllocationsBefore + IterCount + 1 == TotalAllocationsAfter );
        ASSERT_TRUE( TotalDeallocationsBefore + IterCount + 1 == TotalDeallocationsAfter );
    }
}

int main( int argc, char** argv )
{
    testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS( );
}