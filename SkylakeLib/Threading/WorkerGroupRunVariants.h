//!
//! \file WorkerGroupRunVariants.h
//! 
//! \brief Compiletime select worker tick variant
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//!

namespace SKL
{
    //! Active (Proactive)
    template<WorkerGroupTagFlags Flags, bool bAllWorkerGroupsAreActive>
    struct ActiveWorkerVariant
    {
        static_assert( Flags.bIsActive );

        SKL_NOINLINE static void Run( Worker& InWorker, WorkerGroup& InGroup ) noexcept
        {
            SKLL_TRACE();

            const auto Tag                 = InGroup.GetTag(); //!< Stack tag copy
            const auto TickRate            = ( Flags.bSupportsTLSSync || Flags.bHasWorkerGroupSpecificTLSSync ) ? std::max( Tag.TickRate, Tag.SyncTLSTickRate ) : Tag.TickRate;
            const auto MillisecondsToSleep = static_cast<uint32_t>( 1000.0 / static_cast<double>( TickRate ) );
            auto&      OnWorkerTick        = InGroup.OnWorkerTick;
            auto*      ServerTLSSyncSystem = InGroup.GetServerInstance()->GetTSLSyncSystemPtr();
            auto*      MyTLSSyncSystem     = InGroup.MyTLSSyncSystem.get();
            auto&      WorkerServices      = InGroup.GetServerInstance()->GetAllWorkerServices();
            
#if defined(SKL_USE_PRECISE_SLEEP)
            const auto SecondsToSleep = 1.0 / static_cast<double>( TickRate );
            PreciseSleep_WaitableTimer::Create();
#endif

            if constexpr( Flags.bSupportsTLSSync )
            {
                SKL_ASSERT( nullptr != ServerTLSSyncSystem );
                ServerTLSSyncSystem->TLSInitialize( InWorker, InGroup );
            }
            
            if constexpr( Flags.bHasWorkerGroupSpecificTLSSync )
            {
                SKL_ASSERT( nullptr != MyTLSSyncSystem );
                MyTLSSyncSystem->TLSInitialize( InWorker, InGroup );
            }
            
            InWorker.AODTLSContext.exchange( SKL::AODTLSContext::GetInstance() );
            InWorker.ServerInstanceTLSContext.exchange( SKL::ServerInstanceTLSContext::GetInstance() );
            
            #if defined(SKL_KPI_WORKER_TICK)
            KPITimeValue TickTiming;
            #endif
            while( InGroup.IsRunning() ) SKL_LIKELY
            {
                #if defined(SKL_KPI_WORKER_TICK)
                //@TODO should update frequency once n ms or loops
                TickTiming.Begin();
                #endif

                if constexpr( Flags.bEnableAsyncIO )
                {
                    const bool bShouldTermiante{ InGroup.HandleTasks_Proactive( MillisecondsToSleep ) };
                    if ( true == bShouldTermiante ) SKL_UNLIKELY
                    {
                        break;
                    }
                }
                
                if constexpr( Flags.bEnableTaskQueue )
                {
                    if constexpr( CTask_DoThrottleGeneralTaskExecution )
                    {
                        WorkerGroup::HandleGeneralTasksWithThrottle( InWorker );
                    }
                    else
                    {
                        WorkerGroup::HandleGeneralTasks( InWorker );
                    }
                }

                if constexpr( Flags.bHandlesTimerTasks )
                {
                    if constexpr( bAllWorkerGroupsAreActive )
                    {
                        WorkerGroup::HandleTimerTasks_Local();
                    }
                    else
                    {
                        WorkerGroup::HandleTimerTasks_Global( InWorker );
                    }
                }

                if constexpr( Flags.bSupportsAOD )
                {
                    if constexpr( bAllWorkerGroupsAreActive )
                    {
                        WorkerGroup::HandleAODDelayedTasks_Local( InWorker );
                    }
                    else
                    {
                        WorkerGroup::HandleAODDelayedTasks_Global( InWorker );
                    }
                }

                if constexpr( Flags.bTickWorkerServices )
                {
                    for( size_t i = 1; i < WorkerServices.size(); ++i )
                    {
                        WorkerServices[i]->OnTickWorker( InWorker, InGroup );
                    }
                }

                if constexpr( Flags.bSupportsTLSSync )
                {
                    ServerTLSSyncSystem->TLSTick( InWorker, InGroup );
                }
                
                if constexpr( Flags.bHasWorkerGroupSpecificTLSSync )
                {
                    MyTLSSyncSystem->TLSTick( InWorker, InGroup );
                }

                if constexpr( Flags.bCallTickHandler )
                {
                    OnWorkerTick.Dispatch( InWorker, InGroup );
                }

                if constexpr( false == Flags.bEnableAsyncIO )
                {
#if defined(SKL_USE_PRECISE_SLEEP)
                    PreciseSleep( SecondsToSleep );
#else
                    TCLOCK_SLEEP_FOR_MILLIS( MillisecondsToSleep );
#endif
                }
                
                #if defined(SKL_KPI_WORKER_TICK)
                const auto TickElapsedTime = TickTiming.GetElapsedSeconds();
                InWorker.SetAverateTickTimeUsafe( TickElapsedTime );
                #endif
            }
            
            if constexpr( Flags.bSupportsTLSSync )
            {
                ServerTLSSyncSystem->TLSShutdown();
            }
            
            if constexpr( Flags.bHasWorkerGroupSpecificTLSSync )
            {
                MyTLSSyncSystem->TLSShutdown();
            }

#if defined(SKL_USE_PRECISE_SLEEP)
            PreciseSleep_WaitableTimer::Destroy();
#endif
        }
    };

