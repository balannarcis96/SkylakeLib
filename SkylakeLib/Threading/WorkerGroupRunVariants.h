//!
//! \file WorkerGroupRunVariants.h
//! 
//! \brief Compiletime select worker tick variant
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//!

namespace SKL
{
    template<bool bIsActive, bool bHandlesTasks, bool bSupportsAOD, bool bHandlesTimerTasks, bool bSupportsTLSSync, bool bHasTickHandler, bool bAllWorkerGroupsAreActive, bool bTickWorkerServices, bool bSupportsWGTLSSync>
    struct WorkerGroupRunVariant;

    //! Active (Proactive)
    template<bool bHandlesTasks, bool bSupportsAOD, bool bHandlesTimerTasks, bool bSupportsTLSSync, bool bHasTickHandler, bool bAllWorkerGroupsAreActive, bool bTickWorkerServices, bool bSupportsWGTLSSync>
    struct WorkerGroupRunVariant<true, bHandlesTasks, bSupportsAOD, bHandlesTimerTasks, bSupportsTLSSync, bHasTickHandler, bAllWorkerGroupsAreActive, bTickWorkerServices, bSupportsWGTLSSync>
    {
        SKL_NOINLINE static void Run( Worker& InWorker, WorkerGroup& InGroup ) noexcept
        {
            SKLL_TRACE();

            const auto Tag                 = InGroup.GetTag(); //!< Stack tag copy
            const auto TickRate            = ( bSupportsTLSSync || bSupportsWGTLSSync ) ? std::max( Tag.TickRate, Tag.SyncTLSTickRate ) : Tag.TickRate;
            const auto MillisecondsToSleep = static_cast<uint32_t>( 1000.0 / static_cast<double>( TickRate ) );
            const auto SecondsToSleep      = 1.0 / static_cast<double>( TickRate );
            auto&      OnWorkerTick        = InGroup.OnWorkerTick;
            auto*      ServerTLSSyncSystem = InGroup.GetServerInstance()->GetTSLSyncSystemPtr();
            auto*      MyTLSSyncSystem     = InGroup.MyTLSSyncSystem.get();
            auto&      WorkerServices      = InGroup.GetServerInstance()->GetAllWorkerServices();
            
            PreciseSleep_WaitableTimer::Create();

            if constexpr( true == bSupportsTLSSync )
            {
                SKL_ASSERT( nullptr != ServerTLSSyncSystem );
                ServerTLSSyncSystem->TLSInitialize( InWorker, InGroup );
            }
            
            if constexpr( true == bSupportsWGTLSSync )
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
                    ServerTLSSyncSystem->TLSTick( InWorker, InGroup );
                }
                
                if constexpr( true == bSupportsWGTLSSync )
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
                ServerTLSSyncSystem->TLSShutdown();
            }
            
            if constexpr( true == bSupportsWGTLSSync )
            {
                MyTLSSyncSystem->TLSShutdown();
            }

            PreciseSleep_WaitableTimer::Destroy();
        }
    };

    //! Reactive
    template<bool bSupportsTLSSync, bool bSupportsWGTLSSync>
    struct WorkerGroupRunVariant<false, true, false, false, bSupportsTLSSync, false, false, false, bSupportsWGTLSSync>
    {
        SKL_NOINLINE static void Run( Worker& InWorker, WorkerGroup& InGroup ) noexcept
        {
            SKLL_TRACE();

            const auto Tag                 = InGroup.GetTag(); //!< Stack tag copy
            const auto TickRate            = Tag.SyncTLSTickRate;
            const auto MillisecondsToSleep = static_cast<uint32_t>( 1000.0 / static_cast<double>( TickRate ) );
            auto*      ServerTLSSyncSystem = InGroup.GetServerInstance()->GetTSLSyncSystemPtr();
            auto*      MyTLSSyncSystem     = InGroup.MyTLSSyncSystem.get();
            
            if constexpr( true == bSupportsTLSSync )
            {
                SKL_ASSERT( nullptr != ServerTLSSyncSystem );
                ServerTLSSyncSystem->TLSInitialize( InWorker, InGroup );
            }
            
            if constexpr( true == bSupportsWGTLSSync )
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
                    ServerTLSSyncSystem->TLSTick( InWorker, InGroup );
                }

                if constexpr( true == bSupportsWGTLSSync )
                {
                    MyTLSSyncSystem->TLSTick( InWorker, InGroup );
                }
            }
            
            if constexpr( true == bSupportsTLSSync )
            {
                ServerTLSSyncSystem->TLSShutdown();
            }
            
            if constexpr( true == bSupportsWGTLSSync )
            {
                MyTLSSyncSystem->TLSShutdown();
            }
        }
    };

    //! Active (Proactive)
    template<bool bHandlesTasks, bool bSupportsAOD, bool bHandlesTimerTasks, bool bSupportsTLSSync, bool bHasTickHandler, bool bAllWorkerGroupsAreActive, bool bTickWorkerServices, bool bSupportsWGTLSSyncVal>
    using ActiveWorkerVariant = WorkerGroupRunVariant<true, bHandlesTasks, bSupportsAOD, bHandlesTimerTasks, bSupportsTLSSync, bHasTickHandler, bAllWorkerGroupsAreActive, bTickWorkerServices, bSupportsWGTLSSyncVal>;

    //! Reactive
    template<bool bSupportsTLSSync, bool bSupportsWGTLSSync>
    using ReactiveWorkerVariant = WorkerGroupRunVariant<false, true, false, false, bSupportsTLSSync, false, false, false, bSupportsWGTLSSync>;

