//!
//! \file ServerInstance.cpp
//! 
//! \brief Worker group manager abstraction
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#include "SkylakeLib.h"

namespace SKL
{
    RStatus ServerInstance::Initialize( ServerInstanceConfig&& InConfig ) noexcept
    {
        SKL_ASSERT_ALLWAYS( Skylake_IsTheLibraryInitialize() );

        if( false == InConfig.IsValid() )
        {
            return RInvalidParamters;
        }

        // save config
        Config = std::forward<ServerInstanceConfig>( InConfig );

        SimpleServices.reserve( 32 );
        SimpleServices.emplace_back( nullptr ); //index zero is not valid!

        AODServices.reserve( 32 );   
        AODServices.emplace_back( nullptr ); //index zero is not valid!

        ActiveServices.reserve( 32 );
        ActiveServices.emplace_back( nullptr ); //index zero is not valid!

        WorkerServices.reserve( 32 );
        WorkerServices.emplace_back( nullptr ); //index zero is not valid!

        WorkerGroups.reserve( 32 );
        WorkerGroups.emplace_back( nullptr ); //index zero is not valid!

        bool bAllGroupsAreActive{ true };
        bool bAtLeastOneWorkerGroupThatHandlesDelayedTasks{ true };

        // build flags
        for( size_t i = 0; i < Config.WorkerGroups.size(); ++i )
        {
            const auto& WorkerConfig                { Config.WorkerGroups[i] };
            bAllGroupsAreActive &= WorkerConfig.Tag.bIsActive;
            bAtLeastOneWorkerGroupThatHandlesDelayedTasks |= WorkerConfig.Tag.bHandlesTimerTasks;
        }

        ServerBuiltFlags.bAllGroupsAreActive = bAllGroupsAreActive;
        if( true == bAllGroupsAreActive )
        {
            SKLL_VER_FMT( "[ServerInstance:%ws] All worker groups are active-worker-groups!", GetName() );
        }

        ServerBuiltFlags.bSupportsDelayedTasks = bAtLeastOneWorkerGroupThatHandlesDelayedTasks;
        if( false == bAtLeastOneWorkerGroupThatHandlesDelayedTasks )
        {
            SKLL_WRN_FMT( "[ServerInstance:%ws] No worker group to handle delayed tasks, DONT USE DELAYED TASKS!!", GetName() );
        }

        uint64_t NoOfWorkersThatSupportTLSSync{ 0 };

        // create worker groups
        for( size_t i = 0; i < Config.WorkerGroups.size(); ++i )
        {
            const auto& WorkerConfig                { Config.WorkerGroups[i] };
            const bool  bDoesMasterNeedsToBeCreated { ( i == Config.WorkerGroups.size() - 1 ) && true == InConfig.bWillCaptureCallingThread };
 
            if( false == CreateWorkerGroup( WorkerConfig, bDoesMasterNeedsToBeCreated ) )
            {
                SKLL_ERR_FMT( "[ServerInstance:%ws]::Initialize()", Config.Name );
                return RFail;
            }

            if( WorkerConfig.Tag.bSupportsTLSSync )
            {
                NoOfWorkersThatSupportTLSSync += WorkerGroups.back()->GetTotalNumberOfWorkers();
            }
        }

        if( 0 < NoOfWorkersThatSupportTLSSync )
        {
            MyTLSSyncSystem = std::make_unique_cacheline<TLSSyncSystem>();
            if( nullptr == MyTLSSyncSystem.get() )
            {
                SKLL_ERR_FMT( "[ServerInstance:%ws]::Initialize() Failed to allocate TLSSyncSystem!", Config.Name );
                return RFail;
            }

            MyTLSSyncSystem->NoOfWorkersThatSupportTLSSync = NoOfWorkersThatSupportTLSSync;
        }

        SKLL_INF_FMT( "[ServerInstance:%ws] Created %llu Worker Groups. TLSSync workers count: %llu."
                     , Config.Name
                     , WorkerGroups.size()
                     , NoOfWorkersThatSupportTLSSync );

        if( false == OnAddServices() )
        {
            SKLL_ERR_FMT( "[ServerInstance:%ws]::OnAddServices() Failed", Config.Name );
            return RFail;
        }

        // initialize all services
        for( auto* Service : AllServices )
        {
            if( const auto Result{ Service->Initialize() }; RSuccess != Result )
            {
                SKLL_ERR_FMT( "[ServerInstance:%ws]::Service UID:%d failed to Initialize() Result:%d", Config.Name, Service->GetUID(), static_cast<int32_t>( Result ) );
                return RFail;
            }

            TotalNumberOfInitServices.increment();
        }

        return RSuccess;
    }

