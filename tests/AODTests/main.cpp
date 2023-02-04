#include <gtest/gtest.h>

#include <SkylakeLib.h>
#include <ApplicationSetup.h>

FILE* TestLogFile;

namespace AODTests
{
    class AODStandaloneFixture: public ::testing::Test
    {
    public:
        AODStandaloneFixture()
            :ServerInstanceConfig{ L"TEMP" } {}

        void SetUp() override
        {
            ASSERT_TRUE( SKL::RSuccess == SKL::Skylake_InitializeLibrary( 0, nullptr, nullptr ) );

            ServerInstanceConfig.AddNewGroup( SKL::WorkerGroupTag{  .Id = 1, .WorkersCount = 1, .bHandlesTasks = true, .Name = L"TEMP" } );        
            ASSERT_TRUE( SKL::RSuccess == ServerInstance.Initialize( std::move( ServerInstanceConfig ) ) );

            SKL::WorkerGroupTag TempTag{ 
                .Id = 1, 
                .WorkersCount = 1, 
                .bHandlesTasks = true, 
                .Name = L"TEMP" 
            };
            TempTag.Validate();

            ASSERT_TRUE( SKL::RSuccess == SKL::ThreadLocalMemoryManager::Create() );
            ASSERT_TRUE( SKL::RSuccess == SKL::ServerInstanceTLSContext::Create( &ServerInstance, TempTag ) );

            SKL::WorkerGroupTag TempTag2
            { 
                .Id = 1, 
                .WorkersCount = 1, 
                .bHandlesTasks = true, 
                .Name = L"TEMP" 
            };
            TempTag2.Validate();
            ASSERT_TRUE( SKL::RSuccess == SKL::AODTLSContext::Create( &ServerInstance, TempTag2 ) );
        }

        void TearDown() override
        {
            ServerInstance.SignalToStop( true );
            ServerInstance.JoinAllGroups();

            SKL::AODTLSContext::Destroy();
            SKL::ServerInstanceTLSContext::Destroy();
            SKL::ThreadLocalMemoryManager::FreeAllPools();
            SKL::ThreadLocalMemoryManager::Destroy();

            ASSERT_TRUE( SKL::RSuccess == SKL::Skylake_TerminateLibrary() );
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
            ASSERT_TRUE( SKL::RSuccess == SKL::Skylake_InitializeLibrary( 0, nullptr, nullptr ) );
        }

        void TearDown() override
        {
            ASSERT_TRUE( SKL::RSuccess == SKL::Skylake_TerminateLibrary() );
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
            ASSERT_TRUE( SKL::RSuccess == SKL::Skylake_InitializeLibrary( 0, nullptr, nullptr ) );
        }

