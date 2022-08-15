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

        //! Set the functor to be executed when the worker(as master) is termianted [ void( ASD_CDECL *)( Worker&, WorkerGroup& ) noexcept ]
        template<typename TFunctor>
        SKL_FORCEINLINE void SetOnMasterTerminatedHandler( TFunctor OnMasterTermiantedHandler ) noexcept 
        {
            OnMasterTermianted += std::forward<TFunctor>( OnMasterTermiantedHandler );
        }

        //! Is this worker running
        bool GetIsRunning() const noexcept { return bIsRunning.load_relaxed(); }

        //! Is this a master worker
        bool IsMaster() const noexcept { return bIsMasterThread.load_relaxed(); }

        //! Start the worker
        RStatus Start() noexcept;   

        //! Signal the worker to stop
        void SignalStop() noexcept;

        //! Join worker thread
        void Join() noexcept
        {
            if( true == Thread.joinable() )
            {
                Thread.join();
            }
        }

        //! Get the time point at which the worker started 
        TEpochTimePoint GetStartedAt() const noexcept { StartedAt.load_relaxed(); }

    private:
        void RunImpl() noexcept;

        std::jthread                        Thread             {};          //!< Thread of this worker
        std::synced_value<uint32_t>         bIsRunning         { FALSE };   //!< Is this worker signaled to run
        std::synced_value<uint32_t>         bIsMasterThread    { FALSE };   //!< Is this a master worker
        std::relaxed_value<TEpochTimePoint> StartedAt          { 0 };       //!< Time point when the worker started
        RunTask                             OnRun              {};          //!< Task to run as main of the thread
        RunTask                             OnMasterTermianted {};          //!< Task to run when this worker termiantes (if master)
        WorkerGroup*                        Group              { nullptr }; //!< Owning group of this worker

        friend WorkerGroup;
        friend WorkerGroupManager;    
    };  
}