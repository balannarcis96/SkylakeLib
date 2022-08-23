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

        for( auto& Group: WorkerGroups )
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
        for( auto& Group: WorkerGroups )
        {
            if( nullptr == Group ) { continue; }
            Group->Stop();
        }

        return RFail;
    }
    
    bool ServerInstance::CreateWorkerGroup( const WorkerGroupConfig& Config, bool bCreateMaster ) noexcept
    {
        auto NewGroup { std::make_shared<WorkerGroup>( Config.Tag, this ) };

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
            SKL_ASSERT_ALLWAYS( nullptr != MasterWorker.get() );
        }
        
        if( true == NewGroup->GetTag().bHandlesTimerTasks )
        {
            DeferredTasksHandlingGroups.push_back( NewGroup );
        }
        
        if( true == NewGroup->GetTag().bSupportsAOD && true == NewGroup->GetTag().bIsActive )
        {
            DeferredAODTasksHandlingGroups.push_back( NewGroup );
        }

        // save
        WorkerGroups.emplace_back( std::move( NewGroup ) );

        return true;
    }

    bool ServerInstance::OnWorkerStarted( Worker& InWorker, WorkerGroup& Group ) noexcept
    {
        if( true == Group.GetTag().bHasThreadLocalMemoryManager )
        {   
            if( nullptr == ThreadLocalMemoryManager::GetInstance() )
            {
                if ( RSuccess != ThreadLocalMemoryManager::Create() )
                {
                    SKL_ERR_FMT( "[Worker in WG:%ws] Failed to create ThreadLocalMemoryManager", Group.GetTag().Name );
                    return false;
                }   
            }
    
            SKL_VER_FMT( "[Worker in WG:%ws] Created ThreadLocalMemoryManager.", Group.GetTag().Name );
        
            if( true == Group.GetTag().bPreallocateAllThreadLocalPools )
            {
                SKL_VER_FMT( "[Worker in WG:%ws] Preallocated all pools in ThreadLocalMemoryManager.", Group.GetTag().Name );
                ThreadLocalMemoryManager::Preallocate();
            }
        }

        if( RSuccess != ServerInstanceTLSContext::Create( this, Group.GetTag() ) )
        {
            SKL_ERR_FMT("[WorkerGroup:%ws] failed to create ServerInstanceTLSContext for worker!", Group.GetTag().Name );
            return false;
        }

        if( true == Group.GetTag().bSupportsAOD )
        {
            if( RSuccess != AODTLSContext::Create( this, Group.GetTag() ) )
            {
                SKL_ERR_FMT("[WorkerGroup:%ws] failed to create AODTLSContext for worker!", Group.GetTag().Name );
                return false;
            }
        }

        SKL_INF_FMT("[WorkerGroup:%ws] worker started!", Group.GetTag().Name );
        return true;
    }
    
    bool ServerInstance::OnWorkerStopped( Worker& InWorker, WorkerGroup& Group ) noexcept
    {
        if( true == Group.GetTag().bSupportsAOD )
        {
            AODTLSContext::Destroy();
            SKL_VER_FMT( "[Worker in WG:%ws] OnWorkerStopped() Destroyed AODTLSContext.", Group.GetTag().Name );
        }

        ServerInstanceTLSContext::Destroy();
        SKL_VER_FMT( "[Worker in WG:%ws] OnWorkerStopped() Destroyed ServerInstanceTLSContext.", Group.GetTag().Name );

        if( true == Group.GetTag().bHasThreadLocalMemoryManager )
        {
            ThreadLocalMemoryManager::FreeAllPools();
            ThreadLocalMemoryManager::Destroy();
            SKL_VER_FMT( "[Worker in WG:%ws] OnWorkerStopped() Destroyed ThreadLocalMemoryManager.", Group.GetTag().Name );
        }

        SKL_INF_FMT("[WorkerGroup:%ws] worker stopped!", Group.GetTag().Name );
        return true;
    }

    bool ServerInstance::OnAllWorkersStarted( WorkerGroup& Group ) noexcept
    {
        SKL_INF_FMT("[WorkerGroup:%ws] all workers started!", Group.GetTag().Name );
        return true;
    }
    
    bool ServerInstance::OnAllWorkersStopped( WorkerGroup& Group ) noexcept
    {
        SKL_INF_FMT("[WorkerGroup:%ws] all workers stopped!", Group.GetTag().Name );
        return true;
    }

    bool ServerInstance::OnWorkerGroupStarted( WorkerGroup& Group ) noexcept
    {
        SKL_INF_FMT("[WorkerGroup:%ws] started!", Group.GetTag().Name );

        const auto NewActiveWorkerGroupsCount { ActiveWorkerGroups.increment() };
        if( NewActiveWorkerGroupsCount == TotalWorkerGroups.load_relaxed() )
        {
            return OnAllWorkerGroupsStarted();
        }
        
        return true;
    }
    
    bool ServerInstance::OnWorkerGroupStopped( WorkerGroup& Group ) noexcept
    {
        SKL_INF_FMT("[WorkerGroup:%ws] stopped!", Group.GetTag().Name );

        const auto NewActiveWorkerGroupsCount { ActiveWorkerGroups.decrement() };
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

        SKL_INF_FMT("[ServerInstance:%ws] All worker groups started!", Config.Name );
        return true;
    }
    
    bool ServerInstance::OnAllWorkerGroupsStopped() noexcept
    {
        if( false == OnServerStopped() )
        {
            SKL_ERR_FMT("[ServerInstance:%ws] OnServerStopped() Failed!", Config.Name );
        }

        SKL_INF_FMT("[ServerInstance:%ws] All worker groups stopped!", Config.Name );

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

        return true;
    }

    bool ServerInstance::OnAfterServerStopped() noexcept
    {
        SKL_VER_FMT( "[ServerInstance:%ws] Stopped final!", Config.Name );
        return true;
    }
}