    RStatus ServerInstance::StartServer() noexcept
    {
        if( false == Config.IsValid() )
        {
            return RInvalidParamters;
        }
        
        if( false == OnBeforeStartServer() )
        {
            return RFail;
        }

        SyncWorkerStartup.reset( new std::latch( static_cast<ptrdiff_t>( TotalWorkers.load() ) ) );
        SyncWorkerShutdown.reset( new std::latch( static_cast<ptrdiff_t>( TotalWorkers.load() ) ) );

        for( auto* Group: WorkerGroups )
        {
            if( nullptr == Group ) { continue; }
            if( RSuccess != Group->Start() )
            {
                SKLL_ERR_FMT("[WorkerGroup:%ws] Failed to start!", Group->GetTag().Name );
                goto fail_case;
            }
        }

        if ( nullptr != MasterWorker )
        {
            // call run on the calling thread
            MasterWorker->RunImpl();
            JoinAllGroups();

            if( false == OnAfterServerStopped() )
            {
                SKLL_ERR_FMT("[ServerInstance:%ws] OnAfterServerStopped() Failed!", Config.Name );
            }

            return RServerInstanceFinalized;
        }

        return RSuccess;

    fail_case:
        for( auto* Group: WorkerGroups )
        {
            if( nullptr == Group ) { continue; }
            Group->Stop();

            delete Group;
        }
        WorkerGroups.clear();

        return RFail;
    }
    
    //! Signal all worker groups to stop
    void ServerInstance::SignalToStop( bool bForce ) noexcept
    {
        SKLL_TRACE();

        if( false == bIsRunning.exchange( false ) )
        {
            SKLL_VER_FMT( "[ServerInstance:%ws] SignalToStop() Already signaled!", Config.Name );
            return;
        }
        
        if( false == OnBeforeStopServer() )
        {
            if( false == bForce )
            {
                SKLL_VER_FMT( "[ServerInstance:%ws] OnBeforeStopServer() Failed and cancelled the stop process!", Config.Name );
                return;
            }

            SKLL_VER_FMT( "[ServerInstance:%ws] OnBeforeStopServer() Failed! The stop process continues [bForce=true]", Config.Name );
        }

        if( 0 != TotalNumberOfInitServices.load_acquire() )
        {
            SKLL_VER_FMT( "Stopping %u services...", TotalNumberOfInitServices.load_relaxed() );

            // notice all services
            for( auto* Service : AllServices )
            {
                Service->OnServerStopSignaled();
            }
        }
        else
        {
            OnAllServiceStopped();
        }
    }

    bool ServerInstance::CreateWorkerGroup( const WorkerGroupConfig& InConfig, bool bCreateMaster ) noexcept
    {
        auto NewGroup { std::make_unique<WorkerGroup>( InConfig.Tag, this ) };

        // set worker tick handler
        NewGroup->SetWorkerTickHandler( InConfig.OnWorkerTick );
        
        // set worker start handler
        NewGroup->SetWorkerStartHandler( InConfig.OnWorkerStart );
        
        // set worker stop handler
        NewGroup->SetWorkerStopHandler( InConfig.OnWorkerStop );

        // add async TCP acceptors
        for( const auto& Item: InConfig.TCPAcceptorConfigs )
        {
            NewGroup->AddNewTCPAcceptor( Item );
        }
        
        // build the group
        NewGroup->Build( bCreateMaster );

        // cache master worker if possible
        if( true == bCreateMaster )
        {
            MasterWorker = NewGroup->GetTheMasterWorker();
            SKL_ASSERT_ALLWAYS( nullptr != MasterWorker );
        }
        
        if( true == NewGroup->GetTag().bHandlesTimerTasks )
        {
            DeferredTasksHandlingGroups.push_back( NewGroup.get() );
        }
        
        if( true == NewGroup->GetTag().bSupportsAOD && true == NewGroup->GetTag().bIsActive )
        {
            DeferredAODTasksHandlingGroups.push_back( NewGroup.get() );
        }
        
        if( true == NewGroup->GetTag().bSupportsTLSSync )
        {
            TLSSyncHandlingGroup.push_back( NewGroup.get() );
        }

        TotalWorkers.increment( static_cast<uint32_t>( NewGroup->GetTotalNumberOfWorkers() ) );

        // save
        WorkerGroups.emplace_back( NewGroup.release() );
        ( void )TotalWorkerGroups.increment();

        return true;
    }

