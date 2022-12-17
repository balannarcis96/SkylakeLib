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
        SKL_NODISCARD RStatus Initialize( ServerInstanceConfig&& InConfig ) noexcept;

        //! Start all worker groups and use the calling thread for the master worker
        SKL_NODISCARD RStatus StartServer() noexcept;
        
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
        SKL_FORCEINLINE SKL_NODISCARD const ServerInstanceConfig& GetConfig() const noexcept { return Config; }

        //! Get worker group by id
        SKL_NODISCARD WorkerGroup* GetWorkerGroupById( uint16_t Id ) const noexcept 
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
        SKL_FORCEINLINE SKL_NODISCARD WorkerGroup* GetWorkerGroupWithIdAsIndex( uint16_t Id ) const noexcept 
        {
            SKL_ASSERT( WorkerGroups.size() > Id );
            return WorkerGroups[ Id ];
        }

        //! Get all worker groups in this server instance 
        SKL_FORCEINLINE SKL_NODISCARD std::vector<WorkerGroup*>& GetAllWorkerGroups() noexcept { return WorkerGroups; }

        //! Get all worker groups in this server instance 
        SKL_FORCEINLINE SKL_NODISCARD const std::vector<WorkerGroup*>& GetAllWorkerGroups() const noexcept { return WorkerGroups; }

        //! Signal all worker groups to stop
        void SignalToStop( bool bForce = true ) noexcept;

        //! Is any worker group running now
        SKL_NODISCARD bool IsAnyWorkerGroupRunning() const noexcept   
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
        SKL_FORCEINLINE SKL_NODISCARD bool IsRunning() const noexcept { return bIsRunning.load_acquire(); }

        //! Get the name of the server instance
        SKL_FORCEINLINE SKL_NODISCARD const wchar_t* GetName() noexcept { return Config.Name; }

        //! Get server flags
        SKL_FORCEINLINE SKL_NODISCARD ServerInstanceFlags GetFlags() const noexcept { return ServerBuiltFlags; }
        
        //! Get service API
        const std::vector<TServicePtr<SimpleService>>& GetAllSimpleServices() const noexcept { return SimpleServices; }
        const std::vector<TServicePtr<AODService>>&    GetAllAODServices() const noexcept { return AODServices; }
        const std::vector<TServicePtr<ActiveService>>& GetAllActiveServices() const noexcept { return ActiveServices; }
        const std::vector<TServicePtr<WorkerService>>& GetAllWorkerServices() const noexcept { return WorkerServices; }
        const std::vector<IService*>&                      GetAllServices() const noexcept { return AllServices; }

        //! Get a service by UID. O(n)
        template<typename TService = IService>
        SKL_NODISCARD TService* GetServiceById( uint32_t UID ) const noexcept
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

        //! Get simple service by UID. O(n)
        template<typename TService = SimpleService>
        SKL_NODISCARD SimpleService* GetSimpleServiceById( uint32_t UID ) const noexcept
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

        //! Get AOD service by UID. O(n)
        template<typename TService = AODService>
        SKL_NODISCARD AODService* GetAODServiceById( uint32_t UID ) const noexcept
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

        //! Get active service by UID. O(n)
        template<typename TService = ActiveService>
        SKL_NODISCARD TService* GetActiveServiceById( uint32_t UID ) const noexcept
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

        //! Get worker service by UID. O(n)
        template<typename TService = WorkerService>
        SKL_NODISCARD TService* GetWorkerServiceById( uint32_t UID ) const noexcept
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

        //! Get simple service by UID as index. O(1)
        template<typename TService = SimpleService>
        SKL_FORCEINLINE SKL_NODISCARD TService* GetSimpleServiceWithIdAsIndex( uint32_t UID ) const noexcept
        {
            static_assert( std::is_same_v<TService, SimpleService> || std::is_base_of_v<SimpleService, TService> );

            SKL_ASSERT( 0 != UID && UID < static_cast<uint32_t>( SimpleServices.size() ) );
            return static_cast<TService*>( SimpleServices[ UID ].get() );
        }

        //! Get AOD service by UID as index. O(1)
        template<typename TService = AODService>
        SKL_FORCEINLINE SKL_NODISCARD TService* GetAODServiceWithIdAsIndex( uint32_t UID ) const noexcept
        {
            static_assert( std::is_same_v<TService, AODService> || std::is_base_of_v<AODService, TService> );

            SKL_ASSERT( 0 != UID && UID < static_cast<uint32_t>( AODServices.size() ) );
            return static_cast<TService*>( AODServices[ UID ].get() );
        }

        //! Get active service by UID as index. O(1)
        template<typename TService = ActiveService>
        SKL_FORCEINLINE SKL_NODISCARD TService* GetActiveServiceWithIdAsIndex( uint32_t UID ) const noexcept
        {
            static_assert( std::is_same_v<TService, ActiveService> || std::is_base_of_v<ActiveService, TService> );
            SKL_ASSERT( 0 != UID && UID < static_cast<uint32_t>( ActiveServices.size() ) );
            return static_cast<TService*>( ActiveServices[ UID ].get() );
        }

        //! Get worker service by UID as index. O(1)
        template<typename TService = WorkerService>
        SKL_FORCEINLINE SKL_NODISCARD TService* GetWorkerServiceWithIdAsIndex( uint32_t UID ) const noexcept
        {
            static_assert( std::is_same_v<TService, WorkerService> || std::is_base_of_v<WorkerService, TService> );
            SKL_ASSERT( 0 != UID && UID < static_cast<uint32_t>( WorkerServices.size() ) );
            return static_cast<TService*>( WorkerServices[ UID ].get() );
        }

        //! ADD service API
        bool AddService( SimpleService* InService ) noexcept;
        bool AddService( AODService* InService ) noexcept;
        bool AddService( ActiveService* InService ) noexcept;
        bool AddService( WorkerService* InService ) noexcept;
        
        //! Get the total number of worker groups in this server instance
        SKL_FORCEINLINE SKL_NODISCARD uint32_t GetTotalWorkerGroupsCount() const noexcept { return TotalWorkerGroups.load_relaxed(); }

        //! Get the total number of workers running in this server instance
        SKL_FORCEINLINE SKL_NODISCARD uint32_t GetTotalNumberOfRunningWorkers() const noexcept { return TotalNumberOfRunningWorkers.load_relaxed(); }
        
        //! Does this server have at least one worker that support TLS Sync
        SKL_FORCEINLINE SKL_NODISCARD bool SupportsTSLSync() const noexcept
        {
            return nullptr != MyTLSSyncSystem.get();
        }
        
        //! Get reference to the TLSSyncSystem instance for this server application
        SKL_FORCEINLINE SKL_NODISCARD TLSSyncSystem& GetTSLSyncSystem() noexcept
        {
            SKL_ASSERT( SupportsTSLSync() );
            return *MyTLSSyncSystem;
        }
        
        //! Get reference to the TLSSyncSystem instance for this server application
        SKL_FORCEINLINE SKL_NODISCARD const TLSSyncSystem& GetTSLSyncSystem() const noexcept
        {
            SKL_ASSERT( SupportsTSLSync() );
            return *MyTLSSyncSystem;
        }

        //! Get the number of workers that support TLS Sync
        SKL_FORCEINLINE SKL_NODISCARD uint64_t GetNoOfWorkersThatSupportTLSSync() const noexcept
        {
            if( false == SupportsTSLSync() )
            {
                return 0;
            }

            return MyTLSSyncSystem->GetNoOfWorkersThatSupportTLSSync();
        }
        
        //! Get pointer to the TLSSyncSystem instance for this server application
        SKL_FORCEINLINE SKL_NODISCARD TLSSyncSystem* GetTSLSyncSystemPtr() noexcept
        {
            return MyTLSSyncSystem.get();
        }
        
        //! Issue a new TLS sync task on all worker groups that support TLS Sync [WorkerGroup.Tag.bSupportsTLSSync=true]
        //! \remarks accepted signature void( SKL_CDECL* )( Worker&, WorkerGroup&, bool ) noexcept
        template<typename TFunctor>
        void SyncTLS( TFunctor&& InFunctor ) noexcept
        {
            SKL_ASSERT( SupportsTSLSync() );

            auto& TLSSyncSystemInstance{ GetTSLSyncSystem() };
            
            auto* Task{ MakeTLSSyncTaskRaw( static_cast<uint16_t>( TLSSyncSystemInstance.GetNoOfWorkersThatSupportTLSSync() ), std::forward<TFunctor>( InFunctor ) ) };
            SKL_ASSERT( nullptr != Task );
            
            TLSSyncSystemInstance.PushTask( Task );
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

        bool CreateWorkerGroup( const WorkerGroupConfig& InConfig, bool bCreateMaster ) noexcept;

    private:
        std::cacheline_unique_ptr<TLSSyncSystem>    MyTLSSyncSystem                { nullptr }; //!< Instance of the TLSSyncSystem
        std::vector<TServicePtr<SimpleService>>     SimpleServices                 {};          //!< All simple service instances
        std::vector<TServicePtr<AODService>>        AODServices                    {};          //!< All AOD service instances
        std::vector<TServicePtr<ActiveService>>     ActiveServices                 {};          //!< All Active service instances
        std::vector<TServicePtr<WorkerService>>     WorkerServices                 {};          //!< All Worker service instances
        std::vector<IService*>                      AllServices                    {};          //!< Base interface pointer to all services
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