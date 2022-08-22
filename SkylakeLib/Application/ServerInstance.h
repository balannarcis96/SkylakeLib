//!
//! \file ServerInstance.h
//! 
//! \brief Server instance abstraction
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    class ServerInstance
    {
    public:   
        using WorkerGroupConfig = ServerInstanceConfig::WorkerGroupConfig;
        using ServerInstanceConfig = ServerInstanceConfig::ServerInstanceConfig;

        ServerInstance() noexcept = default;
        virtual ~ServerInstance() noexcept  
        {
            SKL_ASSERT( false == IsAnyWorkerGroupRunning() );
        }

        //! Initialize the server instance
        RStatus Initialize( ServerInstanceConfig&& InConfig ) noexcept;

        //! Start all worker groups and use the calling thread for the master worker
        RStatus StartServer() noexcept;
        
        //! Join all worker groups
        void JoinAllGroups() noexcept
        {
            for( auto& Group : WorkerGroups )
            {
                if( nullptr == Group ) { continue; }
                Group->Join();
            }
        }

        //! Get configuration
        const ServerInstanceConfig& GetConfig() const noexcept { return Config; }

        //! Get worker group by id
        std::shared_ptr<WorkerGroup> GetWorkerGroupById( uint16_t Id ) noexcept 
        {
            for( auto& Group: WorkerGroups )
            {
                if( nullptr == Group ) { continue; }
                if( Group->GetTag().Id == Id )
                {
                    return Group;
                }
            }

            return { nullptr };
        }
    
        //! Get worker group by using the Id as index 
        std::shared_ptr<WorkerGroup> GetWorkerGroupWithIdAsIndex( uint16_t Id ) noexcept 
        {
            SKL_ASSERT( WorkerGroups.size() > Id );
            return WorkerGroups[ Id ];
        }
    
        //! Get worker group by using the Id as index 
        WorkerGroup* GetWorkerGroupWithIdAsIndexRaw( uint16_t Id ) noexcept 
        {
            SKL_ASSERT( WorkerGroups.size() > Id );
            return WorkerGroups[ Id ].get();
        }

        //! Get worker group by using the Id as index 
        const std::shared_ptr<WorkerGroup> GetWorkerGroupWithIdAsIndex( uint16_t Id ) const noexcept 
        {
            SKL_ASSERT( WorkerGroups.size() > Id );
            return WorkerGroups[ Id ];
        }
    
        //! Get worker group by using the Id as index 
        const WorkerGroup* GetWorkerGroupWithIdAsIndexRaw( uint16_t Id ) const noexcept 
        {
            SKL_ASSERT( WorkerGroups.size() > Id );
            return WorkerGroups[ Id ].get();
        }

        //! Signal all worker groups to stop
        void SignalToStop( bool bForce = true) noexcept
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

            for( auto& Group: WorkerGroups )
            {
                if( nullptr == Group ) { continue; }
                Group->SignalToStop();
            }
        }

        //! Is any woker group running now
        bool IsAnyWorkerGroupRunning() const noexcept   
        {
            for( auto& Group: WorkerGroups )
            {
                if( nullptr == Group ) { continue; }
                if( true == Group->IsRunning() )
                {
                    return true;
                }
            }

            return false;
        }

        //! Is the server instance running
        bool IsRunning() const noexcept { return bIsRunning.load_acquire(); }

        //! Get the name of the server instance
        const wchar_t* GetName() noexcept { return Config.Name; }

        //! Get server flags
        ServerInstanceFlags GetFlags() const noexcept { return ServerBuiltFlags; }

    protected:
        virtual bool OnWorkerStarted( Worker& InWorker, WorkerGroup& Group ) noexcept;
        virtual bool OnWorkerStopped( Worker& InWorker, WorkerGroup& Group ) noexcept;
        virtual bool OnAllWorkersStarted( WorkerGroup& Group ) noexcept;
        virtual bool OnAllWorkersStopped( WorkerGroup& Group ) noexcept;
        virtual bool OnWorkerGroupStarted( WorkerGroup& Group ) noexcept;
        virtual bool OnWorkerGroupStopped( WorkerGroup& Group ) noexcept;
        virtual bool OnAllWorkerGroupsStarted() noexcept;
        virtual bool OnAllWorkerGroupsStopped() noexcept;
        virtual bool OnBeforeStartServer() noexcept;
        virtual bool OnServerStarted() noexcept;
        virtual bool OnBeforeStopServer() noexcept;
        virtual bool OnServerStopped() noexcept;
        virtual bool OnAfterServerStopped() noexcept;

        bool CreateWorkerGroup( const WorkerGroupConfig& Config, bool bCreateMaster ) noexcept;

    private:
        std::vector<std::shared_ptr<WorkerGroup>> WorkerGroups                   {};        //!< List of all workers groups
        std::shared_ptr<Worker>                   MasterWorker                   {};        //!< Cached pointer to the master worker
        std::synced_value<uint32_t>               ActiveWorkerGroups             { 0 };     //!< Number of running worker groups
        std::synced_value<uint32_t>               TotalWorkerGroups              { 0 };     //!< Total number of worker groups
        std::relaxed_value<uint32_t>              bIsRunning                     { FALSE }; //!< Is the server running
        ServerInstanceFlags                       ServerBuiltFlags               {};
        std::vector<std::shared_ptr<WorkerGroup>> DeferredTasksHandlingGroups    {};        //!< All active worker groups marked with [bHandlesTimerTasks=true]
        std::vector<std::shared_ptr<WorkerGroup>> DeferredAODTasksHandlingGroups {};        //!< All active worker groups marked with [bSupportsAOD=true]
        ServerInstanceConfig                      Config                         {};        //!< Config

        friend class Worker;
        friend class WorkerGroup;
        friend class ServerInstance;
        friend struct AODTLSContext;
        friend struct ServerInstanceTLSContext;
    };
}