    //! Reactive
    template<WorkerGroupTagFlags Flags>
    struct ReactiveWorkerVariant
    {
        static_assert( false == Flags.bIsActive );

        SKL_NOINLINE static void Run( Worker& InWorker, WorkerGroup& InGroup ) noexcept
        {
            SKLL_TRACE();

            const auto Tag                 = InGroup.GetTag(); //!< Stack tag copy
            const auto TickRate            = Tag.SyncTLSTickRate;
            const auto MillisecondsToSleep = static_cast<uint32_t>( 1000.0 / static_cast<double>( TickRate ) );
            auto*      ServerTLSSyncSystem = InGroup.GetServerInstance()->GetTSLSyncSystemPtr();
            auto*      MyTLSSyncSystem     = InGroup.MyTLSSyncSystem.get();
            
            if constexpr( Flags.bSupportsTLSSync )
            {
                SKL_ASSERT( nullptr != ServerTLSSyncSystem );
                ServerTLSSyncSystem->TLSInitialize( InWorker, InGroup );
            }
            
            if constexpr( Flags.bHasWorkerGroupSpecificTLSSync )
            {
                SKL_ASSERT( nullptr != MyTLSSyncSystem );
                MyTLSSyncSystem->TLSInitialize( InWorker, InGroup );
            }
            
            InWorker.AODTLSContext.exchange( SKL::AODTLSContext::GetInstance() );
            InWorker.ServerInstanceTLSContext.exchange( SKL::ServerInstanceTLSContext::GetInstance() );

            while( InGroup.IsRunning() ) SKL_LIKELY
            {
                if constexpr( Flags.bSupportsTLSSync )
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

                if constexpr( Flags.bSupportsTLSSync )
                {
                    ServerTLSSyncSystem->TLSTick( InWorker, InGroup );
                }

                if constexpr( Flags.bHasWorkerGroupSpecificTLSSync )
                {
                    MyTLSSyncSystem->TLSTick( InWorker, InGroup );
                }
            }
            
            if constexpr( Flags.bSupportsTLSSync )
            {
                ServerTLSSyncSystem->TLSShutdown();
            }
            
            if constexpr( Flags.bHasWorkerGroupSpecificTLSSync )
            {
                MyTLSSyncSystem->TLSShutdown();
            }
        }
    };

#define SKL_WORKER_ALL_ACTIVE_RUN_VARTIAN( bEnableAsyncIOVal, bSupportsAODVal, bHandlesTimerTasksVal, bSupportsTLSSyncVal, bCallTickHandlerVal, bTickWorkerServicesVal, bSupportsWGTLSSyncVal, bEnableTaskQueueVal )  \
             if( bEnableAsyncIOVal      == WorkerGroupTag.bEnableAsyncIO                                                                                                                                              \
              && bSupportsAODVal        == WorkerGroupTag.bSupportsAOD                                                                                                                                                \
              && bHandlesTimerTasksVal  == WorkerGroupTag.bHandlesTimerTasks                                                                                                                                          \
              && bSupportsTLSSyncVal    == WorkerGroupTag.bSupportsTLSSync                                                                                                                                            \
              && bCallTickHandlerVal    == WorkerGroupTag.bCallTickHandler                                                                                                                                            \
              && bTickWorkerServicesVal == WorkerGroupTag.bTickWorkerServices                                                                                                                                         \
              && bSupportsWGTLSSyncVal  == WorkerGroupTag.bHasWorkerGroupSpecificTLSSync                                                                                                                              \
              && bEnableTaskQueueVal    == WorkerGroupTag.bEnableTaskQueue )                                                                                                                                          \
            {                                                                                                                                                                                                         \
                constexpr WorkerGroupTagFlags Flags{                                                                                                                                                                  \
                      .bIsActive                       = true                                                                                                                                                         \
                    , .bEnableAsyncIO                  = bEnableAsyncIOVal                                                                                                                                            \
                    , .bSupportsAOD                    = bSupportsAODVal                                                                                                                                              \
                    , .bHandlesTimerTasks              = bHandlesTimerTasksVal                                                                                                                                        \
                    , .bSupportsTLSSync                = bSupportsTLSSyncVal                                                                                                                                          \
                    , .bCallTickHandler                = bCallTickHandlerVal                                                                                                                                          \
                    , .bTickWorkerServices             = bTickWorkerServicesVal                                                                                                                                       \
                    , .bHasWorkerGroupSpecificTLSSync  = bSupportsWGTLSSyncVal                                                                                                                                        \
                    , .bEnableTaskQueue                = bEnableTaskQueueVal                                                                                                                                          \
                };                                                                                                                                                                                                    \
                ActiveWorkerVariant<Flags, true>::Run( Worker, *this );                                                                                                                                               \
            }   