    bool ServerInstance::OnWorkerStarted( Worker& InWorker, WorkerGroup& InGroup ) noexcept
    {
        TRand::InitializeThread();

        if( nullptr == StringUtils::GetInstance() )
        {
            if( RSuccess != StringUtils::Create() )
            {
                SKLL_ERR_FMT( "[Worker in WG:%ws] Failed to create StringUtils", InGroup.GetTag().Name );
                return false;
            }
        }

        if( nullptr == ThreadLocalMemoryManager::GetInstance() )
        {
            if( RSuccess != ThreadLocalMemoryManager::Create() )
            {
                SKLL_ERR_FMT( "[Worker in WG:%ws] Failed to create ThreadLocalMemoryManager", InGroup.GetTag().Name );
                return false;
            }   

            SKLL_VER_FMT( "[Worker in WG:%ws] Created ThreadLocalMemoryManager.", InGroup.GetTag().Name );
        }
    
        if( true == InGroup.GetTag().bPreallocateAllThreadLocalPools )
        {
            SKLL_VER_FMT( "[Worker in WG:%ws] Preallocated all pools in ThreadLocalMemoryManager.", InGroup.GetTag().Name );
            ThreadLocalMemoryManager::Preallocate();
        }

        if( RSuccess != ServerInstanceTLSContext::Create( this, InGroup.GetTag() ) )
        {
            SKLL_ERR_FMT("[WorkerGroup:%ws] failed to create ServerInstanceTLSContext for worker!", InGroup.GetTag().Name );
            return false;
        }

        if( nullptr == AODTLSContext::GetInstance() )
        {
            if( RSuccess != AODTLSContext::Create( this, InGroup.GetTag() ) )
            {
                SKLL_ERR_FMT("[WorkerGroup:%ws] failed to create AODTLSContext for worker!", InGroup.GetTag().Name );
                return false;
            }
        }

        if( true == InGroup.GetTag().bSupportsAOD )
        {
        }

        for( auto& Service : WorkerServices )
        {
            if( nullptr == Service ){ continue; }
            Service->OnWorkerStarted( InWorker, InGroup );
        }

        TotalNumberOfRunningWorkers.increment();

        SKLL_INF_FMT("[WorkerGroup:%ws] Worker started! Count:%u", InGroup.GetTag().Name, GetTotalNumberOfRunningWorkers() );
        return true;
    }
    
    bool ServerInstance::OnWorkerStopped( Worker& InWorker, WorkerGroup& InGroup ) noexcept
    {
        for( auto& Service : WorkerServices )
        {
            if( nullptr == Service ){ continue; }
            Service->OnWorkerStopped( InWorker, InGroup );
        }

        if( true == InGroup.GetTag().bSupportsAOD )
        {
        }

        AODTLSContext::Destroy();
        SKLL_VER_FMT( "[Worker in WG:%ws] OnWorkerStopped() Destroyed AODTLSContext.", InGroup.GetTag().Name );

        ServerInstanceTLSContext::Destroy();
        SKLL_VER_FMT( "[Worker in WG:%ws] OnWorkerStopped() Destroyed ServerInstanceTLSContext.", InGroup.GetTag().Name );

        ThreadLocalMemoryManager::FreeAllPools();
        ThreadLocalMemoryManager::Destroy();
        SKLL_VER_FMT( "[Worker in WG:%ws] OnWorkerStopped() Destroyed ThreadLocalMemoryManager.", InGroup.GetTag().Name );

        TRand::ShutdownThread();

        StringUtils::Destroy();

        TotalNumberOfRunningWorkers.decrement();

        SKLL_VER_FMT( "[WorkerGroup:%ws] worker stopped! Count:%u", InGroup.GetTag().Name, GetTotalNumberOfRunningWorkers() );
        return true;
    }

    bool ServerInstance::OnAllWorkersStarted( WorkerGroup& Group ) noexcept
    {
        SKLL_VER_FMT( "[WorkerGroup:%ws] all workers started!", Group.GetTag().Name );
        SKL_ASSERT( Group.GetTotalNumberOfWorkers() == Group.GetNumberOfRunningWorkers() );
        return true;
    }
    
    bool ServerInstance::OnAllWorkersStopped( WorkerGroup& Group ) noexcept
    {
        SKLL_VER_FMT( "[WorkerGroup:%ws] all workers stopped!", Group.GetTag().Name );
        SKL_ASSERT( 0 == Group.GetNumberOfRunningWorkers() );
        return true;
    }

