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

        //! Set functor to be called each time a worker in the group ticks [void( SKL_CDECL* )( WorkerGroup&, Worker& ) noexcept]
        template<typename TFunctor>
        void SetWorkerTickHandler( TFunctor&& InOnWorkerTick ) noexcept 
        {
            OnWorkerTick += std::forward<TFunctor>( InOnWorkerTick );
        }
    
        //! DO NOT CALL
        const WorkerGroup::WorkerTickTask& GetTaskToDispatch() const noexcept 
        {
            return OnWorkerTick;
        }

    private:
        WorkerGroupTag              Tag          {}; //!< Group tag
        WorkerGroup::WorkerTickTask OnWorkerTick {}; //!< Task to be executed each time a worker in the group ticks

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

        }

        //! Initialize the group manager
        RStatus Initialize( ApplicationWorkersConfig&& InConfig ) noexcept;

        //! Start all worker groups and use the calling thread as for the master worker
        RStatus StartRunningWithCallingThreadAsMaster() noexcept;
        
        //! Get join all worker groups
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