#define SKL_WORKER_ALL_ACTIVE_RUN_VARTIAN( bHandlesTasksVal, bSupportsAODVal, bHandlesTimerTasksVal, bSupportsTLSSyncVal, bCallTickHandlerVal, bTickWorkerServicesVal, bSupportsWGTLSSyncVal )  \
             else if( bHandlesTasksVal == WorkerGroupTag.bHandlesTasks                                                                                                                          \
             && bSupportsAODVal == WorkerGroupTag.bSupportsAOD                                                                                                                                  \
             && bHandlesTimerTasksVal == WorkerGroupTag.bHandlesTimerTasks                                                                                                                      \
             && bSupportsTLSSyncVal == WorkerGroupTag.bSupportsTLSSync                                                                                                                          \
             && bCallTickHandlerVal == WorkerGroupTag.bCallTickHandler                                                                                                                          \
             && bTickWorkerServicesVal == WorkerGroupTag.bTickWorkerServices                                                                                                                    \
             && bSupportsWGTLSSyncVal == WorkerGroupTag.bHasWorkerGroupSpecificTLSSync )                                                                                                        \
            {                                                                                                                                                                                   \
                ActiveWorkerVariant<bHandlesTasksVal, bSupportsAODVal, bHandlesTimerTasksVal, bSupportsTLSSyncVal, bCallTickHandlerVal, true, bTickWorkerServicesVal, bSupportsWGTLSSyncVal>::Run( Worker, *this ); \
            }   

 #define SKL_WORKER_ACTIVE_RUN_VARTIAN( bHandlesTasksVal, bSupportsAODVal, bHandlesTimerTasksVal, bSupportsTLSSyncVal, bCallTickHandlerVal, bTickWorkerServicesVal, bSupportsWGTLSSyncVal )     \
             else if( bHandlesTasksVal == WorkerGroupTag.bHandlesTasks                                                                                                                          \
             && bSupportsAODVal == WorkerGroupTag.bSupportsAOD                                                                                                                                  \
             && bHandlesTimerTasksVal == WorkerGroupTag.bHandlesTimerTasks                                                                                                                      \
             && bSupportsTLSSyncVal == WorkerGroupTag.bSupportsTLSSync                                                                                                                          \
             && bCallTickHandlerVal == WorkerGroupTag.bCallTickHandler                                                                                                                          \
             && bTickWorkerServicesVal == WorkerGroupTag.bTickWorkerServices                                                                                                                    \
             && bSupportsWGTLSSyncVal == WorkerGroupTag.bHasWorkerGroupSpecificTLSSync )                                                                                                        \
            {                                                                                                                                                                                   \
                ActiveWorkerVariant<bHandlesTasksVal, bSupportsAODVal, bHandlesTimerTasksVal, bSupportsTLSSyncVal, bCallTickHandlerVal, false, bTickWorkerServicesVal, bSupportsWGTLSSyncVal>::Run( Worker, *this ); \
            }
}

