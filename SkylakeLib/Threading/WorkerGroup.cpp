//!
//! \file WorkerGroup.cpp
//! 
//! \brief Worker group abstraction
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

#include "SkylakeLib.h"

namespace SKL
{
    RStatus WorkerGroup::Start() noexcept
    {
        if( TRUE == bIsRunning.exchange( true ) )
        {
            SKL_INF_FMT( "[WorkerGroup::Start()][Group:%ws] Already started!", Tag.Name );
            return RSuccess;
        }
        
        for( auto& Worker : Workers )
        {
            if( Worker->IsMaster() )
            {   
                continue;
            }

            if( RSuccess != Worker->Start() )
            {
                SKL_INF_FMT( "[WorkerGroup::Start()][Group:%ws] Failed to start worker!", Tag.Name );
                return RFail;
            }
        }

        return RSuccess;
    }

    void WorkerGroup::SignalToStop() noexcept
    {
        if( false == bIsRunning.exchange( false ) )
        {
            SKL_INF_FMT( "[WorkerGroup::SignalToStop()][Group:%ws] Already signaled to stop!", Tag.Name );
            return;
        }

        if( true == Tag.bHandlesTasks )
        {
            if( RSuccess != AsyncIOAPI.Stop() )
            {
                SKL_ERR_FMT( "[WorkerGroup::SignalToStop()][Group:%ws] Failed to stop the Aasync IO system!", Tag.Name );
            }
        }
    }

    void WorkerGroup::Join() noexcept
    {
        SKL_ASSERT_ALLWAYS( false == IsRunning() );

        for( auto& Worker : Workers )
        {
            if( nullptr == Worker || true == Worker->IsMaster() )
            {
                continue;
            }

            Worker->Join();
        }
    }

    void WorkerGroup::Stop() noexcept
    {
        SignalToStop();
        Join();
    }

    RStatus WorkerGroup::Build( bool bIncludeMaster ) noexcept
    {
        if( false == Tag.IsValid() ) SKL_UNLIKELY
        {
            SKL_ERR( "WorkerGroup::Build() Invalid Tag!" );
            return RInvalidParamters;
        }        

        if( true == Tag.bHandlesTasks )
        {
            if ( RSuccess != AsyncIOAPI.Start( Tag.WorkersCount ) )
            {
                SKL_ERR_FMT( "WorkerGroup::Build() Failed to init the async IO API! GroupId[%ws]", Tag.Name );
                return RFail;
            }
        }

        if( true == Tag.bSupportsTLSSync )
        {
            //@TODO
        }

        return CreateWorkers( bIncludeMaster );
    }

    RStatus WorkerGroup::CreateWorkers( bool bIncludeMaster ) noexcept
    {
        SKL_ASSERT_ALLWAYS( Tag.WorkersCount > 0 );

        std::vector<std::shared_ptr<Worker>> Temp;
        Temp.reserve( static_cast<size_t>( Tag.Id ) );

        std::shared_ptr<Worker> MasterWorker { nullptr };

        for( uint16_t i = 0; i < Tag.WorkersCount; ++i )
        {
            // allocate new woker
            auto NewWorker = std::make_shared<Worker>( this );
            if( nullptr == NewWorker )
            {
                SKL_ERR_FMT( "[WorkerGroup:%ws] Failed to allocate new Worker!", Tag.Name );
                return RAllocationFailed;
            }

            // check if this worker must be the master worker
            const bool bIsSelectedAsMasterWorker { true == bIncludeMaster && i == Tag.WorkersCount - 1 };
            if( true == bIsSelectedAsMasterWorker )
            {
                MasterWorker = NewWorker;
            }
            
            // init as slave worker
            if( RSuccess != HandleSlaveWorker( *NewWorker ) )
            {
                SKL_ERR_FMT( "[WorkerGroup:%ws] Failed init slave Worker!", Tag.Name );
                return RFail;
            }

            Temp.emplace_back( std::move( NewWorker ) );
        }
    
        // move all workers in the main vector
        Workers = std::move( Temp );

        if( nullptr != MasterWorker )
        {
            // promote to master worker
            return HandleMasterWorker( std::move( MasterWorker ) );
        }

        return RSuccess;
    }

