//!
//! \file WorkerGroupManager.cpp
//! 
//! \brief Worker group manager abstraction
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

#include "SkylakeLib.h"

namespace SKL
{
    RStatus WorkerGroupManager::Initialize( ApplicationWorkersConfig&& InConfig ) noexcept
    {
        SKL_ASSERT_ALLWAYS( Skylake_IsTheLibraryInitialize() );

        if( false == InConfig.IsValid() )
        {
            return RInvalidParamters;
        }

        // save config
        Config = std::forward<ApplicationWorkersConfig>( InConfig );

        // create worker groups
        for( size_t i = 0; i < Config.WorkerGroups.size(); ++i )
        {
            const auto& WorkerConfig                { Config.WorkerGroups[i] };
            const bool  bDoesMasterNeedsToBeCreated { i == Config.WorkerGroups.size() - 1 };
 
            if( false == CreateWorkerGroup( WorkerConfig, bDoesMasterNeedsToBeCreated ) )
            {
                SKL_ERR_FMT( "WorkerGroupManager[%ws]::Initialize()", Config.Name );
                return RFail;
            }
        }

        return RSuccess;
    }

    RStatus WorkerGroupManager::StartRunningWithCallingThreadAsMaster() noexcept
    {
        if( false == Config.IsValid() )
        {
            return RInvalidParamters;
        }
        
        for( auto& Group: WorkerGroups )
        {
            if( RSuccess != Group->Start() )
            {
                SKL_ERR_FMT("[WorkerGroup:%ws] Failed to start!", Group->GetTag().Name );
                goto fail_case;
            }
        }

        // call run on the calling thread
        MasterWorker->RunImpl();

        JoinAllGroups();

        SKL_INF_FMT( "[%ws] All worker groups stopped!", Config.Name );

        return RSuccess;

    fail_case:

        for( auto& Group: WorkerGroups )
        {
            Group->Stop();
        }

        return RFail;
    }
    

    bool WorkerGroupManager::CreateWorkerGroup( const ApplicationWorkerGroupConfig& Config, bool bCreateMaster ) noexcept
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
        if( bCreateMaster )
        {
            MasterWorker = NewGroup->GetTheMasterWorker();
            SKL_ASSERT_ALLWAYS( nullptr != MasterWorker.get() );
        }
        
        // save
        WorkerGroups.emplace_back( std::move( NewGroup ) );

        return true;
    }

    void WorkerGroupManager::OnWorkerGroupStarted( WorkerGroup& Group ) noexcept
    {
        const auto NewActiveWorkerGroupsCount { ActiveWorkerGroups.increment() };
        if( NewActiveWorkerGroupsCount == TotalWorkerGroups.load_relaxed() )
        {
            OnAllWorkerGroupsStarted();
        }

        SKL_INF_FMT("[WorkerGroup:%ws] started!", Group.GetTag().Name );
    }
    
    void WorkerGroupManager::OnWorkerGroupStopped( WorkerGroup& Group ) noexcept
    {
        const auto NewActiveWorkerGroupsCount { ActiveWorkerGroups.decrement() };
        if( NewActiveWorkerGroupsCount == 0 )
        {
            OnAllWorkerGroupsStopped();
        }

        SKL_INF_FMT("[WorkerGroup:%ws] stopped!", Group.GetTag().Name );
    }
    
    void WorkerGroupManager::OnAllWorkerGroupsStarted() noexcept
    {
        SKL_INF_FMT("[%ws] All worker groups started!", Config.Name );
    }
    
    void WorkerGroupManager::OnAllWorkerGroupsStopped() noexcept
    {
        SKL_INF_FMT("[%ws] All worker groups stopped!", Config.Name );
    }
}