    bool ServerInstance::OnWorkerGroupStarted( WorkerGroup& Group ) noexcept
    {
        SKLL_VER_FMT("[WorkerGroup:%ws] started!", Group.GetTag().Name );

        const auto NewActiveWorkerGroupsCount { ActiveWorkerGroups.increment() + 1 };
        if( NewActiveWorkerGroupsCount == TotalWorkerGroups.load_relaxed() )
        {
            return OnAllWorkerGroupsStarted();
        }
        
        return true;
    }
    
    bool ServerInstance::OnWorkerGroupStopped( WorkerGroup& Group ) noexcept
    {
        SKLL_VER_FMT("[WorkerGroup:%ws] stopped!", Group.GetTag().Name );

        const auto NewActiveWorkerGroupsCount { ActiveWorkerGroups.decrement() - 1 };
        if( NewActiveWorkerGroupsCount == 0 )
        {
            if( nullptr == MasterWorker )
            {
                if( false == OnAllWorkerGroupsStopped() )
                {
                    SKLL_ERR_FMT("[ServerInstance:%ws] OnAllWorkerGroupsStopped() Failed!", Config.Name );
                }

                return OnAfterServerStopped();
            }

            return OnAllWorkerGroupsStopped();
        }

        return true;
    }
    
    bool ServerInstance::OnAllWorkerGroupsStarted() noexcept
    {
        SKLL_VER_FMT("[ServerInstance:%ws] All worker groups started!", Config.Name );

        if( false == OnServerStarted() )
        {
            SKLL_ERR_FMT("[ServerInstance:%ws] OnServerStarted() Failed!", Config.Name );
            return false;
        }

        return true;
    }
    
    bool ServerInstance::OnAllWorkerGroupsStopped() noexcept
    {
        if( false == OnServerStopped() )
        {
            SKLL_ERR_FMT("[ServerInstance:%ws] OnServerStopped() Failed!", Config.Name );
        }

        SKLL_VER_FMT("[ServerInstance:%ws] All worker groups stopped!", Config.Name );
        return true;
    }

    bool ServerInstance::OnBeforeStartServer() noexcept
    {
        SKLL_VER_FMT( "[ServerInstance:%ws] Will start!", Config.Name );

        //The server is finally running
        bIsRunning.exchange( TRUE );

        return true;
    }
    
    bool ServerInstance::OnServerStarted() noexcept
    {
        SKLL_VER_FMT( "[ServerInstance:%ws] Started!", Config.Name );

        SKL_ASSERT( 1 <= SimpleServices.size() && nullptr == SimpleServices[0] );
        SKL_ASSERT( 1 <= AODServices.size() && nullptr == AODServices[0] );
        SKL_ASSERT( 1 <= ActiveServices.size() && nullptr == ActiveServices[0] );
        SKL_ASSERT( 1 <= WorkerServices.size() && nullptr == WorkerServices[0] );
        
        // notice all services
        for( auto* Service : AllServices )
        {
            Service->OnServerStarted();
        }

        // Start from 1 as first entry is nullptr
        if( 1 < ActiveServices.size() )
        {
            SKLL_VER_FMT( "[ServerInstance:%ws] Started ticking %llu active services registered.", Config.Name, ActiveServices.size() - 1 );

            DeferTask([ this ]( ITask* Self ) noexcept -> void
            {
                // Tick all active services
                // Start from 1 as first entry is nullptr
                for( size_t i = 1; i < ActiveServices.size(); ++i )
                {
                    ActiveServices[ i ]->OnTick();
                }

                if( true == IsRunning() ) SKL_LIKELY
                {
                    // defer again the same task
                    DeferTaskAgain( Self );
                }
                else
                {
                    SKLL_VER_FMT( "[ServerInstance:%ws] Stopped ticking active servers.", Config.Name );
                }
            } );
        }
        else
        {
            SKLL_VER_FMT( "[ServerInstance:%ws] No active services registered.", Config.Name );
        }

        return true;
    }
    
    bool ServerInstance::OnBeforeStopServer() noexcept
    {
        SKLL_VER_FMT( "[ServerInstance:%ws] Will stop!", Config.Name );
        return true;
    }
    
    bool ServerInstance::OnServerStopped() noexcept
    {
        SKLL_VER_FMT( "[ServerInstance:%ws] Stopped!", Config.Name );

        //The server is finally running
        bIsRunning.exchange( TRUE );

        // notice all services
        for( auto* Service : AllServices )
        {
            Service->OnServerStopped();
        }

        return true;
    }

    bool ServerInstance::OnAfterServerStopped() noexcept
    {
        SKLL_VER_FMT( "[ServerInstance:%ws] Stopped final!", Config.Name );
        return true;
    }
    
