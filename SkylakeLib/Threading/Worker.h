//!
//! \file Worker.h
//! 
//! \brief Worker abstraction
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    class Worker
    {
    public:
        using RunTask = ASD::UniqueFunctorWrapper<32, void( ASD_CDECL *)( Worker&, WorkerGroup& ) noexcept>;

        Worker() noexcept = default;
        Worker( WorkerGroup* Group ) noexcept : Group{ Group } {}
        ~Worker() noexcept
        {
            SKL_ASSERT( false == GetIsRunning() );
            Clear();
        }

        //! Set the functor to be executed as the workers main [ void( ASD_CDECL *)( Worker&, WorkerGroup& ) noexcept ]
        template<typename TFunctor>
        SKL_FORCEINLINE void SetOnRunHandler( TFunctor OnRunHandler ) noexcept 
        {
            OnRun += std::forward<TFunctor>( OnRunHandler );
        }

        //! Is this worker running
        bool GetIsRunning() const noexcept { return TRUE == bIsRunning.load_relaxed(); }

        //! Is this a master worker
        bool IsMaster() const noexcept { return TRUE == bIsMasterThread.load_relaxed(); }

        //! Start the worker
        RStatus Start() noexcept;   

        //! Join worker thread
        void Join() noexcept
        {
            if( true == Thread.joinable() )
            {
                Thread.join();
            }

            Clear();
        }

        //! Get the time point at which the worker started 
        TEpochTimePoint GetStartedAt() const noexcept { return StartedAt.load_relaxed(); }
        
        //! Get the time duration this worker was active for
        TEpochTimeDuration GetAliveTime() const noexcept { return GetSystemUpTickCount() - GetStartedAt(); }

        //! Defer task execution on this worker
        SKL_FORCEINLINE void Defer( ITask* InTask ) noexcept
        {
            DelayedTasks.Push( InTask );
        }
        
        //! Defer general task execution on this worker
        SKL_FORCEINLINE void DeferGeneral( ITask* InTask ) noexcept
        {
            Tasks.Push( InTask );
        }

        //! Defer AOD task execution on this worker
        SKL_FORCEINLINE void Defer( IAODSharedObjectTask* InTask ) noexcept
        {
            AODSharedObjectDelayedTasks.Push( InTask );
        }

        //! Defer AOD task execution on this worker
        SKL_FORCEINLINE void Defer( IAODStaticObjectTask* InTask ) noexcept
        {
            AODStaticObjectDelayedTasks.Push( InTask );
        }
        
        //! Defer AOD task execution on this worker
        SKL_FORCEINLINE void Defer( IAODCustomObjectTask* InTask ) noexcept
        {
            AODCustomObjectDelayedTasks.Push( InTask );
        }

        //! Get the group owning this worker
        SKL_FORCEINLINE SKL_NODISCARD WorkerGroup* GetGroup() const noexcept { return Group; }

    private:
        void RunImpl() noexcept;
        void Clear() noexcept;
                
        TaskQueue                                             Tasks                       {};          //!< Single consumer multiple producers queue for general tasks 
        TaskQueue                                             DelayedTasks                {};          //!< Single consumer multiple producers queue for delayed tasks 
        AODTaskQueue                                          AODSharedObjectDelayedTasks {};          //!< Single consumer multiple producers queue for AOD delayed tasks 
        AODTaskQueue                                          AODStaticObjectDelayedTasks {};          //!< Single consumer multiple producers queue for AOD delayed tasks 
        AODTaskQueue                                          AODCustomObjectDelayedTasks {};          //!< Single consumer multiple producers queue for AOD delayed tasks 
        std::synced_value<uint32_t>                           bIsRunning                  { FALSE };   //!< Is this worker signaled to run
        std::synced_value<uint32_t>                           bIsMasterThread             { FALSE };   //!< Is this a master worker
        std::relaxed_value<TEpochTimePoint>                   StartedAt                   { 0 };       //!< Time point when the worker started
        RunTask                                               OnRun                       {};          //!< Task to run as main of the thread
        WorkerGroup*                                          Group                       { nullptr }; //!< Owning group of this worker
        std::jthread                                          Thread                      {};          //!< Thread of this worker
        std::relaxed_value<struct AODTLSContext*>             AODTLSContext               {};          //!< Cached AODTLSContext instance for this worker
        std::relaxed_value<struct ServerInstanceTLSContext*>  ServerInstanceTLSContext    {};          //!< Cached ServerInstanceTLSContext instance for this worker

        friend WorkerGroup;
        friend class ServerInstance;    
        template<WorkerGroupTagFlags, bool> friend struct ActiveWorkerVariant;
        template<WorkerGroupTagFlags>       friend struct ReactiveWorkerVariant;
    };  
}