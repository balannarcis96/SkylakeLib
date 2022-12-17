//!
//! \file WorkerGroupRunVariants.h
//! 
//! \brief Compiletime select worker tick variant
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//!

namespace SKL
{
    template<bool bIsActive, bool bHandlesTasks, bool bSupportsAOD, bool bHandlesTimerTasks, bool bSupportsTLSSync, bool bHasTickHandler, bool bAllWorkerGroupsAreActive, bool bTickWorkerServices>
    struct WorkerGroupRunVariant;

    //! Active (Proactive)
    template<bool bHandlesTasks, bool bSupportsAOD, bool bHandlesTimerTasks, bool bSupportsTLSSync, bool bHasTickHandler, bool bAllWorkerGroupsAreActive, bool bTickWorkerServices>
    struct WorkerGroupRunVariant<true, bHandlesTasks, bSupportsAOD, bHandlesTimerTasks, bSupportsTLSSync, bHasTickHandler, bAllWorkerGroupsAreActive, bTickWorkerServices>
    {
        SKL_NOINLINE static void Run( Worker& InWorker, WorkerGroup& InGroup ) noexcept
        {
            SKLL_TRACE();

            const auto Tag                 { InGroup.GetTag() }; //!< Stack tag copy
            const auto TickRate            { true == Tag.bSupportsTLSSync ? std::max( Tag.TickRate, Tag.SyncTLSTickRate ) : Tag.TickRate };
            const auto MillisecondsToSleep { static_cast<uint32_t>( 1000.0 / static_cast<double>( TickRate ) ) };
            const auto SecondsToSleep      { 1.0 / static_cast<double>( TickRate ) };
            auto&      OnWorkerTick        { InGroup.OnWorkerTick };
            auto*      MyTLSSyncSystem     { InGroup.GetServerInstance()->GetTSLSyncSystemPtr() };
            auto&      WorkerServices      { InGroup.GetServerInstance()->GetAllWorkerServices() };
            
            PreciseSleep_WaitableTimer::Create();

            if constexpr( true == bSupportsTLSSync )
            {
                SKL_ASSERT( nullptr != MyTLSSyncSystem );
                MyTLSSyncSystem->TLSInitialize( InWorker, InGroup );
            }
            
            InWorker.AODTLSContext.exchange( SKL::AODTLSContext::GetInstance() );
            InWorker.ServerInstanceTLSContext.exchange( SKL::ServerInstanceTLSContext::GetInstance() );

            while( InGroup.IsRunning() ) SKL_LIKELY
            {
                if constexpr( true == bHandlesTasks )
                {
                    const bool bShouldTermiante{ InGroup.HandleTasks_Proactive( MillisecondsToSleep ) };
                    if ( true == bShouldTermiante ) SKL_UNLIKELY
                    {
                        break;
                    }
                }

                if constexpr( true == bHandlesTimerTasks )
                {
                    if constexpr( true == bAllWorkerGroupsAreActive )
                    {
                        const bool bShouldTermiante{ InGroup.HandleTimerTasks_Local( InWorker ) };
                        if ( true == bShouldTermiante ) SKL_UNLIKELY
                        {
                            break;
                        }
                    }
                    else
                    {
                        const bool bShouldTermiante{ InGroup.HandleTimerTasks_Global( InWorker ) };
                        if ( true == bShouldTermiante ) SKL_UNLIKELY
                        {
                            break;
                        }
                    }
                }

                if constexpr( true == bSupportsAOD )
                {
                    if constexpr( true == bAllWorkerGroupsAreActive )
                    {
                        const bool bShouldTermiante{ InGroup.HandleAODDelayedTasks_Local( InWorker ) };
                        if ( true == bShouldTermiante ) SKL_UNLIKELY
                        {
                            break;
                        }
                    }
                    else
                    {
                        const bool bShouldTermiante{ InGroup.HandleAODDelayedTasks_Global( InWorker ) };
                        if ( true == bShouldTermiante ) SKL_UNLIKELY
                        {
                            break;
                        }
                    }
                }

                if constexpr( true == bTickWorkerServices )
                {
                    for( size_t i = 1; i < WorkerServices.size(); ++i )
                    {
                        WorkerServices[ i ]->OnTickWorker( InWorker, InGroup );
                    }
                }

                if constexpr( true == bSupportsTLSSync )
                {
                    MyTLSSyncSystem->TLSTick( InWorker, InGroup );
                }

                if constexpr( true == bHasTickHandler )
                {
                    OnWorkerTick.Dispatch( InWorker, InGroup );
                }

                if constexpr( false == bHandlesTasks )
                {
                    PreciseSleep( SecondsToSleep );
                }
            }
            
            if constexpr( true == bSupportsTLSSync )
            {
                MyTLSSyncSystem->TLSShutdown();
            }

            PreciseSleep_WaitableTimer::Destroy();
        }
    };

