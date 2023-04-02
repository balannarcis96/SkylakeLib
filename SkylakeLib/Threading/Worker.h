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

        Worker() noexcept;
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
        SKL_FORCEINLINE SKL_NODISCARD bool GetIsRunning() const noexcept { return TRUE == bIsRunning.load_relaxed(); }

        //! Is this a master worker
        SKL_FORCEINLINE SKL_NODISCARD bool IsMaster() const noexcept { return TRUE == bIsMasterThread.load_relaxed(); }

        //! Start the worker
        SKL_NODISCARD RStatus Start() noexcept;   

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
        SKL_FORCEINLINE SKL_NODISCARD TEpochTimePoint GetStartedAt() const noexcept { return StartedAt.load_relaxed(); }
        
        //! Get the time duration this worker was active for
        SKL_FORCEINLINE SKL_NODISCARD TEpochTimeDuration GetAliveTime() const noexcept { return GetSystemUpTickCount() - GetStartedAt(); }

        //! Defer task execution on this worker
        SKL_FORCEINLINE void Defer( ITask* InTask ) noexcept
        {
            DelayedTasks.Push( InTask );

            #if defined(SKL_KPI_QUEUE_SIZES)
            KPIContext::Increment_DelayedTasksQueueSize( GetIndex() );
            #endif
        }
        
        //! Defer general task execution on this worker
        SKL_FORCEINLINE void DeferGeneral( ITask* InTask ) noexcept
        {
            Tasks.Push( InTask );
            
            #if defined(SKL_KPI_QUEUE_SIZES)
            KPIContext::Increment_TasksQueueSize( GetIndex() );
            #endif
        }

        //! Defer AOD task execution on this worker
        SKL_FORCEINLINE void Defer( IAODSharedObjectTask* InTask ) noexcept
        {
            AODSharedObjectDelayedTasks.Push( InTask );
            
            #if defined(SKL_KPI_QUEUE_SIZES)
            KPIContext::Increment_AODSharedObjectDelayedTasksQueueSize( GetIndex() );
            #endif
        }

        //! Defer AOD task execution on this worker
        SKL_FORCEINLINE void Defer( IAODStaticObjectTask* InTask ) noexcept
        {
            AODStaticObjectDelayedTasks.Push( InTask );
            
            #if defined(SKL_KPI_QUEUE_SIZES)
            KPIContext::Increment_AODStaticObjectDelayedTasksQueueSize( GetIndex() );
            #endif
        }
        
        //! Defer AOD task execution on this worker
        SKL_FORCEINLINE void Defer( IAODCustomObjectTask* InTask ) noexcept
        {
            AODCustomObjectDelayedTasks.Push( InTask );
            
            #if defined(SKL_KPI_QUEUE_SIZES)
            KPIContext::Increment_AODCustomObjectDelayedTasksQueueSize( GetIndex() );
            #endif
        }

        //! Get the group owning this worker
        SKL_FORCEINLINE SKL_NODISCARD WorkerGroup* GetGroup() const noexcept { return Group; }
    
        //! Get the global unique index of this worker
        SKL_FORCEINLINE SKL_NODISCARD int32_t GetIndex() const noexcept { return WorkerIndex; }

    #if defined(SKL_KPI_WORKER_TICK)
        //! Get the average tick time in seconds of this worker
        //! \remarks Valid value only if this is an active worker
        SKL_NODISCARD double GetAverateTickTimeUsafe() const noexcept { return TickAverageTime.GetValue(); }
    protected:
        SKL_NODISCARD void SetAverateTickTimeUsafe( double Value ) noexcept { TickAverageTime.SetValue( Value ); }
    #endif

    private:
        void RunImpl() noexcept;
        void Clear() noexcept;
                
        const int32_t                                         WorkerIndex                {};          //!< Globally unique worker index
        SKL_CACHE_ALIGNED TaskQueue                           Tasks                      {};          //!< Single consumer multiple producers queue for general tasks 
        SKL_CACHE_ALIGNED TaskQueue                           DelayedTasks               {};          //!< Single consumer multiple producers queue for delayed tasks 
        SKL_CACHE_ALIGNED AODTaskQueue                        AODSharedObjectDelayedTasks{};          //!< Single consumer multiple producers queue for AOD delayed tasks 
        SKL_CACHE_ALIGNED AODTaskQueue                        AODStaticObjectDelayedTasks{};          //!< Single consumer multiple producers queue for AOD delayed tasks 
        SKL_CACHE_ALIGNED AODTaskQueue                        AODCustomObjectDelayedTasks{};          //!< Single consumer multiple producers queue for AOD delayed tasks 
        SKL_CACHE_ALIGNED std::synced_value<uint32_t>         bIsRunning                 { FALSE };   //!< Is this worker signaled to run
        std::synced_value<uint32_t>                           bIsMasterThread            { FALSE };   //!< Is this a master worker
        std::relaxed_value<TEpochTimePoint>                   StartedAt                  { 0U };      //!< Time point when the worker started
        RunTask                                               OnRun                      {};          //!< Task to run as main of the thread
        WorkerGroup*                                          Group                      { nullptr }; //!< Owning group of this worker
        std::jthread                                          Thread                     {};          //!< Thread of this worker
        std::relaxed_value<struct AODTLSContext*>             AODTLSContext              {};          //!< Cached AODTLSContext instance for this worker
        std::relaxed_value<struct ServerInstanceTLSContext*>  ServerInstanceTLSContext   {};          //!< Cached ServerInstanceTLSContext instance for this worker

        #if defined(SKL_KPI_WORKER_TICK)
        SKL_CACHE_ALIGNED KPIValueAveragePoint<false> TickAverageTime{}; // Average tick time KPI
        #endif

        friend WorkerGroup;
        friend class ServerInstance;    
        template<WorkerGroupTagFlags, bool> friend struct ActiveWorkerVariant;
        template<WorkerGroupTagFlags>       friend struct ReactiveWorkerVariant;
    };  
}