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
    struct WorkerGroupTag
    {
        uint32_t       TickRate         { 0 };       //!< Tick rate of worker [ bIsActive == true ]
        uint32_t       SyncTLSTickRate  { 0 };       //!< Tick rate of tls sync [ bSupportsTLSSync == true ]
        uint16_t       Id               { 0 };       //!< UID of the tag [max 65536 workers]
        uint16_t       WorkersCount     { 0 };       //!< Number of workers in the group
        bool           bIsActive        { false };   //!< Is this an pro-active worker [ it has an active ticks/second loop ] 
        bool           bHandlesTasks    { false };   //!< Does this group handle tasks and async IO tasks [ an AsyncIO instance will be created for the group if true ]
        bool           bSupportsTLSSync { false };   //!< Supports TLSSync [ TLSSync it is its own feature, please get documented before use ]
        const wchar_t *Name             { nullptr }; //!< Name of the worker group

        SKL_FORCEINLINE bool IsValid() const noexcept { return 0 != Id; }
    }; 

    class WorkerGroup
    {
    public:
        using WorkerTask = ASD::CopyFunctorWrapper<32, void( SKL_CDECL* )( Worker&, WorkerGroup& ) noexcept>;

        WorkerGroup( const WorkerGroupTag& Tag, WorkerGroupManager* Manager ) noexcept 
            : Tag{ Tag }
            , Manager{ Manager } 
        {
            SKL_ASSERT( true == Tag.IsValid() );
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
        SKL_FORCEINLINE void SetWorkerTickHandler( const WorkerTask& InOnWorkerTick ) noexcept 
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

        //! Join all worker in the group except the master
        void Join() noexcept;

        //! Is the worker group valid
        bool IsValid() const noexcept { return Tag.IsValid(); }

        //! Is the worker group running
        bool IsRunning() const noexcept { return bIsRunning.load_relaxed(); }

        //! Get the number of running workers
        uint32_t GetNumberOfRunningWorkers() const noexcept { return RunningWorkers.load_relaxed(); }

        //! Get the total number of running workers
        uint32_t GetTotalNumberOfRunningWorkers() const noexcept { return TotalWorkers.load_relaxed(); }

        //! Get the tag of the group
        const WorkerGroupTag& GetTag() const noexcept { return Tag; }

        //! Get the master worker [if any]
        std::shared_ptr<Worker> GetTheMasterWorker() noexcept { return MasterWorker; }

        //! Is this group the master workers group 
        bool IsMasterWorkerGroup() const noexcept { return nullptr != MasterWorker.get(); };

        //! Deferre functor exectuion to the a worker in this group [if the group handles async IO only!]
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
        
    private:
        RStatus CreateWorkers( bool bIncludeMaster ) noexcept;

        RStatus HandleSlaveWorker( Worker& Worker ) noexcept;

        RStatus HandleMasterWorker( std::shared_ptr<Worker> MasterWorker ) noexcept;

        void OnWorkerStarted( Worker& Worker ) noexcept;

        void OnWorkerStopped( Worker& Worker ) noexcept;

        void OnAllWorkersStarted() noexcept;

        void OnAllWorkersStopped() noexcept;

        bool HandleTasks_Proactive( uint32_t MillisecondsToSleep ) noexcept;

        bool HandleTasks_Reactive() noexcept;
        
        bool HandleAsyncIOTask( AsyncIOOpaqueType* InOpaque, uint32_t NumberOfBytesTransferred ) noexcept;

        bool HandleTask( TCompletionKey InCompletionKey ) noexcept;

        bool HandleTLSSync( Worker& Worker ) noexcept;

        bool StartAllTCPAcceptors() noexcept;

        void StopAllTCPAcceptors() noexcept;

    private:
        const WorkerGroupTag                      Tag                 {};          //!< Worker group tag
        std::synced_value<uint32_t>               RunningWorkers      { 0 };       //!< Count of active running workers
        std::synced_value<uint32_t>               TotalWorkers        { 0 };       //!< Count of total workers registered in the group
        std::synced_value<uint32_t>               bIsRunning          { FALSE };   //!< Is the group marked as active
        AsyncIO                                   AsyncIOAPI          {};          //!< Async IO interface
        std::vector<std::shared_ptr<Worker>>      Workers             {};          //!< All workers registered in the group
        std::vector<std::unique_ptr<TCPAcceptor>> TCPAcceptors        {};          //!< All tcp async acceptors in the group
        WorkerGroupManager*                       Manager             { nullptr }; //!< Manager of this group
        WorkerTask                                OnWorkerTick        {};          //!< Task to be executed each time a worker ticks
        WorkerTask                                OnWorkerStartTask   {};          //!< Task to be executed each time a worker starts
        WorkerTask                                OnWorkerStopTask    {};          //!< Task to be executed each time a worker stoppes
        std::shared_ptr<Worker>                   MasterWorker        {};          //!< Cached pointer to the master worker [if any]

        friend Worker;    
        friend WorkerGroupManager;    
    }; 
}