    RStatus WorkerGroup::HandleSlaveWorker( Worker& Worker ) noexcept
    {
        SKL_ASSERT_ALLWAYS( false == Tag.bIsActive || Tag.TickRate > 0 );

        Worker.SetOnRunHandler([]( SKL::Worker& Worker, WorkerGroup& Group ) noexcept -> void
        {
            const auto& Tag { Group.GetTag() };
            if( true == Tag.bIsActive )
            {
                Group.ProactiveWorkerRun( Worker );
            }
            else
            {
                Group.ReactiveWorkerRun( Worker );
            }
        });     

        return RSuccess;
    }

    RStatus WorkerGroup::HandleMasterWorker( std::shared_ptr<Worker> InMasterWorker ) noexcept
    {
        // marsk as master
        InMasterWorker->bIsMasterThread.exchange( TRUE ); 

        // cache master worker pointer
        MasterWorker = InMasterWorker;

        return RSuccess;
    }

    void WorkerGroup::ProactiveWorkerRun( Worker& Worker ) noexcept
    {
        const auto Tag                 { GetTag() }; //!< Stack tag copy
        const auto TickRate            { true == Tag.bSupportsTLSSync ? std::min( Tag.TickRate, Tag.SyncTLSTickRate ) : Tag.TickRate };
        const auto MillisecondsToSleep { static_cast<uint32_t>( 1000.0f / static_cast<float>( TickRate ) ) };

        if( true == Tag.bHandlesTasks )
        {
            if( true == Tag.bSupportsTLSSync )
            {
                while( IsRunning() ) SKL_LIKELY
                {
                    bool bShouldTermiante { HandleTasks_Proactive( MillisecondsToSleep ) };
                    if ( true == bShouldTermiante ) SKL_UNLIKELY
                    {
                        break;
                    }

                    OnWorkerTick.Dispatch( Worker, *this );

                    bShouldTermiante = HandleTLSSync( Worker );
                    if ( true == bShouldTermiante ) SKL_UNLIKELY
                    {
                        break;
                    }
                }
            }
            else
            {
                while( IsRunning() ) SKL_LIKELY
                {
                    const bool bShouldTermiante { HandleTasks_Proactive( MillisecondsToSleep ) };
                    if ( true == bShouldTermiante ) SKL_UNLIKELY
                    {
                        break;
                    }

                    OnWorkerTick.Dispatch( Worker, *this );
                }
            }
        }
        else
        {
            if( true == Tag.bSupportsTLSSync )
            {
                while( IsRunning() ) SKL_LIKELY
                {
                    OnWorkerTick.Dispatch( Worker, *this );

                    const bool bShouldTermiante { HandleTLSSync( Worker ) };
                    if ( true == bShouldTermiante ) SKL_UNLIKELY
                    {
                        break;
                    }

                    TCLOCK_SLEEP_FOR_MILLIS( MillisecondsToSleep );
                }
            }
            else
            {
                while( IsRunning() ) SKL_LIKELY
                {
                    OnWorkerTick.Dispatch( Worker, *this );
                    TCLOCK_SLEEP_FOR_MILLIS( MillisecondsToSleep );
                }
            }
        }
    }
    
    void WorkerGroup::ReactiveWorkerRun( Worker& Worker ) noexcept
    {
        const auto Tag                 { GetTag() }; //!< Stack tag copy
        const auto TickRate            { Tag.SyncTLSTickRate };
        const auto MillisecondsToSleep { static_cast<uint32_t>( 1000.0f / static_cast<float>( TickRate ) ) };

        // REACTIVE WORKERS MUST HANDLE TASKS
        SKL_ASSERT_ALLWAYS( true == Tag.bHandlesTasks );

        if( true == Tag.bHandlesTasks )
        {
            while( IsRunning() ) SKL_LIKELY
            {
                bool bShouldTermiante = HandleTasks_Proactive( MillisecondsToSleep );
                if ( true == bShouldTermiante ) SKL_UNLIKELY
                {
                    break;
                }

                bShouldTermiante = HandleTLSSync( Worker );
                if ( true == bShouldTermiante ) SKL_UNLIKELY
                {
                    break;
                }
            }
        }
        else
        {
            while( IsRunning() ) SKL_LIKELY
            {
                const bool bShouldTermiante = HandleTasks_Reactive();             
                if ( true == bShouldTermiante ) SKL_UNLIKELY
                {
                    break;
                }
            }
        }
    }

