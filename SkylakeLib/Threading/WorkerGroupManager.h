//!
//! \file WorkerGroupManager.h
//! 
//! \brief Worker group manager abstraction
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    struct ApplicationWorkerGroupConfig
    {
        ApplicationWorkerGroupConfig() noexcept = default;
        ApplicationWorkerGroupConfig( const WorkerGroupTag& GroupTag ) noexcept : Tag{ GroupTag } {}

        //! Set the group tag [mandatory]
        void SetTag( const WorkerGroupTag& GroupTag ) noexcept 
        {
            Tag = GroupTag;
        }
        
        //! Is this config valid
        bool IsValid() const noexcept { return true == Tag.IsValid(); }

        //! Set functor to be called each time a worker in the group ticks [void( SKL_CDECL* )( Worker&, WorkerGroup& ) noexcept]
        template<typename TFunctor>
        void SetWorkerTickHandler( TFunctor&& InOnWorkerTick ) noexcept 
        {
            OnWorkerTick += std::forward<TFunctor>( InOnWorkerTick );
        }
    
        //! Set functor to be called each time a worker in the group starts [void( SKL_CDECL* )( Worker&, WorkerGroup& ) noexcept]
        template<typename TFunctor>
        void SetWorkerStartHandler( TFunctor&& InOnWorkerStart ) noexcept 
        {
            OnWorkerStart += std::forward<TFunctor>( InOnWorkerStart );
        }
    
        //! Set functor to be called each time a worker in the group stops [void( SKL_CDECL* )( Worker&, WorkerGroup& ) noexcept]
        template<typename TFunctor>
        void SetWorkerStopHandler( TFunctor&& InOnWorkerStop ) noexcept 
        {
            OnWorkerStop += std::forward<TFunctor>( InOnWorkerStop );
        }
    
        //! Add new tcp async acceptor for this worker group
        bool AddTCPAsyncAcceptor( const TCPAcceptorConfig& Config ) noexcept 
        {
            TCPAcceptorConfigs.push_back( Config );
        }

        //! DO NOT CALL
        const WorkerGroup::WorkerTask& GetTaskToDispatch() const noexcept 
        {
            return OnWorkerTick;
        }

    private:
        WorkerGroupTag                  Tag               {}; //!< Group tag
        WorkerGroup::WorkerTask         OnWorkerTick      {}; //!< Task to be executed each time a worker in the group ticks
        WorkerGroup::WorkerTask         OnWorkerStart     {}; //!< Task to be executed each time a worker in the group start
        WorkerGroup::WorkerTask         OnWorkerStop      {}; //!< Task to be executed each time a worker in the group stops
        std::vector<TCPAcceptorConfig>  TCPAcceptorConfigs{}; //!< List of all tcp async acceptors to create to be handled by the workers in the group

        friend WorkerGroupManager;
    }; 

    struct ApplicationWorkersConfig
    {
        ApplicationWorkersConfig() noexcept = default;
        ApplicationWorkersConfig( const wchar_t* Name ) noexcept : Name { Name } {}

        //! Add new worker group config 
        void AddNewGroup( ApplicationWorkerGroupConfig&& GroupTag ) noexcept 
        {
            WorkerGroups.emplace_back( std::forward<ApplicationWorkerGroupConfig>( GroupTag ) );
        }

        //! Is this config valid
        bool IsValid() const noexcept 
        { 
            if( true == WorkerGroups.empty() || nullptr == Name )
            {
                return false;
            }

            for( const auto& WorkerGroupConfig: WorkerGroups )
            {
                if( false == WorkerGroupConfig.IsValid() )
                {
                    return false;
                }
            }

            return true;
        }

    private:
        const wchar_t*                            Name         { nullptr }; //!< Workers manager instance name
        std::vector<ApplicationWorkerGroupConfig> WorkerGroups {};          //!< Config for all needed worker groups

        friend WorkerGroupManager;
    };  

    class WorkerGroupManager final
    {
    public:   
        WorkerGroupManager() noexcept = default;
        ~WorkerGroupManager() noexcept  
        {
            SKL_ASSERT( false == IsAnyWorkerGroupRunning() );
        }

        //! Initialize the manager
        RStatus Initialize( ApplicationWorkersConfig&& InConfig ) noexcept;

        //! Start all worker groups and use the calling thread for the master worker
        RStatus StartRunningWithCallingThreadAsMaster() noexcept;
        
        //! Join all worker groups
        void JoinAllGroups() noexcept
        {
            for( auto& Group : WorkerGroups )
            {
                Group->Join();
            }
        }

        //! Get configuration
        const ApplicationWorkersConfig& GetConfig() const noexcept { return Config; }

        //! Get worker group by id
        std::shared_ptr<WorkerGroup> GetWorkerGroupById( uint16_t Id ) noexcept 
        {
            for( auto& Group: WorkerGroups )
            {
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
        void SignalToStop() noexcept
        {
            for( auto& Group: WorkerGroups )
            {
                Group->SignalToStop();
            }
        }

        //! Is any woker group running now
        bool IsAnyWorkerGroupRunning() const noexcept   
        {
            for( auto& Group: WorkerGroups )
            {
                if( true == Group->IsRunning() )
                {
                    return true;
                }
            }

            return false;
        }

    private:
        void OnWorkerGroupStarted( WorkerGroup& Group ) noexcept;
        void OnWorkerGroupStopped( WorkerGroup& Group ) noexcept;
        void OnAllWorkerGroupsStarted() noexcept;
        void OnAllWorkerGroupsStopped() noexcept;

        bool CreateWorkerGroup( const ApplicationWorkerGroupConfig& Config, bool bCreateMaster ) noexcept;

    private:
        std::vector<std::shared_ptr<WorkerGroup>> WorkerGroups       {};    //!< List of all workers groups
        std::shared_ptr<Worker>                   MasterWorker       {};    //!< Cached pointer to the master worker
        std::synced_value<uint32_t>               ActiveWorkerGroups { 0 }; //!< Number of running worker groups
        std::synced_value<uint32_t>               TotalWorkerGroups  { 0 }; //!< Total number of worker groups
        ApplicationWorkersConfig                  Config             {};    //!< Config

        friend class Worker;
        friend class WorkerGroup;
    };
}