    //! Reactive
    template<bool bSupportsTLSSync>
    struct WorkerGroupRunVariant<false, true, false, false, bSupportsTLSSync, false, false, false>
    {
        SKL_NOINLINE static void Run( Worker& InWorker, WorkerGroup& InGroup ) noexcept
        {
            SKLL_TRACE();

            const auto Tag                 { InGroup.GetTag() }; //!< Stack tag copy
            const auto TickRate            { Tag.SyncTLSTickRate };
            const auto MillisecondsToSleep { static_cast<uint32_t>( 1000.0 / static_cast<double>( TickRate ) ) };
            auto*      MyTLSSyncSystem     { InGroup.GetServerInstance()->GetTSLSyncSystemPtr() };
            
            if constexpr( true == bSupportsTLSSync )
            {
                SKL_ASSERT( nullptr != MyTLSSyncSystem );
                MyTLSSyncSystem->TLSInitialize( InWorker, InGroup );
            }
            
            InWorker.AODTLSContext.exchange( SKL::AODTLSContext::GetInstance() );
            InWorker.ServerInstanceTLSContext.exchange( SKL::ServerInstanceTLSContext::GetInstance() );

            while( InGroup.IsRunning() ) SKL_LIKELY
            {
                if constexpr( true == bSupportsTLSSync )
                {
                    const bool bShouldTermiante{ InGroup.HandleTasks_Proactive( MillisecondsToSleep ) };
                    if ( true == bShouldTermiante ) SKL_UNLIKELY
                    {
                        break;
                    }
                }
                else
                {
                    const bool bShouldTermiante{ InGroup.HandleTasks_Reactive() };
                    if ( true == bShouldTermiante ) SKL_UNLIKELY
                    {
                        break;
                    }
                }

                if constexpr( true == bSupportsTLSSync )
                {
                    MyTLSSyncSystem->TLSTick( InWorker, InGroup );
                }
            }
            
            if constexpr( true == bSupportsTLSSync )
            {
                MyTLSSyncSystem->TLSShutdown();
            }
        }
    };

    //! Active (Proactive)
    template<bool bHandlesTasks, bool bSupportsAOD, bool bHandlesTimerTasks, bool bSupportsTLSSync, bool bHasTickHandler, bool bAllWorkerGroupsAreActive, bool bTickWorkerServices>
    using ActiveWorkerVariant = WorkerGroupRunVariant<true, bHandlesTasks, bSupportsAOD, bHandlesTimerTasks, bSupportsTLSSync, bHasTickHandler, bAllWorkerGroupsAreActive, bTickWorkerServices>;

    //! Reactive
    template<bool bSupportsTLSSync>
    using ReactiveWorkerVariant = WorkerGroupRunVariant<false, true, false, false, bSupportsTLSSync, false, false, false>;

#define SKL_WORKER_ALL_ACTIVE_RUN_VARTIAN( bHandlesTasksVal, bSupportsAODVal, bHandlesTimerTasksVal, bSupportsTLSSyncVal, bCallTickHandlerVal, bTickWorkerServicesVal )                         \
             else if( bHandlesTasksVal == WorkerGroupTag.bHandlesTasks                                                                                                                          \
             && bSupportsAODVal == WorkerGroupTag.bSupportsAOD                                                                                                                                  \
             && bHandlesTimerTasksVal == WorkerGroupTag.bHandlesTimerTasks                                                                                                                      \
             && bSupportsTLSSyncVal == WorkerGroupTag.bSupportsTLSSync                                                                                                                          \
             && bCallTickHandlerVal == WorkerGroupTag.bCallTickHandler                                                                                                                          \
             && bTickWorkerServicesVal == WorkerGroupTag.bTickWorkerServices )                                                                                                                  \
            {                                                                                                                                                                                   \
                ActiveWorkerVariant<bHandlesTasksVal, bSupportsAODVal, bHandlesTimerTasksVal, bSupportsTLSSyncVal, bCallTickHandlerVal, true, bTickWorkerServicesVal>::Run( Worker, *this );    \
            }   

 #define SKL_WORKER_ACTIVE_RUN_VARTIAN( bHandlesTasksVal, bSupportsAODVal, bHandlesTimerTasksVal, bSupportsTLSSyncVal, bCallTickHandlerVal, bTickWorkerServicesVal )                            \
             else if( bHandlesTasksVal == WorkerGroupTag.bHandlesTasks                                                                                                                          \
             && bSupportsAODVal == WorkerGroupTag.bSupportsAOD                                                                                                                                  \
             && bHandlesTimerTasksVal == WorkerGroupTag.bHandlesTimerTasks                                                                                                                      \
             && bSupportsTLSSyncVal == WorkerGroupTag.bSupportsTLSSync                                                                                                                          \
             && bCallTickHandlerVal == WorkerGroupTag.bCallTickHandler                                                                                                                          \
             && bTickWorkerServicesVal == WorkerGroupTag.bTickWorkerServices )                                                                                                                  \
            {                                                                                                                                                                                   \
                ActiveWorkerVariant<bHandlesTasksVal, bSupportsAODVal, bHandlesTimerTasksVal, bSupportsTLSSyncVal, bCallTickHandlerVal, false, bTickWorkerServicesVal>::Run( Worker, *this );   \
            }
}

