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
        }

        //! Set the functor to be executed as the workers main [ void( ASD_CDECL *)( Worker&, WorkerGroup& ) noexcept ]
        template<typename TFunctor>
        SKL_FORCEINLINE void SetOnRunHandler( TFunctor OnRunHandler ) noexcept 
        {
            OnRun += std::forward<TFunctor>( OnRunHandler );
        }

        //! Is this worker running
        bool GetIsRunning() const noexcept { return bIsRunning.load_relaxed(); }

        //! Is this a master worker
        bool IsMaster() const noexcept { return bIsMasterThread.load_relaxed(); }

        //! Start the worker
        RStatus Start() noexcept;   

        //! Join worker thread
        void Join() noexcept
        {
            if( true == Thread.joinable() )
            {
                Thread.join();
            }
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

    private:
        void RunImpl() noexcept;
                
        TaskQueue                           DelayedTasks                {};          //!< Single consummer multiple producers queue for delayed tasks 
        AODTaskQueue                        AODSharedObjectDelayedTasks {};          //!< Single consummer multiple producers queue for AOD delayed tasks 
        AODTaskQueue                        AODStaticObjectDelayedTasks {};          //!< Single consummer multiple producers queue for AOD delayed tasks 
        std::synced_value<uint32_t>         bIsRunning                  { FALSE };   //!< Is this worker signaled to run
        std::synced_value<uint32_t>         bIsMasterThread             { FALSE };   //!< Is this a master worker
        std::relaxed_value<TEpochTimePoint> StartedAt                   { 0 };       //!< Time point when the worker started
        RunTask                             OnRun                       {};          //!< Task to run as main of the thread
        WorkerGroup*                        Group                       { nullptr }; //!< Owning group of this worker
        std::jthread                        Thread                      {};          //!< Thread of this worker

        friend WorkerGroup;
        friend class ServerInstance;    
    };  
}