    bool WorkerGroup::HandleTasks_Proactive( uint32_t MillisecondsToSleep ) noexcept
    {
        AsyncIOOpaqueType* OpaqueType               { nullptr };
        TCompletionKey     CompletionKey            { nullptr };
        uint32_t           NumberOfBytesTransferred { 0 };

        const auto Result { AsyncIOAPI.TryGetCompletedAsyncRequest( &OpaqueType, &NumberOfBytesTransferred, &CompletionKey, MillisecondsToSleep ) };
        if( RTimeout == Result )
        {
            return false;
        }
        
        if( RSuccess != Result ) SKL_UNLIKELY
        {
            if( RSystemFailure ==  Result )
            {
                SKL_WRN_FMT( "WorkerGroup::HandleTasks_Reactive() [Group:%ws] Failed with status: SystemFailure", Tag.Name );
            }
        
            // signal to termiante the worker group
            return true;
        }
        
        SKL_ASSERT( nullptr != OpaqueType || nullptr != CompletionKey );

        if( nullptr != OpaqueType )
        {
            return HandleAsyncIOTask( OpaqueType, NumberOfBytesTransferred );
        }    
    
        return HandleTask( CompletionKey );
    }

    bool WorkerGroup::HandleTasks_Reactive() noexcept
    {
        AsyncIOOpaqueType* OpaqueType               { nullptr };
        TCompletionKey     CompletionKey            { nullptr };
        uint32_t           NumberOfBytesTransferred { 0 };

        const auto Result { AsyncIOAPI.GetCompletedAsyncRequest( &OpaqueType, &NumberOfBytesTransferred, &CompletionKey ) };
        if( RSuccess != Result ) SKL_UNLIKELY
        {
            if( RSystemFailure ==  Result )
            {
                SKL_WRN_FMT( "WorkerGroup::HandleTasks_Reactive() [Group:%ws] Failed with status: SystemFailure", Tag.Name );
            }
        
            // signal to termiante the worker group
            return true;
        }
        
        SKL_ASSERT( nullptr != OpaqueType || nullptr != CompletionKey );

        if( nullptr != OpaqueType )
        {
            return HandleAsyncIOTask( OpaqueType, NumberOfBytesTransferred );
        }    
    
        return HandleTask( CompletionKey );
    }

    bool WorkerGroup::HandleAsyncIOTask( AsyncIOOpaqueType* InOpaque, uint32_t NumberOfBytesTransferred ) noexcept
    {
        SKL_ASSERT( nullptr != InOpaque );

        {
            // cast back to shared object IAsyncIOTask
            TSharedPtr<IAsyncIOTask> Task { reinterpret_cast<IAsyncIOTask*>( InOpaque ) };
            
            // dispatch the task
            Task->Dispatch();
        }

        return false;
    }

    bool WorkerGroup::HandleTask( TCompletionKey InCompletionKey ) noexcept
    {
        SKL_ASSERT( nullptr != InCompletionKey );

        {
            // cast back to shared object ITask
            TSharedPtr<ITask> Task { reinterpret_cast<ITask*>( InCompletionKey ) };
        
            // dispatch the task
            Task->Dispatch();
        }

        return false;
    }

    bool WorkerGroup::HandleTLSSync( Worker& Worker ) noexcept
    {
        //@TODO
        return  false;
    }

    void WorkerGroup::OnWorkerStarted( Worker& Worker ) noexcept
    {
        const auto NewRunningWorkersCount { RunningWorkers.increment() };

        SKL_VER_FMT( "[WG:%ws] Worker started!", Tag.Name );

        if( false == OnWorkerStartTask.IsNull() )
        {
            OnWorkerStartTask.Dispatch( Worker, *this );
        }

        if( TotalWorkers.load_relaxed() == NewRunningWorkersCount )
        {
            SKL_VER_FMT( "[WG:%ws] All workers started!", Tag.Name );
            Manager->OnWorkerGroupStarted( *this );
        }
    }
    
    void WorkerGroup::OnWorkerStopped( Worker& Worker ) noexcept
    {
        const auto NewRunningWorkersCount { RunningWorkers.decrement() };

        SKL_VER_FMT( "[WG:%ws] Worker stopped!", Tag.Name );

        if( false == OnWorkerStopTask.IsNull() )
        {
            OnWorkerStopTask.Dispatch( Worker, *this );
        }

        if( 0 == NewRunningWorkersCount )
        {
            SKL_VER_FMT( "[WG:%ws] All workers stopped!", Tag.Name );
            Manager->OnWorkerGroupStopped( *this );
        }
    }
}