        void TearDown() override
        {
            ASSERT_TRUE( SKL::RSuccess == SKL::Skylake_TerminateLibrary() );
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
                ( void )InGroup.Defer( [ /*this*/ ]( SKL::ITask* /*Self*/ ) noexcept -> void 
                {
                    for( uint64_t i = 0; i < IterCount; ++i )
                    {
                        SKL::DeferTask( [ /*this*/ ]( SKL::ITask* /*Self*/ ) noexcept -> void 
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
            ASSERT_TRUE( SKL::RSuccess == SKL::Skylake_InitializeLibrary( 0, nullptr, nullptr ) );
        }

        void TearDown() override
        {
            ASSERT_TRUE( SKL::RSuccess == SKL::Skylake_TerminateLibrary() );
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
                ( void )InGroup.Defer( [ this, ExecCounter ]( SKL::ITask* /*Self*/ ) mutable noexcept -> void 
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
        static constexpr uint64_t IterCount{ 50 };

        struct MyObject : SKL::AOD::SharedObject
        {
            int Counter { IterCount };

            MyObject() noexcept : SKL::AOD::SharedObject{ this } 
            {
                SKLL_TRACE();
            }
            ~MyObject() noexcept
            {
                SKLL_TRACE();
                SKL_ASSERT_ALLWAYS( 0 == Counter );
            }
        };
        
        AODTestsFixture4() noexcept
            : ::testing::Test(),
              TestApplication( L"AOD_TESTS_APP" ) 
        {
            SKLL_TRACE();
        }      

        ~AODTestsFixture4() noexcept
        {
            SKLL_TRACE();
        }

        void SetUp() override
        {
            ASSERT_TRUE( SKL::RSuccess == SKL::Skylake_InitializeLibrary( 0, nullptr, nullptr ) );
        }

        void TearDown() override
        {
            ASSERT_TRUE( SKL::RSuccess == SKL::Skylake_TerminateLibrary() );
        }

        bool OnAllWorkersStarted( SKL::WorkerGroup& InGroup ) noexcept override
        {
            SKLL_TRACE();

            if ( false == TestApplication::OnAllWorkersStarted( InGroup ) )
            {
                SKL_ASSERT( false );
                return false;
            }
            /*
            if( false == InGroup.GetTag().bIsActive )
            {
                auto obj = SKL::MakeShared<MyObject>();

                for( uint64_t i = 0; i < IterCount; ++i )
                {
                    const auto Result{ obj->DoAsyncAfter( 500, [ this, &InGroup ]( SKL::AOD::SharedObject& InObj ) noexcept -> void 
                    {
                        auto& Self = reinterpret_cast<MyObject&>( InObj );

                        SKLL_TRACE_MSG_FMT( "Before Counter:%d", Self.Counter );

                        auto incResult{ a.increment() };

                        if( 0 != incResult )
                        {
                            const auto ThreadId { std::this_thread::get_id() };
                            static_assert( sizeof( ThreadId ) >= sizeof( uint32_t ) );
                            const auto tId{ *reinterpret_cast<const uint32_t*>( &ThreadId ) };
                            SKLL_ERR_FMT( "ThreadId: %u", tId );
                        }

                        SKL_ASSERT( 0 == incResult );

                        const auto NewCounter = --Self.Counter;
                        
                        if( NewCounter == 5002 )
                        {
                            SKL_BREAK();
                        }

                        SKL_ASSERT( true == Counts.empty() || NewCounter != Counts.top() );

                        Counts.push( NewCounter );

                        if( 0 == NewCounter )           
                        {
                            InGroup.GetServerInstance()->SignalToStop( true );
                        }

                        SKLL_TRACE_MSG_FMT( "After Counter:%d", Self.Counter );

                        auto decResult{ a.decrement() };
                        
                        if( 1 != decResult )
                        {
                            const auto ThreadId { std::this_thread::get_id() };
                            static_assert( sizeof( ThreadId ) >= sizeof( uint32_t ) );
                            const auto tId{ *reinterpret_cast<const uint32_t*>( &ThreadId ) };
                            SKLL_ERR_FMT( "Dec-ThreadId: %u", tId );
                        }

                        SKL_ASSERT( 1 == decResult );

                    } ) };

                   SKL_ASSERT_ALLWAYS( SKL::RSuccess == Result );
                }

                obj.reset();
            }*/

            return true;
        }

        bool OnServerStarted() noexcept override
        {
            SKLL_TRACE();

            if ( false == TestApplication::OnServerStarted() )
            {
                SKL_ASSERT( false );
                return false;
            }
            
            /*if( false == InGroup.GetTag().bIsActive )
            {*/
                auto obj = SKL::MakeShared<MyObject>();

                for( uint64_t i = 0; i < IterCount; ++i )
                {
                    const auto Result{ obj->DoAsyncAfter( 500, [ this ]( SKL::AOD::SharedObject& InObj ) noexcept -> void 
                    {
                        auto& Self = reinterpret_cast<MyObject&>( InObj );

                        SKLL_TRACE_MSG_FMT( "Before Counter:%d", Self.Counter );

                        auto incResult{ a.increment() };

                        if( 0 != incResult )
                        {
                            const auto ThreadId { std::this_thread::get_id() };
                            static_assert( sizeof( ThreadId ) >= sizeof( uint32_t ) );
                            const auto tId{ *reinterpret_cast<const uint32_t*>( &ThreadId ) };
                            SKLL_ERR_FMT( "ThreadId: %u", tId );
                        }

                        SKL_ASSERT( 0 == incResult );

                        const auto NewCounter = --Self.Counter;
                        
                        if( NewCounter == 5002 )
                        {
                            SKL_BREAK();
                        }

                        SKL_ASSERT( true == Counts.empty() || NewCounter != Counts.top() );

                        Counts.push( NewCounter );

                        if( 0 == NewCounter )           
                        {
                            SignalToStop( true );
                        }

                        SKLL_TRACE_MSG_FMT( "After Counter:%d", Self.Counter );

                        auto decResult{ a.decrement() };
                        
                        if( 1 != decResult )
                        {
                            const auto ThreadId { std::this_thread::get_id() };
                            static_assert( sizeof( ThreadId ) >= sizeof( uint32_t ) );
                            const auto tId{ *reinterpret_cast<const uint32_t*>( &ThreadId ) };
                            SKLL_ERR_FMT( "Dec-ThreadId: %u", tId );
                        }

                        SKL_ASSERT( 1 == decResult );

                    } ) };

                   SKL_ASSERT_ALLWAYS( SKL::RSuccess == Result );
                }

                obj.reset();
            //}

            return true;
        }

        std::synced_value<int32_t> a;
        std::stack<int32_t, std::vector<int32_t>> Counts;
    };
    
    class AODTestsFixture_CustomObject : public ::testing::Test, public TestApplication
    {
    public:
        static constexpr uint64_t IterCount{ 1 };

        struct MyObject;

        struct MyObject : SKL::AOD::CustomObject
        {
            int Counter { IterCount };

            MyObject() noexcept : SKL::AOD::CustomObject{} 
            {
                SKLL_TRACE();
            }
            ~MyObject() noexcept
            {
                SKLL_TRACE();
                SKLL_INF_FMT( "AODTestsFixture4::MyObject::~MyObject() Counter:%d", Counter );
                SKL_ASSERT_ALLWAYS( 0 == Counter );
            }
        };
        
        AODTestsFixture_CustomObject()
            : ::testing::Test(),
              TestApplication( L"AOD_TESTS_APP" ) {}      

        void SetUp() override
        {
            ASSERT_TRUE( SKL::RSuccess == SKL::Skylake_InitializeLibrary( 0, nullptr, nullptr ) );
        }

        void TearDown() override
        {
            ASSERT_TRUE( SKL::RSuccess == SKL::Skylake_TerminateLibrary() );
        }

        bool OnServerStarted() noexcept override
        {
            if ( false == TestApplication::OnServerStarted() )
            {
                return false;
            }

            auto obj = SKL::MakeSharedVirtualDeleted<MyObject>( { &SKL::GlobalAllocatedDeleter<MyObject> } );

            for( uint64_t i = 0; i < IterCount; ++i )
            {
                const auto Result { obj->DoAsyncAfter( 5, [ this ]( SKL::AOD::CustomObject& InObj ) noexcept -> void 
                {
                    auto& Self = reinterpret_cast<MyObject&>( InObj );
                    if( 0 == --Self.Counter )           
                    {
                        SignalToStop( true );
                    }
                } ) };

                SKL_ASSERT_ALLWAYS( SKL::RSuccess == Result );
            }

            obj.reset();

            return true;
        }
    };

    TEST_F( AODStandaloneFixture, AODObjectSingleThread )
    {
        struct MyObject : SKL::AOD::SharedObject
        {
            MyObject() noexcept : SKL::AOD::SharedObject{ this } {}

            int a { 0 };
        };
        
        auto obj = SKL::MakeShared<MyObject>();

        ASSERT_TRUE( nullptr != obj.get() );
        ASSERT_TRUE( 0 == obj->a );
        const auto TotalAllocationsBefore{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsBefore{ SKL::GlobalMemoryManager::TotalDeallocations.load() };
        
        ( void )obj->DoAsync( []( SKL::AOD::SharedObject& Obj ) noexcept -> void
        {
            auto& Self = reinterpret_cast<MyObject&>( Obj );
        
            Self.a = 55;
        } );

        ASSERT_TRUE( 55 == obj->a );

        SKL::AODTLSContext::Destroy();

        const auto TotalAllocationsAfter{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsAfter{ SKL::GlobalMemoryManager::TotalDeallocations.load() };
        ASSERT_EQ( TotalAllocationsBefore + 1 , TotalAllocationsAfter );
        ASSERT_EQ( TotalDeallocationsBefore + 1, TotalDeallocationsAfter );
    }

    TEST_F( AODStandaloneFixture, AODObjectSingleThread_MultipleCalls )
    {
        struct MyObject : SKL::AOD::SharedObject
        {
            MyObject() noexcept : SKL::AOD::SharedObject{ this } {}

            int a { 0 };
        };
        
        auto obj = SKL::MakeShared<MyObject>();

        ASSERT_TRUE( nullptr != obj.get() );
        ASSERT_TRUE( 0 == obj->a );
        const auto TotalAllocationsBefore{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsBefore{ SKL::GlobalMemoryManager::TotalDeallocations.load() };
        
        for( int i = 0; i < 50; ++i )
        {
            ( void )obj->DoAsync( [i]( SKL::AOD::SharedObject& Obj ) noexcept -> void
            {
                auto& Self = Obj.GetParentObject<MyObject>();
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
        struct MyObject : SKL::AOD::SharedObject
        {
            MyObject() noexcept : SKL::AOD::SharedObject{ this } {}
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
            .bPreallocateAllThreadLocalPools = false,
            .bSupportesTCPAsyncAcceptors     = false,
            .bCallTickHandler                = true,
            .Name                            = L"AODOBJECTSINGLETHREAD_GROUP"
        }, [ Ptr = obj.get() ]( SKL::Worker& /*InWorker*/, SKL::WorkerGroup& InGroup ) mutable noexcept -> void
        {
            for( uint64_t i = 0; i < IterCount; ++i )
            {
                ( void )Ptr->DoAsync( []( SKL::AOD::SharedObject& InObj ) noexcept -> void
                {
                    auto& Self = InObj.GetParentObject<MyObject>();
                    ++Self.a;
                } );
            }

            std::this_thread::sleep_for( TCLOCK_MILLIS( 1000 ) );
            InGroup.GetServerInstance()->SignalToStop( true );
        } ) );

        ASSERT_TRUE( true == Start( true ) );

        ASSERT_TRUE( 16 * IterCount == obj->a );
        const auto TotalAllocationsAfter{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsAfter{ SKL::GlobalMemoryManager::TotalDeallocations.load() };
        //const auto CustomSizeAllocations{ SKL::GlobalMemoryManager::CustomSizeAllocations.load() };
        //const auto CustomSizeDeallocations{ SKL::GlobalMemoryManager::CustomSizeDeallocations.load() };
        ASSERT_TRUE( TotalAllocationsBefore + 16 * IterCount == TotalAllocationsAfter );
        ASSERT_TRUE( TotalDeallocationsBefore + 16 * IterCount == TotalDeallocationsAfter );
    }

    TEST_F( AODTestsFixture, AODObjectMultipleSymetricWorkers_OneDeferedTask )
    {
        struct MyObject : SKL::AOD::SharedObject
        {
            MyObject() noexcept : SKL::AOD::SharedObject{ this } {}
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

                ASSERT_TRUE( SKL::RSuccess == Ptr->DoAsyncAfter( 1000, []( SKL::AOD::SharedObject& InObject ) noexcept -> void 
                {
                    auto& Self = reinterpret_cast<MyObject&>( InObject );
                    SKLL_INF( "################# stop #################" );
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
        struct MyObject : SKL::AOD::SharedObject
        {
            MyObject() noexcept : SKL::AOD::SharedObject{ this } {}
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
                    ASSERT_TRUE( SKL::RSuccess == Ptr->DoAsyncAfter( 5, []( SKL::AOD::SharedObject& InObject ) noexcept -> void 
                    {
                        auto& Self = reinterpret_cast<MyObject&>( InObject );
                        if( 1 == Self.Counter.decrement() )
                        {
                            SKLL_INF( "############# LAST DECREMENT #############" );
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

                SKL::DeferTask( [ &InGroup ]( SKL::ITask* /*Self*/ ) noexcept -> void 
                {
                    SKLL_INF( "FROM TASK" );
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
                    SKL::DeferTask( [ &Counter ]( SKL::ITask* /*Self*/ ) noexcept -> void 
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

        auto OnTick = []( SKL::Worker& /*InWorker*/, SKL::WorkerGroup& InGroup ) mutable noexcept -> void
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
            .bPreallocateAllThreadLocalPools = false,
            .bSupportesTCPAsyncAcceptors     = false,
            .bCallTickHandler                = false,
            .Name                            = L"AODObjectMultipleWorkers_MultipleDeferedTasks_REACTIVE"
        }, []( SKL::Worker& /*InWorker*/, SKL::WorkerGroup& /*InGroup*/ ) noexcept -> void {} ) );
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
            .bPreallocateAllThreadLocalPools = false,
            .bSupportesTCPAsyncAcceptors     = false,
            .bCallTickHandler                = false,
            .Name                            = L"AODObjectMultipleWorkers_MultipleDeferedTasks_REACTIVE"
        }, []( SKL::Worker& /*InWorker*/, SKL::WorkerGroup& /*InGroup*/ ) noexcept -> void {} ) );

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
            .bPreallocateAllThreadLocalPools = false,
            .bSupportesTCPAsyncAcceptors     = false,
            .bCallTickHandler                = false,
            .Name                            = L"AODObjectMultipleWorkers_MultipleDeferedTasks_REACTIVE"
        }, []( SKL::Worker& /*InWorker*/, SKL::WorkerGroup& /*InGroup*/ ) noexcept -> void {} ) );
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
            .bPreallocateAllThreadLocalPools = false,
            .bSupportesTCPAsyncAcceptors     = false,
            .bCallTickHandler                = false,
            .Name                            = L"AODObjectMultipleWorkers_MultipleDeferedTasks_ACTIVE"
        }, []( SKL::Worker& /*InWorker*/, SKL::WorkerGroup& /*InGroup*/ ) noexcept -> void {} ) );
        ASSERT_TRUE( true == Start( true ) );

        JoinAllGroups();
        const auto TotalAllocationsAfter{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsAfter{ SKL::GlobalMemoryManager::TotalDeallocations.load() };
        ASSERT_TRUE( TotalAllocationsBefore + IterCount + 1 == TotalAllocationsAfter );
        ASSERT_TRUE( TotalDeallocationsBefore + IterCount + 1 == TotalDeallocationsAfter );
    }
    
    TEST_F( AODTestsFixture_CustomObject, AODObjectReactiveAndActiveWorkers_AODDeferredFromReactive_CustomObject )
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
            .bPreallocateAllThreadLocalPools = false,
            .bSupportesTCPAsyncAcceptors     = false,
            .bCallTickHandler                = false,
            .Name                            = L"AODObjectMultipleWorkers_MultipleDeferedTasks_REACTIVE"
        }, []( SKL::Worker& /*InWorker*/, SKL::WorkerGroup& /*InGroup*/ ) noexcept -> void {} ) );
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
            .bPreallocateAllThreadLocalPools = false,
            .bSupportesTCPAsyncAcceptors     = false,
            .bCallTickHandler                = false,
            .Name                            = L"AODObjectMultipleWorkers_MultipleDeferedTasks_ACTIVE"
        }, []( SKL::Worker& /*InWorker*/, SKL::WorkerGroup& /*InGroup*/ ) noexcept -> void {} ) );
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
    //TestLogFile = fopen( "./log.log", "w+" );
    if( nullptr != TestLogFile )
    {
        SKL::Skylake_InitializeLibrary( argc, argv, TestLogFile );
    }
    else
    {
        puts( "!!!Failed to open log file" );
    }

    testing::InitGoogleTest( &argc, argv );
    auto result{ RUN_ALL_TESTS( ) };

    if( nullptr != TestLogFile )
    {
       fprintf( TestLogFile, "########### TEST LOG END ###########\n" );
       fclose( TestLogFile );
    }

    return result;
}