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
    
            SKL::RStatus OnStopService() noexcept override
            {
                SKL_ASSERT_ALLWAYS( 3 == ++SeqCounter );
                return SKL::RSuccess;
            }

            int32_t SeqCounter{ 0 };
        };

        SimpleService_TestsFixture()
            : ::testing::Test(),
              TestApplication( L"AOD_TESTS_APP" ) {}      
  
        bool OnAddServices() noexcept override
        {
            SKL_ASSERT_ALLWAYS( true == AddService( SKL::CreateService<MySimpleService>() ) );
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
    
            SKL::RStatus OnStopService() noexcept override
            {
                SKL_ASSERT_ALLWAYS( 3 == ++SeqCounter );
                return SKL::RSuccess;
            }

            int32_t SeqCounter{ 0 };
        };

        AODService_TestsFixture()
            : ::testing::Test(),
              TestApplication( L"AOD_TESTS_APP" ) {}      
  
        bool OnAddServices() noexcept override
        {
            SKL_ASSERT_ALLWAYS( true == AddService( SKL::CreateService<MyService>() ) );
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
    
            SKL::RStatus OnStopService() noexcept override
            {
                //SKL_ASSERT_ALLWAYS( 3 == ++SeqCounter );
                return SKL::RSuccess;
            }

            void OnTick() noexcept override
            {
                if( ++SeqCounter == 120 ) // 2 seconds = 60 * 2
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
            SKL_ASSERT_ALLWAYS( true == AddService( SKL::CreateService<MyService>() ) );
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
    
            SKL::RStatus OnStopService() noexcept override
            {
                SKL_ASSERT_ALLWAYS( 3 == ++SeqCounter );
                SKL_ASSERT_ALLWAYS( CWorkersCount == DoneCount.load_acquire() );

                return SKL::RSuccess;
            }

            //! [Callback] Each time a worker started
            SKL::RStatus OnWorkerStarted( SKL::Worker& /*InWorker*/, SKL::WorkerGroup& /*InWorkerGroup*/ ) noexcept override
            {
                TLSCounter::SetValue( 0 );

                return SKL::RSuccess;
            }

            //! [Callback] Each time a worker stopped
            void OnWorkerStopped( SKL::Worker& /*InWorker*/, SKL::WorkerGroup& /*InWorkerGroup*/ ) noexcept override
            {
                const auto Value{ TLSCounter::GetValue() };
                SKL_ASSERT_ALLWAYS( CIterCount <= Value );
                TLSCounter::SetValue( 0 );
            }

            //! [Callback] Tick for each active worker
            void OnTickWorker( SKL::Worker& /*InWorker*/, SKL::WorkerGroup& /*InWorkerGroup*/ ) noexcept override
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
            SKL_ASSERT_ALLWAYS( true == AddService( SKL::CreateService<MyService>() ) );
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
    
    class SimpleService_AsyncShutdown_TestsFixture : public ::testing::Test, public TestApplication
    {
    public:
        class MySimpleService: public SKL::SimpleService
        {
        public:
            MySimpleService() noexcept: SKL::SimpleService{ 1 }{ }

        protected:
            SKL::RStatus Initialize() noexcept override
            {
                GTRACE();

                SKL_ASSERT_ALLWAYS( 1 == ++SeqCounter );
                return SKL::RSuccess;
            }

            void OnServerStarted() noexcept override
            {
                GTRACE();

                SKL_ASSERT_ALLWAYS( 2 == ++SeqCounter );

                SKL::DeferTask([ this ]( SKL::ITask* ) noexcept -> void 
                {
                    GTRACE_DEBUG( "STOP!" );
                    GetServerInstance().SignalToStop();
                });
            }
    
            void OnServerStopped() noexcept override
            {
                GTRACE();

                SKL_ASSERT_ALLWAYS( 4 == ++SeqCounter );
            }
    
            SKL::RStatus OnStopService() noexcept override
            {
                GTRACE();

                SKL_ASSERT_ALLWAYS( 3 == ++SeqCounter );
                
                SKL::DeferTask( [ this ]( SKL::ITask* ) noexcept -> void 
                {
                    GTRACE();

                    // finally signal that the service was stopped
                    OnServiceStopped( SKL::RSuccess  );
                } );

                // signal that we need to perform async operation to stop
                return SKL::RPending;
            }

            int32_t SeqCounter{ 0 };
        };

        SimpleService_AsyncShutdown_TestsFixture()
            : ::testing::Test(),
              TestApplication( L"AOD_TESTS_APP" ) {}      
  
        bool OnAddServices() noexcept override
        {
            SKL_ASSERT_ALLWAYS( true == AddService( SKL::CreateService<MySimpleService>() ) );
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
    
    class SimpleService_AsyncShutdown_EntityStore_TestsFixture : public ::testing::Test, public TestApplication
    {
    public:
        class MySimpleService: public SKL::SimpleService
        {
        public:
            using MyEntityId = SKL::TEntityId<uint32_t>;
            static constexpr SKL::TEntityType MyEntityType = 1;
            static constexpr SKL::EntityStoreFlags MyEntityStoreFlags {};

            struct RootComponentData
            {
                int32_t a { 5 };
                int32_t b { 5 };

                void OnDestroy() noexcept {}
            };
            struct OtherComponent
            {
                int32_t b { 55 };
            };

            using MyEntityStore = SKL::EntityStore<MyEntityType, MyEntityId, 1024, MyEntityStoreFlags, RootComponentData, OtherComponent>;
            using TEntityPtr = MyEntityStore::TEntitySharedPtr;

            MySimpleService() noexcept: SKL::SimpleService{ 1 }{ }

        protected:
            SKL::RStatus Initialize() noexcept override
            {
                if( SKL::RSuccess != Store.Initialize() )
                {
                    return SKL::RFail;
                }

                Store.Activate();

                Store.SetOnAllFreed( [this]() noexcept -> void 
                {
                    puts( "All entities freed!" );
                    OnServiceStopped( SKL::RSuccess  );
                } );

                AllocatedPtr = Store.AllocateEntity( 151 );
                SKL_ASSERT( nullptr != AllocatedPtr.get() );

                return SKL::RSuccess;
            }

            void OnServerStarted() noexcept override
            {
                SKL::DeferTask([ this ]( SKL::ITask* ) noexcept -> void 
                {
                    GetServerInstance().SignalToStop( true );
                });
            }
    
            void OnServerStopped() noexcept override
            {
            }
    
            SKL::RStatus OnStopService() noexcept override
            {
                Store.Deactivate();

                SKL::DeferTask([ this ]( SKL::ITask* ) noexcept -> void 
                {
                    AllocatedPtr.reset();
                });

                return SKL::RPending;
            }

            TEntityPtr    AllocatedPtr{ nullptr };
            MyEntityStore Store       {};
        };

        SimpleService_AsyncShutdown_EntityStore_TestsFixture()
            : ::testing::Test(),
              TestApplication( L"AOD_TESTS_APP" ) {}      
  
        bool OnAddServices() noexcept override
        {
            SKL_ASSERT_ALLWAYS( true == AddService( SKL::CreateService<MySimpleService>() ) );
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
    
    class SimpleService_AsyncShutdown_EntityStore_AOD_TestsFixture : public ::testing::Test, public TestApplication
    {
    public:
        class MySimpleService: public SKL::SimpleService
        {
        public:
            using MyEntityId = SKL::TEntityId<uint32_t>;
            static constexpr SKL::TEntityType MyEntityType = 1;
            static constexpr SKL::EntityStoreFlags MyEntityStoreFlags {};

            struct RootComponentData
            {
                int32_t a { 5 };
                int32_t b { 5 };

                void OnDestroy() noexcept {}
            };
            struct OtherComponent
            {
                int32_t b { 55 };
            };

            using MyEntityStore = SKL::EntityStore<MyEntityType, MyEntityId, 1024, MyEntityStoreFlags, RootComponentData, OtherComponent>;
            using TEntityPtr = MyEntityStore::TEntitySharedPtr;

            MySimpleService() noexcept: SKL::SimpleService{ 1 }{ }

        protected:
            SKL::RStatus Initialize() noexcept override
            {
                if( SKL::RSuccess != Store.Initialize() )
                {
                    return SKL::RFail;
                }

                Store.Activate();

                Store.SetOnAllFreed( [this]() noexcept -> void 
                {
                    puts( "All entities freed!" );
                    OnServiceStopped( SKL::RSuccess  );
                } );

                AllocatedPtr = Store.AllocateEntity( 151 );
                SKL_ASSERT( nullptr != AllocatedPtr.get() );

                return SKL::RSuccess;
            }

            void OnServerStarted() noexcept override
            {
                ( void )AllocatedPtr->DoAsyncAfter( 300, [ this ]( SKL::AOD::CustomObject& /*SelfObj*/ ) noexcept -> void
                {
                    GTRACE_DEBUG( "DO ASYNC" );

                    SKL::DeferTask( [ this ]( SKL::ITask* ) noexcept -> void 
                    {
                        GTRACE_DEBUG( "STOP" );
                        GetServerInstance().SignalToStop( true );
                    } );
                } );
            }
    
            void OnServerStopped() noexcept override
            {
            }
    
            SKL::RStatus OnStopService() noexcept override
            {
                Store.Deactivate();

                SKL::DeferTask([ this ]( SKL::ITask* ) noexcept -> void 
                {
                    AllocatedPtr.reset();
                });

                return SKL::RPending;
            }

            TEntityPtr    AllocatedPtr{ nullptr };
            MyEntityStore Store       {};
        };

        SimpleService_AsyncShutdown_EntityStore_AOD_TestsFixture()
            : ::testing::Test(),
              TestApplication( L"AOD_TESTS_APP" ) {}      
  
        bool OnAddServices() noexcept override
        {
            SKL_ASSERT_ALLWAYS( true == AddService( SKL::CreateService<MySimpleService>() ) );
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
#if defined(SKL_MEMORY_STATISTICS)
        const auto TotalAllocationsBefore{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsBefore{ SKL::GlobalMemoryManager::TotalDeallocations.load() };
#endif
        SKL::WorkerGroupTag Tag{
            .TickRate        = 30, 
            .SyncTLSTickRate = 0,
            .Id              = 1,
            .WorkersCount    = 2,
            .Name            = L"SimpleService_BasicAPI_ACTIVE"
        };
        Tag.bIsActive = true;
        Tag.bSupportsAOD = true;
        Tag.bHandlesTimerTasks = true;
        ASSERT_TRUE( true == AddNewWorkerGroup( Tag, []( SKL::Worker& , SKL::WorkerGroup& ) noexcept -> void {} ) );

        ASSERT_TRUE( true == Start( true ) );
        JoinAllGroups();

#if defined(SKL_MEMORY_STATISTICS)
        const auto TotalAllocationsAfter{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsAfter{ SKL::GlobalMemoryManager::TotalDeallocations.load() };
        ASSERT_TRUE( TotalAllocationsBefore + 1 == TotalAllocationsAfter );
        ASSERT_TRUE( TotalDeallocationsBefore + 1 == TotalDeallocationsAfter );
#endif
    }

    TEST_F( AODService_TestsFixture, AODService_BasicAPI )
    {
#if defined(SKL_MEMORY_STATISTICS)
        const auto TotalAllocationsBefore{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsBefore{ SKL::GlobalMemoryManager::TotalDeallocations.load() };
#endif
        
        SKL::WorkerGroupTag Tag{
            .TickRate        = 30, 
            .SyncTLSTickRate = 0,
            .Id              = 1,
            .WorkersCount    = 2,
            .Name            = L"AODService_BasicAPI_ACTIVE"
        };
        Tag.bIsActive = true;
        Tag.bSupportsAOD = true;
        Tag.bHandlesTimerTasks = true;
        ASSERT_TRUE( true == AddNewWorkerGroup( Tag, []( SKL::Worker& , SKL::WorkerGroup& ) noexcept -> void {} ) );

        ASSERT_TRUE( true == Start( true ) );
        JoinAllGroups();

#if defined(SKL_MEMORY_STATISTICS)
        const auto TotalAllocationsAfter{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsAfter{ SKL::GlobalMemoryManager::TotalDeallocations.load() };
        ASSERT_TRUE( TotalAllocationsBefore + 2 == TotalAllocationsAfter );
        ASSERT_TRUE( TotalDeallocationsBefore + 2 == TotalDeallocationsAfter );
#endif
    }

    TEST_F( ActiveService_TestsFixture, ActiveService_BasicAPI )
    {
#if defined(SKL_MEMORY_STATISTICS)
        const auto TotalAllocationsBefore{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsBefore{ SKL::GlobalMemoryManager::TotalDeallocations.load() };
#endif
        
        SKL::WorkerGroupTag Tag{
            .TickRate        = 60, 
            .SyncTLSTickRate = 0,
            .Id              = 1,
            .WorkersCount    = 2,
            .Name            = L"ActiveService_BasicAPI_ACTIVE"
        };
        Tag.bIsActive = true;
        Tag.bSupportsAOD = true;
        Tag.bHandlesTimerTasks = true;
        ASSERT_TRUE( true == AddNewWorkerGroup( Tag, []( SKL::Worker& , SKL::WorkerGroup& ) noexcept -> void {} ) );

        ASSERT_TRUE( true == Start( true ) );
        JoinAllGroups();

#if defined(SKL_MEMORY_STATISTICS)
        const auto TotalAllocationsAfter{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsAfter{ SKL::GlobalMemoryManager::TotalDeallocations.load() };
        ASSERT_TRUE( TotalAllocationsBefore + 1 == TotalAllocationsAfter );
        ASSERT_TRUE( TotalDeallocationsBefore + 1 == TotalDeallocationsAfter );
#endif
    }

    TEST_F( WorkerService_TestsFixture, WorkerService_BasicAPI )
    {
#if defined(SKL_MEMORY_STATISTICS)
        const auto TotalAllocationsBefore{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsBefore{ SKL::GlobalMemoryManager::TotalDeallocations.load() };
#endif
        
        SKL::WorkerGroupTag Tag{
            .TickRate        = CTickRate, 
            .SyncTLSTickRate = 0,
            .Id              = 1,
            .WorkersCount    = CWorkersCount,
            .Name            = L"ActiveService_BasicAPI_ACTIVE"
        };
        Tag.bIsActive = true;
        Tag.bSupportsAOD = true;
        Tag.bHandlesTimerTasks = true;
        Tag.bTickWorkerServices = true;
        ASSERT_TRUE( true == AddNewWorkerGroup( Tag, []( SKL::Worker& , SKL::WorkerGroup& ) noexcept -> void {} ) );

        ASSERT_TRUE( true == Start( true ) );
        JoinAllGroups();

#if defined(SKL_MEMORY_STATISTICS)
        const auto TotalAllocationsAfter{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsAfter{ SKL::GlobalMemoryManager::TotalDeallocations.load() };
        ASSERT_TRUE( TotalAllocationsBefore == TotalAllocationsAfter );
        ASSERT_TRUE( TotalDeallocationsBefore == TotalDeallocationsAfter );
#endif
    }

    TEST_F( SimpleService_AsyncShutdown_TestsFixture, SimpleService_AsyncShutdown )
    {
#if defined(SKL_MEMORY_STATISTICS)
        const auto TotalAllocationsBefore{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsBefore{ SKL::GlobalMemoryManager::TotalDeallocations.load() };
#endif
        
        SKL::WorkerGroupTag Tag{
            .TickRate        = 30, 
            .SyncTLSTickRate = 0,
            .Id              = 1,
            .WorkersCount    = 2,
            .Name            = L"SimpleService_AsyncShutdown_ACTIVE"
        };
        Tag.bIsActive = true;
        Tag.bSupportsAOD = true;
        Tag.bHandlesTimerTasks = true;
        ASSERT_TRUE( true == AddNewWorkerGroup( Tag, []( SKL::Worker& , SKL::WorkerGroup& ) noexcept -> void {} ) );

        ASSERT_TRUE( true == Start( true ) );
        JoinAllGroups();

#if defined(SKL_MEMORY_STATISTICS)
        const auto TotalAllocationsAfter{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsAfter{ SKL::GlobalMemoryManager::TotalDeallocations.load() };
        ASSERT_TRUE( TotalAllocationsBefore + 2 == TotalAllocationsAfter );
        ASSERT_TRUE( TotalDeallocationsBefore + 2 == TotalDeallocationsAfter );
#endif
    }
    
    TEST_F( SimpleService_AsyncShutdown_EntityStore_TestsFixture, SimpleService_EntityStore_AsyncShutdown )
    {
#if defined(SKL_MEMORY_STATISTICS)
        const auto TotalAllocationsBefore{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsBefore{ SKL::GlobalMemoryManager::TotalDeallocations.load() };
#endif
        
        SKL::WorkerGroupTag Tag{
            .TickRate        = 30, 
            .SyncTLSTickRate = 0,
            .Id              = 1,
            .WorkersCount    = 2,
            .Name            = L"SimpleService_EntityStore_AsyncShutdown_ACTIVE"
        };
        Tag.bIsActive = true;
        Tag.bSupportsAOD = true;
        Tag.bHandlesTimerTasks = true;
        ASSERT_TRUE( true == AddNewWorkerGroup( Tag, []( SKL::Worker& , SKL::WorkerGroup& ) noexcept -> void {} ) );

        ASSERT_TRUE( true == Start( true ) );
        JoinAllGroups();

#if defined(SKL_MEMORY_STATISTICS)
        const auto TotalAllocationsAfter{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsAfter{ SKL::GlobalMemoryManager::TotalDeallocations.load() };
        ASSERT_TRUE( TotalAllocationsBefore + 2 == TotalAllocationsAfter );
        ASSERT_TRUE( TotalDeallocationsBefore + 2 == TotalDeallocationsAfter );
#endif
    }

    TEST_F( SimpleService_AsyncShutdown_EntityStore_AOD_TestsFixture, SimpleService_EntityStore_AOD_AsyncShutdown )
    {
#if defined(SKL_MEMORY_STATISTICS)
        const auto TotalAllocationsBefore{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsBefore{ SKL::GlobalMemoryManager::TotalDeallocations.load() };
#endif
        
        SKL::WorkerGroupTag Tag{
            .TickRate        = 30, 
            .SyncTLSTickRate = 0,
            .Id              = 1,
            .WorkersCount    = 2,
            .Name            = L"SimpleService_EntityStore_AOD_AsyncShutdown_ACTIVE"
        };
        Tag.bIsActive = true;
        Tag.bSupportsAOD = true;
        Tag.bHandlesTimerTasks = true;
        ASSERT_TRUE( true == AddNewWorkerGroup( Tag, []( SKL::Worker& , SKL::WorkerGroup& ) noexcept -> void {} ) );

        ASSERT_TRUE( true == Start( true ) );
        JoinAllGroups();

#if defined(SKL_MEMORY_STATISTICS)
        const auto TotalAllocationsAfter{ SKL::GlobalMemoryManager::TotalAllocations.load() };
        const auto TotalDeallocationsAfter{ SKL::GlobalMemoryManager::TotalDeallocations.load() };
        ASSERT_TRUE( TotalAllocationsBefore + 3 == TotalAllocationsAfter );
        ASSERT_TRUE( TotalDeallocationsBefore + 3 == TotalDeallocationsAfter );
#endif
    }
}

int main( int argc, char** argv )
{
    testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS( );
}
