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
            SKL_ASSERT_ALLWAYS( 0 == GetNumberOfRunningWorkers() );
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
        RStatus Build( bool bIncludeMaster ) noexcept;

        //! Attempt to start the worker group
        RStatus Start() noexcept;

        //! Stop and join all workers in group except the master
        void Stop() noexcept;

        //! Signal all active workers to stop
        void SignalToStop() noexcept;

        //! Join all worker in the group except the master(if present)
        void Join() noexcept;

        //! Is the worker group valid
        bool IsValid() const noexcept { return Tag.IsValid(); }

        //! Is the worker group running
        bool IsRunning() const noexcept { return bIsRunning.load_relaxed(); }

        //! Get the number of running workers
        uint32_t GetNumberOfRunningWorkers() const noexcept { return RunningWorkers.load_relaxed(); }

        //! Get the total number of workers
        uint32_t GetTotalNumberOfWorkers() const noexcept { return TotalWorkers.load_relaxed(); }

        //! Get the tag of the group
        const WorkerGroupTag& GetTag() const noexcept { return Tag; }

        //! Get the master worker [if any]
        std::shared_ptr<Worker> GetTheMasterWorker() noexcept { return MasterWorker; }

        //! Is this group the master workers group 
        bool IsMasterWorkerGroup() const noexcept { return nullptr != MasterWorker.get(); };

        //! Deferre functor exectuion to the a worker in this group [if the group bHandlesTasks=true only!] [void( void ) noexcept]
        template<typename TFunctor>
        RStatus Deferre( TFunctor&& InFunctor ) noexcept
        {
            auto* NewTask { MakeTaskRaw( std::forward<TFunctor>( InFunctor ) ) };
            return AsyncIOAPI.QueueAsyncWork( reinterpret_cast<TCompletionKey>( NewTask ) );
        }
    
        //! Create new tcp async acceptor on this instance
        RStatus AddNewTCPAcceptor( const TCPAcceptorConfig& Config ) noexcept;

        //! Query tcp async acceptor by id
        TCPAcceptor* GetTCPAcceptorById( uint32_t Id ) noexcept
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
        TCPAcceptor* GetTCPAcceptor( uint32_t Ip, uint16_t Port ) noexcept
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
        SKL_FORCEINLINE TCPAcceptor* GetTCPAcceptorIdAsIndex( uint32_t Id ) noexcept
        {
            SKL_ASSERT( TCPAcceptors.size() > Id );
            return TCPAcceptors[ Id ].get();
        }

        //! DO NOT CALL
        void ProactiveWorkerRun( Worker& Worker ) noexcept;

        //! DO NOT CALL
        void ReactiveWorkerRun( Worker& Worker ) noexcept;
        
        //! Get the server instance
        ServerInstance* GetServerInstance() noexcept { return Manager; }
        
        //! Get the server instance
        const ServerInstance* GetServerInstance() const noexcept { return Manager; }

        //! Get workers 
        std::span<std::shared_ptr<Worker>> GetWorkers() noexcept
        {
            return { Workers };
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
        RStatus HandleMasterWorker( std::shared_ptr<Worker> MasterWorker ) noexcept;
        bool HandleTasks_Proactive( uint32_t MillisecondsToSleep ) noexcept;
        bool HandleTasks_Reactive() noexcept;
        bool HandleAsyncIOTask( AsyncIOOpaqueType* InOpaque, uint32_t NumberOfBytesTransferred ) noexcept;
        bool HandleTask( TCompletionKey InCompletionKey ) noexcept;
        bool HandleTLSSync( Worker& Worker ) noexcept;
        bool HandleAODDelayedTasks_Local( Worker& Worker ) noexcept;
        bool HandleTimerTasks_Local( Worker& Worker ) noexcept;
        bool HandleAODDelayedTasks_Global( Worker& Worker ) noexcept;
        bool HandleTimerTasks_Global( Worker& Worker ) noexcept;

    private:
        const WorkerGroupTag                      Tag                 {};          //!< Worker group tag
        AsyncIO                                   AsyncIOAPI          {};          //!< Async IO interface
        WorkerTickTask                            OnWorkerTick        {};          //!< Task to be executed each time a worker ticks
        WorkerTask                                OnWorkerStartTask   {};          //!< Task to be executed each time a worker starts
        WorkerTask                                OnWorkerStopTask    {};          //!< Task to be executed each time a worker stoppes
        std::vector<std::unique_ptr<TCPAcceptor>> TCPAcceptors        {};          //!< All tcp async acceptors in the group
        std::vector<std::shared_ptr<Worker>>      Workers             {};          //!< All workers registered in the group
        ServerInstance*                           Manager             { nullptr }; //!< Manager of this group
        std::synced_value<uint32_t>               RunningWorkers      { 0 };       //!< Count of active running workers
        std::synced_value<uint32_t>               TotalWorkers        { 0 };       //!< Count of total workers registered in the group
        std::synced_value<uint32_t>               bIsRunning          { FALSE };   //!< Is the group marked as active
        std::shared_ptr<Worker>                   MasterWorker        {};          //!< Cached pointer to the master worker [if any]

        friend Worker;    
        friend ServerInstance;    
        friend struct AODObject;
        template<bool bIsActive, bool bHandlesTasks, bool bSupportsAOD, bool bHandlesTimerTasks, bool bSupportsTLSSync, bool bHasTickHandler, bool bAllWorkerGroupsAreActive>
        friend struct WorkerGroupRunVariant;
    }; 
}