 #define SKL_WORKER_ACTIVE_RUN_VARTIAN( bEnableAsyncIOVal, bSupportsAODVal, bHandlesTimerTasksVal, bSupportsTLSSyncVal, bCallTickHandlerVal, bTickWorkerServicesVal, bSupportsWGTLSSyncVal, bEnableTaskQueueVal )     \
             if( bEnableAsyncIOVal      == WorkerGroupTag.bEnableAsyncIO                                                                                                                                              \
              && bSupportsAODVal        == WorkerGroupTag.bSupportsAOD                                                                                                                                                \
              && bHandlesTimerTasksVal  == WorkerGroupTag.bHandlesTimerTasks                                                                                                                                          \
              && bSupportsTLSSyncVal    == WorkerGroupTag.bSupportsTLSSync                                                                                                                                            \
              && bCallTickHandlerVal    == WorkerGroupTag.bCallTickHandler                                                                                                                                            \
              && bTickWorkerServicesVal == WorkerGroupTag.bTickWorkerServices                                                                                                                                         \
              && bSupportsWGTLSSyncVal  == WorkerGroupTag.bHasWorkerGroupSpecificTLSSync                                                                                                                              \
              && bEnableTaskQueueVal    == WorkerGroupTag.bEnableTaskQueue )                                                                                                                                          \
            {                                                                                                                                                                                                         \
                constexpr WorkerGroupTagFlags Flags{                                                                                                                                                                  \
                      .bIsActive                       = true                                                                                                                                                         \
                    , .bEnableAsyncIO                  = bEnableAsyncIOVal                                                                                                                                            \
                    , .bSupportsAOD                    = bSupportsAODVal                                                                                                                                              \
                    , .bHandlesTimerTasks              = bHandlesTimerTasksVal                                                                                                                                        \
                    , .bSupportsTLSSync                = bSupportsTLSSyncVal                                                                                                                                          \
                    , .bCallTickHandler                = bCallTickHandlerVal                                                                                                                                          \
                    , .bTickWorkerServices             = bTickWorkerServicesVal                                                                                                                                       \
                    , .bHasWorkerGroupSpecificTLSSync  = bSupportsWGTLSSyncVal                                                                                                                                        \
                    , .bEnableTaskQueue                = bEnableTaskQueueVal                                                                                                                                          \
                };                                                                                                                                                                                                    \
                ActiveWorkerVariant<Flags, false>::Run( Worker, *this );                                                                                                                                              \
            }    

 #define SKL_WORKER_REACTIVE_RUN_VARTIAN( bSupportsTLSSyncVal, bSupportsWGTLSSyncVal )      \
             if( bSupportsTLSSyncVal   == WorkerGroupTag.bSupportsTLSSync                   \
              && bSupportsWGTLSSyncVal == WorkerGroupTag.bHasWorkerGroupSpecificTLSSync )   \
            {                                                                               \
                constexpr WorkerGroupTagFlags Flags{                                        \
                      .bIsActive                       = false                              \
                    , .bEnableAsyncIO                  = false                              \
                    , .bSupportsAOD                    = false                              \
                    , .bHandlesTimerTasks              = false                              \
                    , .bSupportsTLSSync                = bSupportsTLSSyncVal                \
                    , .bCallTickHandler                = false                              \
                    , .bTickWorkerServices             = false                              \
                    , .bHasWorkerGroupSpecificTLSSync  = bSupportsWGTLSSyncVal              \
                    , .bEnableTaskQueue                = false                              \
                };                                                                          \
                ReactiveWorkerVariant<Flags>::Run( Worker, *this );                         \
            }   

}

