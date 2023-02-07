//!
//! \file WorkerGroup.h
//! 
//! \brief Worker group abstraction
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{   
    class WorkerGroup
    {
    public:
        using WorkerTask = ASD::CopyFunctorWrapper<32, bool( SKL_CDECL* )( Worker&, WorkerGroup& ) noexcept>;
        using WorkerTickTask = ASD::CopyFunctorWrapper<32, void( SKL_CDECL* )( Worker&, WorkerGroup& ) noexcept>;

        WorkerGroup( const WorkerGroupTag& Tag, ServerInstance* Manager ) noexcept 
            : Tag{ Tag }
            , Manager{ Manager } 
        {
            (void)Tag.Validate();

            SKL_ASSERT( true == Tag.IsValid() );
            TCPAcceptors.reserve( 32 );
            TCPAcceptors.emplace_back( nullptr ); //index 0 is invalid
        }
        ~WorkerGroup() noexcept
        {
            Join();

            if( 0 != GetNumberOfRunningWorkers() )
            {
                SKL_BREAK();
            }

            SKL_ASSERT_ALLWAYS( 0 == GetNumberOfRunningWorkers() );

            MasterWorker = nullptr;
        }

        //! Set functor to be called each time a worker ticks [void( SKL_CDECL* )( Worker&, WorkerGroup& ) noexcept]
        template<typename TFunctor>
        SKL_FORCEINLINE void SetWorkerTickHandler( TFunctor&& InOnWorkerTick ) noexcept 
        {
            OnWorkerTick += std::forward<TFunctor>( InOnWorkerTick );
        }

        //! Set functor to be called each time a worker ticks [void( SKL_CDECL* )( Worker&, WorkerGroup& ) noexcept]
        SKL_FORCEINLINE void SetWorkerTickHandler( const WorkerTickTask& InOnWorkerTick ) noexcept 
        {
            OnWorkerTick = InOnWorkerTick;
        }

        //! Set functor to be called each time a worker starts [void( SKL_CDECL* )( Worker&, WorkerGroup& ) noexcept]
        template<typename TFunctor>
        SKL_FORCEINLINE void SetWorkerStartHandler( TFunctor&& InOnWorkerStarted ) noexcept 
        {
            OnWorkerStartTask += std::forward<TFunctor>( InOnWorkerStarted );
        }

        //! Set functor to be called each time a worker starts [void( SKL_CDECL* )( Worker&, WorkerGroup& ) noexcept]
        SKL_FORCEINLINE void SetWorkerStartHandler( const WorkerTask& InOnWorkerStarted ) noexcept 
        {
            OnWorkerStartTask = InOnWorkerStarted;
        }

        //! Set functor to be called each time a worker stops [void( SKL_CDECL* )( Worker&, WorkerGroup& ) noexcept]
        template<typename TFunctor>
        SKL_FORCEINLINE void SetWorkerStopHandler( TFunctor&& InOnWorkerStopped ) noexcept 
        {
            OnWorkerStopTask += std::forward<TFunctor>( InOnWorkerStopped );
        }

        //! Set functor to be called each time a worker stops [void( SKL_CDECL* )( Worker&, WorkerGroup& ) noexcept]
        SKL_FORCEINLINE void SetWorkerStopHandler( const WorkerTask& InOnWorkerStopped ) noexcept 
        {
            OnWorkerStopTask = InOnWorkerStopped;
        }

        //! Initialize and prepare all components of this workers group
        SKL_NODISCARD RStatus Build( bool bIncludeMaster ) noexcept;

        //! Attempt to start the worker group
        SKL_NODISCARD RStatus Start() noexcept;

        //! Stop and join all workers in group except the master
        void Stop() noexcept;

        //! Signal all active workers to stop
        void SignalToStop() noexcept;

        //! Join all worker in the group except the master(if present)
        void Join() noexcept;

        //! Is the worker group valid
        SKL_NODISCARD bool IsValid() const noexcept { return Tag.IsValid(); }

        //! Is the worker group running
        SKL_NODISCARD bool IsRunning() const noexcept { return bIsRunning.load_relaxed(); }

        //! Get the number of running workers
        SKL_NODISCARD uint32_t GetNumberOfRunningWorkers() const noexcept { return RunningWorkers.load_relaxed(); }

        //! Get the total number of workers
        SKL_NODISCARD uint32_t GetTotalNumberOfWorkers() const noexcept { return TotalWorkers.load_relaxed(); }

        //! Get the tag of the group
        SKL_NODISCARD const WorkerGroupTag& GetTag() const noexcept { return Tag; }

        //! Get the master worker [if any]
        SKL_NODISCARD Worker* GetTheMasterWorker() noexcept { return MasterWorker; }

        //! Is this group the master workers group 
        SKL_NODISCARD bool IsMasterWorkerGroup() const noexcept { return nullptr != MasterWorker; };

        //! Defer functor execution to any worker in this group through the async IO queue [if the group bEnableAsyncIO=true only!] [void( ITask* Self ) noexcept]
        template<typename TFunctor>
        SKL_NODISCARD RStatus Defer( TFunctor&& InFunctor ) noexcept
        {
            auto* NewTask { MakeTaskRaw( std::forward<TFunctor>( InFunctor ) ) };
            if( nullptr == NewTask ) SKL_UNLIKELY
            {
                return RAllocationFailed;
            }

            return AsyncIOAPI.QueueAsyncWork( reinterpret_cast<TCompletionKey>( NewTask ) );
        }
    
        //! Create new tcp async acceptor on this instance
        SKL_NODISCARD RStatus AddNewTCPAcceptor( const TCPAcceptorConfig& Config ) noexcept;

        //! Query tcp async acceptor by id
        SKL_NODISCARD TCPAcceptor* GetTCPAcceptorById( uint32_t Id ) noexcept
        {
            for( auto& Acceptor : TCPAcceptors )
            {
                if( nullptr == Acceptor ) { continue; }
            
                if( Acceptor->IsValid() && Acceptor->GetConfig().Id == Id )
                {
                    return Acceptor.get();
                }
            }

            return nullptr;
        }

        //! Query tcp async acceptor by ip and port
        SKL_NODISCARD TCPAcceptor* GetTCPAcceptor( uint32_t Ip, uint16_t Port ) noexcept
        {
            for( auto& Acceptor : TCPAcceptors )
            {
                if( nullptr == Acceptor ) { continue; }

                if( Acceptor->IsValid() && Acceptor->GetConfig().IpAddress == Ip && Acceptor->GetConfig().Port == Port )
                {
                    return Acceptor.get();
                }
            }

            return nullptr;
        }

        //! Get tcp async acceptor with the id as index
        SKL_FORCEINLINE SKL_NODISCARD TCPAcceptor* GetTCPAcceptorIdAsIndex( uint32_t Id ) noexcept
        {
            SKL_ASSERT( TCPAcceptors.size() > Id );
            return TCPAcceptors[ Id ].get();
        }

        //! DO NOT CALL
        void ProactiveWorkerRun( Worker& Worker ) noexcept;

        //! DO NOT CALL
        void ReactiveWorkerRun( Worker& Worker ) noexcept;
        
        //! Get the server instance
        SKL_FORCEINLINE SKL_NODISCARD ServerInstance* GetServerInstance() noexcept { return Manager; }
        
        //! Get the server instance
        SKL_FORCEINLINE SKL_NODISCARD const ServerInstance* GetServerInstance() const noexcept { return Manager; }

        //! Get workers 
        SKL_FORCEINLINE SKL_NODISCARD std::vector<std::unique_ptr<Worker>>& GetWorkers() noexcept { return Workers; }

        //! Get workers 
        SKL_FORCEINLINE SKL_NODISCARD const std::vector<std::unique_ptr<Worker>>& GetWorkers() const noexcept { return Workers; }

        //! Get the async io instance for this worker group
        SKL_FORCEINLINE SKL_NODISCARD AsyncIO& GetAsyncIOAPI() noexcept{ return AsyncIOAPI; }
        
        //! Does this worker group supports worker group level TLS Sync
        SKL_FORCEINLINE SKL_NODISCARD bool SupportsTSLSync() const noexcept
        {
            return nullptr != MyTLSSyncSystem.get();
        }
        
        //! Issue a new TLS sync task on all this group [WorkerGroup.Tag.bHasWorkerGroupSpecificTLSSync=true]
        //! \remarks accepted signature void( SKL_CDECL* )( Worker&, WorkerGroup&, bool ) noexcept
        template<typename TFunctor>
        void SyncTLS( TFunctor&& InFunctor ) noexcept
        {
            SKL_ASSERT( SupportsTSLSync() );

            auto& TLSSyncSystemInstance{ *MyTLSSyncSystem };
            
            auto* Task{ MakeTLSSyncTaskRaw( static_cast<uint16_t>( TLSSyncSystemInstance.GetNoOfWorkersThatSupportTLSSync() ), std::forward<TFunctor>( InFunctor ) ) };
            SKL_ASSERT( nullptr != Task );
            
            TLSSyncSystemInstance.PushTask( Task );
        }
        
    private:
        RStatus CreateWorkers( bool bIncludeMaster ) noexcept;

        bool StartAllTCPAcceptors() noexcept;
        void StopAllTCPAcceptors() noexcept;

        bool OnWorkerStarted( Worker& Worker ) noexcept;
        bool OnWorkerStopped( Worker& Worker ) noexcept;
        bool OnAllWorkersStarted() noexcept;
        bool OnAllWorkersStopped() noexcept;

        RStatus HandleSlaveWorker( Worker& Worker ) noexcept;
        RStatus HandleMasterWorker( Worker* MasterWorker ) noexcept;
        bool HandleTasks_Proactive( uint32_t MillisecondsToSleep ) noexcept;
        bool HandleTasks_Reactive() noexcept;
        bool HandleAsyncIOTask( AsyncIOOpaqueType* InOpaque, uint32_t NumberOfBytesTransferred ) noexcept;
        bool HandleTask( TCompletionKey InCompletionKey ) noexcept;
        bool HandleAODDelayedTasks_Local( Worker& Worker ) noexcept;
        bool HandleTimerTasks_Local( Worker& Worker ) noexcept;
        bool HandleAODDelayedTasks_Global( Worker& Worker ) noexcept;
        bool HandleTimerTasks_Global( Worker& Worker ) noexcept;

    private:
        std::synced_value<uint32_t>               bIsRunning          { FALSE };   //!< Is the group marked as active
        AsyncIO                                   AsyncIOAPI          {};          //!< Async IO interface
        const WorkerGroupTag                      Tag                 {};          //!< Worker group tag
        WorkerTickTask                            OnWorkerTick        {};          //!< Task to be executed each time a worker ticks
        WorkerTask                                OnWorkerStartTask   {};          //!< Task to be executed each time a worker starts
        WorkerTask                                OnWorkerStopTask    {};          //!< Task to be executed each time a worker stops
        std::vector<std::unique_ptr<TCPAcceptor>> TCPAcceptors        {};          //!< All TCP async acceptors in the group
        std::vector<std::unique_ptr<Worker>>      Workers             {};          //!< All workers registered in the group
        ServerInstance*                           Manager             { nullptr }; //!< Manager of this group
        std::synced_value<uint32_t>               RunningWorkers      { 0 };       //!< Count of active running workers
        std::synced_value<uint32_t>               TotalWorkers        { 0 };       //!< Count of total workers registered in the group
        Worker*                                   MasterWorker        { nullptr }; //!< Cached pointer to the master worker [if any]
        std::cacheline_unique_ptr<TLSSyncSystem>  MyTLSSyncSystem     { nullptr }; //!< Group level TLS sync system

        friend Worker;    
        friend ServerInstance;    
        friend struct AODObject;
        template<WorkerGroupTagFlags, bool> friend struct ActiveWorkerVariant;
        template<WorkerGroupTagFlags>       friend struct ReactiveWorkerVariant;
    }; 
}