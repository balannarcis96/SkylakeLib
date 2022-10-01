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
            JoinAllGroups();
            SKL_ASSERT( false == IsAnyWorkerGroupRunning() );

            for( auto* Group : WorkerGroups )
            {
                if( Group != nullptr )
                {
                   delete Group;
                }
            }
            WorkerGroups.clear();
        }

        //! Initialize the server instance
        RStatus Initialize( ServerInstanceConfig&& InConfig ) noexcept;

        //! Start all worker groups and use the calling thread for the master worker
        RStatus StartServer() noexcept;
        
        //! Join all worker groups
        void JoinAllGroups() noexcept
        {
            for( auto* Group : WorkerGroups )
            {
                if( nullptr == Group ) { continue; }
                Group->Join();
            }
        }

        //! Get configuration
        const ServerInstanceConfig& GetConfig() const noexcept { return Config; }

        //! Get worker group by id
        WorkerGroup* GetWorkerGroupById( uint16_t Id ) const noexcept 
        {
            for( auto* Group: WorkerGroups )
            {
                if( nullptr == Group ) { continue; }
                if( Group->GetTag().Id == Id )
                {
                    return Group;
                }
            }

            return nullptr;
        }

        //! Get worker group by using the Id as index 
        SKL_FORCEINLINE WorkerGroup* GetWorkerGroupWithIdAsIndex( uint16_t Id ) const noexcept 
        {
            SKL_ASSERT( WorkerGroups.size() > Id );
            return WorkerGroups[ Id ];
        }

        //! Get all worker groups in this server instance 
        SKL_FORCEINLINE std::vector<WorkerGroup*>& GetAllWorkerGroups() noexcept { return WorkerGroups; }

        //! Get all worker groups in this server instance 
        SKL_FORCEINLINE const std::vector<WorkerGroup*>& GetAllWorkerGroups() const noexcept { return WorkerGroups; }

        //! Signal all worker groups to stop
        void SignalToStop( bool bForce = true ) noexcept;

        //! Is any woker group running now
        bool IsAnyWorkerGroupRunning() const noexcept   
        {
            for( auto* Group: WorkerGroups )
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
        SKL_FORCEINLINE bool IsRunning() const noexcept { return bIsRunning.load_acquire(); }

        //! Get the name of the server instance
        SKL_FORCEINLINE const wchar_t* GetName() noexcept { return Config.Name; }

        //! Get server flags
        SKL_FORCEINLINE ServerInstanceFlags GetFlags() const noexcept { return ServerBuiltFlags; }
        
        //! Get service API
        const std::vector<std::unique_ptr<SimpleService>>& GetAllSimpleServices() const noexcept { return SimpleServices; }
        const std::vector<std::unique_ptr<AODService>>&    GetAllAODServices() const noexcept { return AODServices; }
        const std::vector<std::unique_ptr<ActiveService>>& GetAllActiveServices() const noexcept { return ActiveServices; }
        const std::vector<std::unique_ptr<WorkerService>>& GetAllWorkerServices() const noexcept { return WorkerServices; }
        const std::vector<IService*>&                      GetAllServices() const noexcept { return AllServices; }

        template<typename TService = IService>
        TService* GetServiceById( uint32_t UID ) const noexcept
        {
            static_assert( 
                   std::is_same_v<IService, TService> 
                || std::is_base_of_v<SimpleService, TService> 
                || std::is_base_of_v<AODService, TService> 
                || std::is_base_of_v<ActiveService, TService> 
                || std::is_base_of_v<WorkerService, TService> 
            );

            SKL_ASSERT( 0 != UID );
            for( auto* Service : AllServices )
            {
                if( nullptr == Service ){ continue; }
                if( UID == Service->GetUID() )
                {
                    return static_cast<TService*>( Service );
                }
            }

            return nullptr;
        }

        template<typename TService = SimpleService>
        SimpleService* GetSimpleServiceById( uint32_t UID ) const noexcept
        {
            static_assert( std::is_same_v<TService, ActiveService> || std::is_base_of_v<ActiveService, TService> );

            SKL_ASSERT( 0 != UID );
            for( auto& Service : SimpleServices )
            {
                if( nullptr == Service ){ continue; }
                if( UID == Service->GetUID() )
                {
                    return static_cast<TService*>( Service.get() );
                }
            }

            return nullptr;
        }

        template<typename TService = AODService>
        AODService* GetAODServiceById( uint32_t UID ) const noexcept
        {
            static_assert( std::is_same_v<TService, ActiveService> || std::is_base_of_v<ActiveService, TService> );

            SKL_ASSERT( 0 != UID );
            for( auto& Service : AODServices )
            {
                if( nullptr == Service ){ continue; }
                if( UID == Service->GetUID() )
                {
                    return static_cast<TService*>( Service.get() );
                }
            }

            return nullptr;
        }

        template<typename TService = ActiveService>
        TService* GetActiveServiceById( uint32_t UID ) const noexcept
        {
            static_assert( std::is_same_v<TService, ActiveService> || std::is_base_of_v<ActiveService, TService> );

            SKL_ASSERT( 0 != UID );
            for( auto& Service : ActiveServices )
            {
                if( nullptr == Service ){ continue; }
                if( UID == Service->GetUID() )
                {
                    return static_cast<TService*>( Service.get() );
                }
            }

            return nullptr;
        }

        template<typename TService = WorkerService>
        TService* GetWorkerServiceById( uint32_t UID ) const noexcept
        {
            static_assert( std::is_same_v<TService, WorkerService> || std::is_base_of_v<WorkerService, TService> );

            SKL_ASSERT( 0 != UID );
            for( auto& Service : WorkerServices )
            {
                if( nullptr == Service ){ continue; }
                if( UID == Service->GetUID() )
                {
                    return static_cast<TService*>( Service.get() );
                }
            }

            return nullptr;
        }

        template<typename TService = SimpleService>
        SKL_FORCEINLINE TService* GetSimpleServiceWithIdAsIndex( uint32_t UID ) const noexcept
        {
            static_assert( std::is_same_v<TService, SimpleService> || std::is_base_of_v<SimpleService, TService> );

            SKL_ASSERT( 0 != UID && UID < static_cast<uint32_t>( SimpleServices.size() ) );
            return static_cast<TService*>( SimpleServices[ UID ].get() );
        }

        template<typename TService = AODService>
        SKL_FORCEINLINE TService* GetAODServiceWithIdAsIndex( uint32_t UID ) const noexcept
        {
            static_assert( std::is_same_v<TService, AODService> || std::is_base_of_v<AODService, TService> );

            SKL_ASSERT( 0 != UID && UID < static_cast<uint32_t>( AODServices.size() ) );
            return static_cast<TService*>( SimpleServices[ UID ].get() );
        }

        template<typename TService = ActiveService>
        SKL_FORCEINLINE TService* GetActiveServiceWithIdAsIndex( uint32_t UID ) const noexcept
        {
            static_assert( std::is_same_v<TService, ActiveService> || std::is_base_of_v<ActiveService, TService> );
            SKL_ASSERT( 0 != UID && UID < static_cast<uint32_t>( ActiveServices.size() ) );
            return static_cast<TService*>( SimpleServices[ UID ].get() );
        }

        template<typename TService = WorkerService>
        SKL_FORCEINLINE TService* GetWorkerServiceWithIdAsIndex( uint32_t UID ) const noexcept
        {
            static_assert( std::is_same_v<TService, WorkerService> || std::is_base_of_v<WorkerService, TService> );
            SKL_ASSERT( 0 != UID && UID < static_cast<uint32_t>( WorkerServices.size() ) );
            return static_cast<TService*>( SimpleServices[ UID ].get() );
        }

        //! ADD service API
        bool AddService( SimpleService* InService ) noexcept;
        bool AddService( AODService* InService ) noexcept;
        bool AddService( ActiveService* InService ) noexcept;
        bool AddService( WorkerService* InService ) noexcept;
        
        uint32_t GetTotalGroupsCount() const noexcept { return TotalWorkerGroups.load_relaxed(); }
        uint32_t GetTotalNumberOfRunningWorkers() const noexcept { return TotalNumberOfRunningWorkers.load_relaxed(); }

        //! Issue a new TLS sync task on a specific worker group that support TLS Sync [bSupportsTLSSync=true] by Id
        template<typename TFunctor>
        SKL_FORCEINLINE void SyncTSLOnGroupById( uint16_t GroupId, TFunctor&& InFunctor ) noexcept
        {
            SKL_ASSERT( false == TLSSyncHandlingGroup.empty() );
            auto* GroupPtr{ GetWorkerGroupById( GroupId ) };
            SKL_ASSERT( nullptr != GroupPtr );
            SKL_ASSERT( true == GroupPtr->GetTag().bSupportsTLSSync );

            GroupPtr->SyncTSL( std::forward<TFunctor>( InFunctor ) );
        }
        
        //! Issue a new TLS sync task on a specific worker group that support TLS Sync [bSupportsTLSSync=true] by Id as index
        template<typename TFunctor>
        SKL_FORCEINLINE void SyncTSLOnGroupByIdAsIndex( uint16_t GroupId, TFunctor&& InFunctor ) noexcept
        {
            SKL_ASSERT( false == TLSSyncHandlingGroup.empty() );
            auto* GroupPtr{ GetWorkerGroupWithIdAsIndex( GroupId ) };
            SKL_ASSERT( nullptr != GroupPtr );
            SKL_ASSERT( true == GroupPtr->GetTag().bSupportsTLSSync );

            GroupPtr->SyncTSL( std::forward<TFunctor>( InFunctor ) );
        }

    protected:
        virtual bool OnAddServices() noexcept { return true; }
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
        virtual void OnServiceStopped( IService* InService, RStatus InStatus ) noexcept;
        virtual void OnAllServiceStopped() noexcept;

        bool CreateWorkerGroup( const WorkerGroupConfig& Config, bool bCreateMaster ) noexcept;

    private:
        std::vector<WorkerGroup*>                   WorkerGroups                   {};          //!< List of all workers groups
        Worker*                                     MasterWorker                   { nullptr }; //!< Cached pointer to the master worker
        ServerInstanceFlags                         ServerBuiltFlags               {};          //!< Server instance flags
        std::synced_value<uint32_t>                 ActiveWorkerGroups             { 0 };       //!< Number of running worker groups
        std::synced_value<uint32_t>                 TotalWorkerGroups              { 0 };       //!< Total number of worker groups
        std::synced_value<uint32_t>                 TotalWorkers                   { 0 };       //!< Total number of workers
        std::vector<WorkerGroup*>                   DeferredTasksHandlingGroups    {};          //!< All active worker groups marked with [bHandlesTimerTasks=true]
        std::vector<WorkerGroup*>                   DeferredAODTasksHandlingGroups {};          //!< All active worker groups marked with [bSupportsAOD=true]
        std::vector<WorkerGroup*>                   TLSSyncHandlingGroup           {};          //!< All worker groups marked with [bSupportsTLSSync=true]
        std::relaxed_value<uint32_t>                bIsRunning                     { FALSE };   //!< Is the server running
        std::relaxed_value<uint32_t>                TotalNumberOfRunningWorkers    { 0 };       //!< Total number of running workers
        ServerInstanceConfig                        Config                         {};          //!< Config
        std::vector<std::unique_ptr<SimpleService>> SimpleServices                 {};          //!< All simple service instances
        std::vector<std::unique_ptr<AODService>>    AODServices                    {};          //!< All AOD service instances
        std::vector<std::unique_ptr<ActiveService>> ActiveServices                 {};          //!< All Active service instances
        std::vector<std::unique_ptr<WorkerService>> WorkerServices                 {};          //!< All Worker service instances
        std::vector<IService*>                      AllServices                    {};          //!< Base interface pointer to all services
        std::relaxed_value<uint32_t>                TotalNumberOfInitServices      { 0 };       //!< Total number initialized services
        std::unique_ptr<std::latch>                 SyncWorkerStartup              {};          //!< Latch used to sync all workers startup
        std::unique_ptr<std::latch>                 SyncWorkerShutdown             {};          //!< Latch used to sync all workers shutdown

        friend class Worker;
        friend class WorkerGroup;
        friend class ServerInstance;
        friend class IService;
        friend struct AODTLSContext;
        friend struct ServerInstanceTLSContext;
    };
}