    void ServerInstance::OnServiceStopped( IService* InService, RStatus InStatus ) noexcept
    {   
        SKLL_VER_FMT( "Service %u %s! Status[%d]"
            , InService->GetUID()
            , RSuccess == InStatus ? "stopped successfully" : "failed to stop"
            , RSTATUS_TO_NUMERIC( InStatus ) );

        if( RSuccess != InStatus )
        {
            // do smth?
        }

        if( 1 == TotalNumberOfInitServices.decrement() )
        {
            // all services stopped, continue with the shutdown
            OnAllServiceStopped();
        }
    } 
    
    void ServerInstance::OnAllServiceStopped() noexcept
    {   
        SKLL_VER( "All services stopped!" ); 

        SKLL_VER( "Stopping all worker groups!" ); 
        for( auto* Group: WorkerGroups )
        {
            if( nullptr == Group ) { continue; }
            Group->SignalToStop();
        }
    } 

    bool ServerInstance::AddService( SimpleService* InService ) noexcept
    {
        SKL_ASSERT( false == IsRunning() );

        if( nullptr == InService )
        {
            SKLL_ERR_FMT( "[ServerInstance: %ws]::AddService( SimpleService ) nullptr service!", GetName() );
            return false;
        }

        if( 0 != ( reinterpret_cast<uint64_t>( InService ) % SKL_CACHE_LINE_SIZE ) )
        {
            SKLL_ERR_FMT( "[ServerInstance: %ws]::AddService( SimpleService ) Use CreateService<T>(...) to create the service!", GetName() );
            return false;
        }

        if( nullptr != GetServiceById( InService->GetUID() ) )
        {
            SKLL_ERR_FMT( "[ServerInstance: %ws]::AddService( SimpleService ) A service with UID:%d was already added!", GetName(), InService->GetUID() );
            return false;
        }

        SimpleServices.emplace_back( InService );
        SKL_ASSERT( static_cast<uint32_t>( SimpleServices.size() - 1 ) == InService->GetUID() );

        // Set the server instance
        InService->MyServerInstance = this;

        AllServices.push_back( InService );
        
        return true;
    }

    bool ServerInstance::AddService( AODService* InService ) noexcept
    {
        SKL_ASSERT( false == IsRunning() );

        if( nullptr == InService )
        {
            SKLL_ERR_FMT( "[ServerInstance: %ws]::AddService( AODService ) nullptr service!", GetName() );
            return false;
        }

        if( nullptr != GetServiceById( InService->GetUID() ) )
        {
            SKLL_ERR_FMT( "[ServerInstance: %ws]::AddService( AODService ) A service with UID:%d was already added!", GetName(), InService->GetUID() );
            return false;
        }

        AODServices.emplace_back( InService );
        SKL_ASSERT( static_cast<uint32_t>( AODServices.size() - 1 ) == InService->GetUID() );

        // Set the server instance
        InService->MyServerInstance = this;

        AllServices.push_back( InService );
        
        return true;
    }

    bool ServerInstance::AddService( ActiveService* InService ) noexcept
    {
        SKL_ASSERT( false == IsRunning() );

        if( nullptr == InService )
        {
            SKLL_ERR_FMT( "[ServerInstance: %ws]::AddService( ActiveService ) nullptr service!", GetName() );
            return false;
        }

        if( nullptr != GetServiceById( InService->GetUID() ) )
        {
            SKLL_ERR_FMT( "[ServerInstance: %ws]::AddService( ActiveService ) A service with UID:%d was already added!", GetName(), InService->GetUID() );
            return false;
        }

        ActiveServices.emplace_back( InService );
        SKL_ASSERT( static_cast<uint32_t>( ActiveServices.size() - 1 ) == InService->GetUID() );

        // Set the server instance
        InService->MyServerInstance = this;

        AllServices.push_back( InService );
        
        return true;
    }

    bool ServerInstance::AddService( WorkerService* InService ) noexcept
    {
        SKL_ASSERT( false == IsRunning() );

        if( nullptr == InService )
        {
            SKLL_ERR_FMT( "[ServerInstance: %ws]::AddService( WorkerService ) nullptr service!", GetName() );
            return false;
        }

        if( nullptr != GetServiceById( InService->GetUID() ) )
        {
            SKLL_ERR_FMT( "[ServerInstance: %ws]::AddService( WorkerService ) A service with UID:%d was already added!", GetName(), InService->GetUID() );
            return false;
        }

        WorkerServices.emplace_back( InService );
        SKL_ASSERT( static_cast<uint32_t>( WorkerServices.size() - 1 ) == InService->GetUID() );

        // Set the server instance
        InService->MyServerInstance = this;

        AllServices.push_back( InService );
        
        return true;
    }
}
