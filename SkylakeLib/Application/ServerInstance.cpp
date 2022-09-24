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
            SKL_VER_FMT( "[ServerInstance:%ws] All worker groups are active-worker-groups!", GetName() );
        }

        ServerBuiltFlags.bSupportsDelayedTasks = bAtLeastOneWorkerGroupThatHandlesDelayedTasks;
        if( false == bAtLeastOneWorkerGroupThatHandlesDelayedTasks )
        {
            SKL_WRN_FMT( "[ServerInstance:%ws] No worker group to handle delayed tasks, DONT USE DELAYED TASKS!!", GetName() );
        }

        // create worker groups
        for( size_t i = 0; i < Config.WorkerGroups.size(); ++i )
        {
            const auto& WorkerConfig                { Config.WorkerGroups[i] };
            const bool  bDoesMasterNeedsToBeCreated { ( i == Config.WorkerGroups.size() - 1 ) && true == InConfig.bWillCaptureCallingThread };
 
            if( false == CreateWorkerGroup( WorkerConfig, bDoesMasterNeedsToBeCreated ) )
            {
                SKL_ERR_FMT( "ServerInstance[%ws]::Initialize()", Config.Name );
                return RFail;
            }
        }

        if( false == OnAddServices() )
        {
            SKL_ERR_FMT( "ServerInstance[%ws]::OnAddServices() Failed", Config.Name );
            return RFail;
        }

        // initialize all services
        for( auto* Service : AllServices )
        {
            if( const auto Result{ Service->Initialize() }; RSuccess != Result )
            {
                SKL_ERR_FMT( "ServerInstance[%ws]::Service UID:%d failed to Initialize() Result:%d", Config.Name, Service->GetUID(), static_cast<int32_t>( Result ) );
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

        for( auto* Group: WorkerGroups )
        {
            if( nullptr == Group ) { continue; }
            if( RSuccess != Group->Start() )
            {
                SKL_ERR_FMT("[WorkerGroup:%ws] Failed to start!", Group->GetTag().Name );
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
                SKL_ERR_FMT("[ServerInstance:%ws] OnAfterServerStopped() Failed!", Config.Name );
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
        if( false == bIsRunning.exchange( false ) )
        {
            SKL_VER_FMT( "[ServerInstance:%ws] SignalToStop() Already signaled!", Config.Name );
            return;
        }
        
        if( false == OnBeforeStopServer() )
        {
            if( false == bForce )
            {
                SKL_VER_FMT( "[ServerInstance:%ws] OnBeforeStopServer() Failed and cancelled the stop process!", Config.Name );
                return;
            }

            SKL_VER_FMT( "[ServerInstance:%ws] OnBeforeStopServer() Failed! The stop process continues [bForce=true]", Config.Name );
        }

        SKL_VER_FMT( "Stopping %u services...", TotalNumberOfInitServices.load_relaxed() );
        
        if( 0 != TotalNumberOfInitServices.load_acquire() )
        {
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

    bool ServerInstance::CreateWorkerGroup( const WorkerGroupConfig& Config, bool bCreateMaster ) noexcept
    {
        auto NewGroup { std::make_unique<WorkerGroup>( Config.Tag, this ) };

        // set worker tick handler
        NewGroup->SetWorkerTickHandler( Config.OnWorkerTick );
        
        // set worker start handler
        NewGroup->SetWorkerStartHandler( Config.OnWorkerStart );
        
        // set worker stop handler
        NewGroup->SetWorkerStopHandler( Config.OnWorkerStop );

        // add async tcp acceptors
        for( const auto& Config: Config.TCPAcceptorConfigs )
        {
            NewGroup->AddNewTCPAcceptor( Config );
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

        // save
        WorkerGroups.emplace_back( NewGroup.release() );
        ( void )TotalWorkerGroups.increment();

        return true;
    }

    bool ServerInstance::OnWorkerStarted( Worker& InWorker, WorkerGroup& InGroup ) noexcept
    {
        TRand::InitializeThread();

        if( true == InGroup.GetTag().bHasThreadLocalMemoryManager )
        {   
            if( nullptr == ThreadLocalMemoryManager::GetInstance() )
            {
                if ( RSuccess != ThreadLocalMemoryManager::Create() )
                {
                    SKL_ERR_FMT( "[Worker in WG:%ws] Failed to create ThreadLocalMemoryManager", InGroup.GetTag().Name );
                    return false;
                }   

                SKL_VER_FMT( "[Worker in WG:%ws] Created ThreadLocalMemoryManager.", InGroup.GetTag().Name );
            }
        
            if( true == InGroup.GetTag().bPreallocateAllThreadLocalPools )
            {
                SKL_VER_FMT( "[Worker in WG:%ws] Preallocated all pools in ThreadLocalMemoryManager.", InGroup.GetTag().Name );
                ThreadLocalMemoryManager::Preallocate();
            }
        }

        if( RSuccess != ServerInstanceTLSContext::Create( this, InGroup.GetTag() ) )
        {
            SKL_ERR_FMT("[WorkerGroup:%ws] failed to create ServerInstanceTLSContext for worker!", InGroup.GetTag().Name );
            return false;
        }

        if( true == InGroup.GetTag().bSupportsAOD )
        {
            if( RSuccess != AODTLSContext::Create( this, InGroup.GetTag() ) )
            {
                SKL_ERR_FMT("[WorkerGroup:%ws] failed to create AODTLSContext for worker!", InGroup.GetTag().Name );
                return false;
            }
        }

        for( auto& Service : WorkerServices )
        {
            if( nullptr == Service ){ continue; }
            Service->OnWorkerStarted( InWorker, InGroup );
        }

        TotalNumberOfRunningWorkers.increment();

        SKL_INF_FMT("[WorkerGroup:%ws] Worker started! Count:%u", InGroup.GetTag().Name, GetTotalNumberOfRunningWorkers() );
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
            AODTLSContext::Destroy();
            SKL_VER_FMT( "[Worker in WG:%ws] OnWorkerStopped() Destroyed AODTLSContext.", InGroup.GetTag().Name );
        }

        ServerInstanceTLSContext::Destroy();
        SKL_VER_FMT( "[Worker in WG:%ws] OnWorkerStopped() Destroyed ServerInstanceTLSContext.", InGroup.GetTag().Name );

        if( true == InGroup.GetTag().bHasThreadLocalMemoryManager )
        {
            ThreadLocalMemoryManager::FreeAllPools();
            ThreadLocalMemoryManager::Destroy();
            SKL_VER_FMT( "[Worker in WG:%ws] OnWorkerStopped() Destroyed ThreadLocalMemoryManager.", InGroup.GetTag().Name );
        }

        TRand::ShutdownThread();

        TotalNumberOfRunningWorkers.decrement();

        SKL_VER_FMT( "[WorkerGroup:%ws] worker stopped! Count:%u", InGroup.GetTag().Name, GetTotalNumberOfRunningWorkers() );
        return true;
    }

    bool ServerInstance::OnAllWorkersStarted( WorkerGroup& Group ) noexcept
    {
        SKL_VER_FMT( "[WorkerGroup:%ws] all workers started!", Group.GetTag().Name );
        SKL_ASSERT( Group.GetTotalNumberOfWorkers() == Group.GetNumberOfRunningWorkers() );
        return true;
    }
    
    bool ServerInstance::OnAllWorkersStopped( WorkerGroup& Group ) noexcept
    {
        SKL_VER_FMT( "[WorkerGroup:%ws] all workers stopped!", Group.GetTag().Name );
        SKL_ASSERT( 0 == Group.GetNumberOfRunningWorkers() );
        return true;
    }

    bool ServerInstance::OnWorkerGroupStarted( WorkerGroup& Group ) noexcept
    {
        SKL_VER_FMT("[WorkerGroup:%ws] started!", Group.GetTag().Name );

        const auto NewActiveWorkerGroupsCount { ActiveWorkerGroups.increment() + 1 };
        if( NewActiveWorkerGroupsCount == TotalWorkerGroups.load_relaxed() )
        {
            return OnAllWorkerGroupsStarted();
        }
        
        return true;
    }
    
    bool ServerInstance::OnWorkerGroupStopped( WorkerGroup& Group ) noexcept
    {
        SKL_VER_FMT("[WorkerGroup:%ws] stopped!", Group.GetTag().Name );

        const auto NewActiveWorkerGroupsCount { ActiveWorkerGroups.decrement() - 1 };
        if( NewActiveWorkerGroupsCount == 0 )
        {
            if( nullptr == MasterWorker )
            {
                if( false == OnAllWorkerGroupsStopped() )
                {
                    SKL_ERR_FMT("[ServerInstance:%ws] OnAllWorkerGroupsStopped() Failed!", Config.Name );
                }

                return OnAfterServerStopped();
            }

            return OnAllWorkerGroupsStopped();
        }

        return true;
    }
    
    bool ServerInstance::OnAllWorkerGroupsStarted() noexcept
    {
        if( false == OnServerStarted() )
        {
            SKL_ERR_FMT("[ServerInstance:%ws] OnServerStarted() Failed!", Config.Name );
            return false;
        }

        SKL_VER_FMT("[ServerInstance:%ws] All worker groups started!", Config.Name );
        return true;
    }
    
    bool ServerInstance::OnAllWorkerGroupsStopped() noexcept
    {
        if( false == OnServerStopped() )
        {
            SKL_ERR_FMT("[ServerInstance:%ws] OnServerStopped() Failed!", Config.Name );
        }

        SKL_VER_FMT("[ServerInstance:%ws] All worker groups stopped!", Config.Name );
        return true;
    }

    bool ServerInstance::OnBeforeStartServer() noexcept
    {
        SKL_VER_FMT( "[ServerInstance:%ws] Will start!", Config.Name );

        //The server is finally running
        bIsRunning.exchange( TRUE );

        return true;
    }
    
    bool ServerInstance::OnServerStarted() noexcept
    {
        SKL_VER_FMT( "[ServerInstance:%ws] Started!", Config.Name );

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
            SKL_VER_FMT( "[ServerInstance:%ws] Started ticking %llu active services registered.", Config.Name, ActiveServices.size() - 1 );

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
                    SKL_VER_FMT( "[ServerInstance:%ws] Stopped ticking active servers.", Config.Name );
                }
            } );
        }
        else
        {
            SKL_VER_FMT( "[ServerInstance:%ws] No active services registered.", Config.Name );
        }

        return true;
    }
    
    bool ServerInstance::OnBeforeStopServer() noexcept
    {
        SKL_VER_FMT( "[ServerInstance:%ws] Will stop!", Config.Name );
        return true;
    }
    
    bool ServerInstance::OnServerStopped() noexcept
    {
        SKL_VER_FMT( "[ServerInstance:%ws] Stopped!", Config.Name );

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
        SKL_VER_FMT( "[ServerInstance:%ws] Stopped final!", Config.Name );
        return true;
    }
    
    void ServerInstance::OnServiceStopped( IService* InService, RStatus InStatus ) noexcept
    {   
        SKL_VER_FMT( "Service %u %s! Status[%d]"
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
        SKL_VER( "All services stopped!" ); 

        SKL_VER( "Stopping all worker groups!" ); 
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
            SKL_ERR_FMT( "[ServerInstance: %ws]::AddService( SimpleService ) nullptr service!", GetName() );
            return false;
        }

        if( nullptr != GetServiceById( InService->GetUID() ) )
        {
            SKL_ERR_FMT( "[ServerInstance: %ws]::AddService( SimpleService ) A service with UID:%d was already added!", GetName(), InService->GetUID() );
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
            SKL_ERR_FMT( "[ServerInstance: %ws]::AddService( AODService ) nullptr service!", GetName() );
            return false;
        }

        if( nullptr != GetServiceById( InService->GetUID() ) )
        {
            SKL_ERR_FMT( "[ServerInstance: %ws]::AddService( AODService ) A service with UID:%d was already added!", GetName(), InService->GetUID() );
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
            SKL_ERR_FMT( "[ServerInstance: %ws]::AddService( ActiveService ) nullptr service!", GetName() );
            return false;
        }

        if( nullptr != GetServiceById( InService->GetUID() ) )
        {
            SKL_ERR_FMT( "[ServerInstance: %ws]::AddService( ActiveService ) A service with UID:%d was already added!", GetName(), InService->GetUID() );
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
            SKL_ERR_FMT( "[ServerInstance: %ws]::AddService( WorkerService ) nullptr service!", GetName() );
            return false;
        }

        if( nullptr != GetServiceById( InService->GetUID() ) )
        {
            SKL_ERR_FMT( "[ServerInstance: %ws]::AddService( WorkerService ) A service with UID:%d was already added!", GetName(), InService->GetUID() );
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