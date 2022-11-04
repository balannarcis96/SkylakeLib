//!
//! \file WorkerGroup.cpp
//! 
//! \brief Worker group abstraction
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#include "SkylakeLib.h"
#include "WorkerGroupRunVariants.h"

namespace SKL
{
    RStatus WorkerGroup::Start() noexcept
    {
        if( TRUE == bIsRunning.exchange( true ) )
        {
            SKLL_INF_FMT( "[WorkerGroup::Start()][Group:%ws] Already started!", Tag.Name );
            return RSuccess;
        }
        
        for( auto& Worker : Workers )
        {
            if( nullptr == Worker ) { continue; }

            if( Worker->IsMaster() )
            {   
                continue;
            }

            if( RSuccess != Worker->Start() )
            {
                SKLL_INF_FMT( "[WorkerGroup::Start()][Group:%ws] Failed to start worker!", Tag.Name );
                return RFail;
            }
        }

        return RSuccess;
    }

    void WorkerGroup::SignalToStop() noexcept
    {
        if( false == bIsRunning.exchange( false ) )
        {
            SKLL_INF_FMT( "[WorkerGroup::SignalToStop()][Group:%ws] Already signaled to stop!", Tag.Name );
            return;
        }

        // stop all acceptors
        StopAllTCPAcceptors();

        if( true == Tag.bHandlesTasks )
        {
            if( RSuccess != AsyncIOAPI.Stop() )
            {
                SKLL_ERR_FMT( "[WorkerGroup::SignalToStop()][Group:%ws] Failed to stop the Aasync IO system!", Tag.Name );
            }
        }
    }

    void WorkerGroup::Join() noexcept
    {
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
            SKLL_ERR( "WorkerGroup::Build() Invalid Tag!" );
            return RInvalidParamters;
        }        

        if( true == Tag.bHandlesTasks )
        {
            if ( RSuccess != AsyncIOAPI.Start( Tag.WorkersCount ) )
            {
                SKLL_ERR_FMT( "WorkerGroup::Build() Failed to init the async IO API! GroupId[%ws]", Tag.Name );
                return RFail;
            }
        }

        //@TODO move the TLSSyncSystem at ServerInstance level
        if( true == Tag.bSupportsTLSSync )
        {
            MyTLSSyncSystem.reset( new TLSSyncSystem() );
            SKL_ASSERT( nullptr != MyTLSSyncSystem.get() );
            if ( nullptr == MyTLSSyncSystem )
            {
                SKLL_ERR_FMT( "WorkerGroup::Build() Failed to allocate the TLSSyncSystem! GroupId[%ws]", Tag.Name );
                return RFail;
            }
        }

        return CreateWorkers( bIncludeMaster );
    }

    RStatus WorkerGroup::CreateWorkers( bool bIncludeMaster ) noexcept
    {
        SKL_ASSERT_ALLWAYS( Tag.WorkersCount > 0 );

        std::vector<std::unique_ptr<Worker>> Temp{};
        Temp.reserve( static_cast<size_t>( Tag.WorkersCount ) + 1 );
        Temp.emplace_back( nullptr ); //index zero is not valid!

        Worker* NewMasterWorker{ nullptr };

        for( uint16_t i = 0; i < Tag.WorkersCount; ++i )
        {
            // allocate new woker
            auto NewWorker = std::make_unique<Worker>( this );
            if( nullptr == NewWorker )
            {
                SKLL_ERR_FMT( "[WorkerGroup:%ws] Failed to allocate new Worker!", Tag.Name );
                return RAllocationFailed;
            }

            // check if this worker must be the master worker
            const bool bIsSelectedAsMasterWorker{ true == bIncludeMaster && i == Tag.WorkersCount - 1 };
            if( true == bIsSelectedAsMasterWorker )
            {
                NewMasterWorker = NewWorker.get();
            }
            
            // init as slave worker
            if( RSuccess != HandleSlaveWorker( *NewWorker ) )
            {
                SKLL_ERR_FMT( "[WorkerGroup:%ws] Failed init slave Worker!", Tag.Name );
                return RFail;
            }

            ( void )TotalWorkers.increment();
            Temp.emplace_back( std::move( NewWorker ) );
        }
    
        // move all workers in the main vector
        Workers = std::move( Temp );

        if( nullptr != NewMasterWorker )
        {
            // promote to master worker
            return HandleMasterWorker( NewMasterWorker );
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

    RStatus WorkerGroup::HandleMasterWorker( Worker* InMasterWorker ) noexcept
    {
        // mark as master
        InMasterWorker->bIsMasterThread.exchange( TRUE ); 

        // cache master worker pointer
        MasterWorker = InMasterWorker;

        return RSuccess;
    }

    void WorkerGroup::ProactiveWorkerRun( Worker& Worker ) noexcept
    {
        const auto& WorkerGroupTag{ GetTag() };
        const auto bAllGroupsAreActive{ GetServerInstance()->GetFlags().bAllGroupsAreActive };
        if( TRUE == bAllGroupsAreActive )
        {
            if( false == WorkerGroupTag.bHandlesTasks
             && false == WorkerGroupTag.bSupportsAOD 
             && false == WorkerGroupTag.bHandlesTimerTasks 
             && false == WorkerGroupTag.bSupportsTLSSync 
             && false == WorkerGroupTag.bCallTickHandler )
            {
                ActiveWorkerVariant<false, false, false, false, false, true>::Run( Worker, *this );
            }
            //SKL_WORKER_ALL_ACTIVE_RUN_VARTIAN( false    ,false    ,false    ,false    ,false )
            SKL_WORKER_ALL_ACTIVE_RUN_VARTIAN( false    ,false    ,false    ,false    , true )
            SKL_WORKER_ALL_ACTIVE_RUN_VARTIAN( false    ,false    ,false    , true    ,false )
            SKL_WORKER_ALL_ACTIVE_RUN_VARTIAN( false    ,false    ,false    , true    , true )
            SKL_WORKER_ALL_ACTIVE_RUN_VARTIAN( false    ,false    , true    ,false    ,false )
            SKL_WORKER_ALL_ACTIVE_RUN_VARTIAN( false    ,false    , true    ,false    , true )
            SKL_WORKER_ALL_ACTIVE_RUN_VARTIAN( false    ,false    , true    , true    ,false )
            SKL_WORKER_ALL_ACTIVE_RUN_VARTIAN( false    ,false    , true    , true    , true )
            SKL_WORKER_ALL_ACTIVE_RUN_VARTIAN( false    , true    ,false    ,false    ,false )
            SKL_WORKER_ALL_ACTIVE_RUN_VARTIAN( false    , true    ,false    ,false    , true )
            SKL_WORKER_ALL_ACTIVE_RUN_VARTIAN( false    , true    ,false    , true    ,false )
            SKL_WORKER_ALL_ACTIVE_RUN_VARTIAN( false    , true    ,false    , true    , true )
            SKL_WORKER_ALL_ACTIVE_RUN_VARTIAN( false    , true    , true    ,false    ,false )
            SKL_WORKER_ALL_ACTIVE_RUN_VARTIAN( false    , true    , true    ,false    , true )
            SKL_WORKER_ALL_ACTIVE_RUN_VARTIAN( false    , true    , true    , true    ,false )
            SKL_WORKER_ALL_ACTIVE_RUN_VARTIAN( false    , true    , true    , true    , true )
            SKL_WORKER_ALL_ACTIVE_RUN_VARTIAN(  true    ,false    ,false    ,false    ,false )
            SKL_WORKER_ALL_ACTIVE_RUN_VARTIAN(  true    ,false    ,false    ,false    , true )
            SKL_WORKER_ALL_ACTIVE_RUN_VARTIAN(  true    ,false    ,false    , true    ,false )
            SKL_WORKER_ALL_ACTIVE_RUN_VARTIAN(  true    ,false    ,false    , true    , true )
            SKL_WORKER_ALL_ACTIVE_RUN_VARTIAN(  true    ,false    , true    ,false    ,false )
            SKL_WORKER_ALL_ACTIVE_RUN_VARTIAN(  true    ,false    , true    ,false    , true )
            SKL_WORKER_ALL_ACTIVE_RUN_VARTIAN(  true    ,false    , true    , true    ,false )
            SKL_WORKER_ALL_ACTIVE_RUN_VARTIAN(  true    ,false    , true    , true    , true )
            SKL_WORKER_ALL_ACTIVE_RUN_VARTIAN(  true    , true    ,false    ,false    ,false )
            SKL_WORKER_ALL_ACTIVE_RUN_VARTIAN(  true    , true    ,false    ,false    , true )
            SKL_WORKER_ALL_ACTIVE_RUN_VARTIAN(  true    , true    ,false    , true    ,false )
            SKL_WORKER_ALL_ACTIVE_RUN_VARTIAN(  true    , true    ,false    , true    , true )
            SKL_WORKER_ALL_ACTIVE_RUN_VARTIAN(  true    , true    , true    ,false    ,false )
            SKL_WORKER_ALL_ACTIVE_RUN_VARTIAN(  true    , true    , true    ,false    , true )
            SKL_WORKER_ALL_ACTIVE_RUN_VARTIAN(  true    , true    , true    , true    ,false )
            SKL_WORKER_ALL_ACTIVE_RUN_VARTIAN(  true    , true    , true    , true    , true )
        }
        else
        {
            if( false == WorkerGroupTag.bHandlesTasks
             && false == WorkerGroupTag.bSupportsAOD 
             && false == WorkerGroupTag.bHandlesTimerTasks 
             && false == WorkerGroupTag.bSupportsTLSSync 
             && false == WorkerGroupTag.bCallTickHandler )
            {
                ActiveWorkerVariant<false, false, false, false, false, false>::Run( Worker, *this );
            }
            //SKL_WORKER_ACTIVE_RUN_VARTIAN( false    ,false    ,false    ,false    ,false )
            SKL_WORKER_ACTIVE_RUN_VARTIAN( false    ,false    ,false    ,false    , true )
            SKL_WORKER_ACTIVE_RUN_VARTIAN( false    ,false    ,false    , true    ,false )
            SKL_WORKER_ACTIVE_RUN_VARTIAN( false    ,false    ,false    , true    , true )
            SKL_WORKER_ACTIVE_RUN_VARTIAN( false    ,false    , true    ,false    ,false )
            SKL_WORKER_ACTIVE_RUN_VARTIAN( false    ,false    , true    ,false    , true )
            SKL_WORKER_ACTIVE_RUN_VARTIAN( false    ,false    , true    , true    ,false )
            SKL_WORKER_ACTIVE_RUN_VARTIAN( false    ,false    , true    , true    , true )
            SKL_WORKER_ACTIVE_RUN_VARTIAN( false    , true    ,false    ,false    ,false )
            SKL_WORKER_ACTIVE_RUN_VARTIAN( false    , true    ,false    ,false    , true )
            SKL_WORKER_ACTIVE_RUN_VARTIAN( false    , true    ,false    , true    ,false )
            SKL_WORKER_ACTIVE_RUN_VARTIAN( false    , true    ,false    , true    , true )
            SKL_WORKER_ACTIVE_RUN_VARTIAN( false    , true    , true    ,false    ,false )
            SKL_WORKER_ACTIVE_RUN_VARTIAN( false    , true    , true    ,false    , true )
            SKL_WORKER_ACTIVE_RUN_VARTIAN( false    , true    , true    , true    ,false )
            SKL_WORKER_ACTIVE_RUN_VARTIAN( false    , true    , true    , true    , true )
            SKL_WORKER_ACTIVE_RUN_VARTIAN(  true    ,false    ,false    ,false    ,false )
            SKL_WORKER_ACTIVE_RUN_VARTIAN(  true    ,false    ,false    ,false    , true )
            SKL_WORKER_ACTIVE_RUN_VARTIAN(  true    ,false    ,false    , true    ,false )
            SKL_WORKER_ACTIVE_RUN_VARTIAN(  true    ,false    ,false    , true    , true )
            SKL_WORKER_ACTIVE_RUN_VARTIAN(  true    ,false    , true    ,false    ,false )
            SKL_WORKER_ACTIVE_RUN_VARTIAN(  true    ,false    , true    ,false    , true )
            SKL_WORKER_ACTIVE_RUN_VARTIAN(  true    ,false    , true    , true    ,false )
            SKL_WORKER_ACTIVE_RUN_VARTIAN(  true    ,false    , true    , true    , true )
            SKL_WORKER_ACTIVE_RUN_VARTIAN(  true    , true    ,false    ,false    ,false )
            SKL_WORKER_ACTIVE_RUN_VARTIAN(  true    , true    ,false    ,false    , true )
            SKL_WORKER_ACTIVE_RUN_VARTIAN(  true    , true    ,false    , true    ,false )
            SKL_WORKER_ACTIVE_RUN_VARTIAN(  true    , true    ,false    , true    , true )
            SKL_WORKER_ACTIVE_RUN_VARTIAN(  true    , true    , true    ,false    ,false )
            SKL_WORKER_ACTIVE_RUN_VARTIAN(  true    , true    , true    ,false    , true )
            SKL_WORKER_ACTIVE_RUN_VARTIAN(  true    , true    , true    , true    ,false )
            SKL_WORKER_ACTIVE_RUN_VARTIAN(  true    , true    , true    , true    , true )
        }
    }
    
    void WorkerGroup::ReactiveWorkerRun( Worker& Worker ) noexcept
    {
        const auto& WorkerGroupTag{ GetTag() };
        if( true == WorkerGroupTag.bSupportsTLSSync )
        {
            ReactiveWorkerVariant<true>::Run( Worker, *this );
        }
        else
        {
            ReactiveWorkerVariant<false>::Run( Worker, *this );
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
                SKLL_WRN_FMT( "WorkerGroup::HandleTasks_Reactive() [Group:%ws] Failed with status: SystemFailure", Tag.Name );
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
                SKLL_WRN_FMT( "WorkerGroup::HandleTasks_Reactive() [Group:%ws] Failed with status: SystemFailure", Tag.Name );
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
            auto* Task { reinterpret_cast<IAsyncIOTask*>( InOpaque ) };
            
            // dispatch the task
            Task->Dispatch( NumberOfBytesTransferred );

            // release ref
            TSharedPtr<IAsyncIOTask>::Static_Reset( Task );
        }

        return false;
    }

    bool WorkerGroup::HandleTask( TCompletionKey InCompletionKey ) noexcept
    {
        SKL_ASSERT( nullptr != InCompletionKey );

        {
            // cast back to shared object ITask
            auto* Task { reinterpret_cast<ITask*>( InCompletionKey ) };
        
            // dispatch the task
            Task->Dispatch();

            // release ref
            TSharedPtr<ITask>::Static_Reset( Task );
        }

        return false;
    }

    bool WorkerGroup::HandleAODDelayedTasks_Local( Worker& /*Worker*/ ) noexcept
    {
        //SKLL_TRACE();

        auto& TLSContext{ *AODTLSContext::GetInstance() };
        auto  Now{ GetSystemUpTickCount() };

        //Shared Object tasks
        while( false == TLSContext.DelayedSharedObjectTasks.empty() )
        {
            auto* Task{ TLSContext.DelayedSharedObjectTasks.top() };
            SKL_ASSERT( nullptr != Task );

            if( true == Task->IsDue( Now ) )
            {
                TLSContext.DelayedSharedObjectTasks.pop();

                auto* Parent{ Task->GetParent() };
                SKL_ASSERT( nullptr != Parent );
                ( void )Parent->Dispatch( Task );
            }
            else
            {
                break;
            }
        }
        
        //Update now
        Now = GetSystemUpTickCount();

        //Custom Object tasks
        while( false == TLSContext.DelayedCustomObjectTasks.empty() )
        {
            auto* Task{ TLSContext.DelayedCustomObjectTasks.top() };
            SKL_ASSERT( nullptr != Task );

            if( true == Task->IsDue( Now ) )
            {
                TLSContext.DelayedCustomObjectTasks.pop();

                auto* Parent{ Task->GetParent() };
                SKL_ASSERT( nullptr != Parent );
                ( void )Parent->Dispatch( Task );
            }
            else
            {
                break;
            }
        }

        //Update now
        Now = GetSystemUpTickCount();

        //Static Object tasks
        while( false == TLSContext.DelayedStaticObjectTasks.empty() )
        {
            auto* Task{ TLSContext.DelayedStaticObjectTasks.top() };
            SKL_ASSERT( nullptr != Task );

            if( true == Task->IsDue( Now ) )
            {
                TLSContext.DelayedStaticObjectTasks.pop();

                auto* Parent{ Task->GetParent() };
                SKL_ASSERT( nullptr != Parent );
                ( void )Parent->Dispatch( Task );
            }
            else
            {
                break;
            }
        }

        return  false;
    }

    bool WorkerGroup::HandleAODDelayedTasks_Global( Worker& Worker ) noexcept
    {
        //SKLL_TRACE();

        auto& TLSContext{ *AODTLSContext::GetInstance() };
        auto  Now{ GetSystemUpTickCount() };

        //Shared Object tasks
        while( auto* NewTask{ reinterpret_cast<IAODSharedObjectTask*>( Worker.AODSharedObjectDelayedTasks.Pop() ) } )
        {
            if( true == NewTask->IsDue( Now ) )
            {
                auto* Parent{ NewTask->GetParent() };
                SKL_ASSERT( nullptr != Parent );
                ( void )Parent->Dispatch( NewTask );
            }
            else
            {
                TLSContext.DelayedSharedObjectTasks.push( NewTask );
            }
        }

        //Update now
        Now = GetSystemUpTickCount();

        //Custom Object tasks
        while( auto* NewTask{ reinterpret_cast<IAODCustomObjectTask*>( Worker.AODCustomObjectDelayedTasks.Pop() ) } )
        {
            if( true == NewTask->IsDue( Now ) )
            {
                auto* Parent{ NewTask->GetParent() };
                SKL_ASSERT( nullptr != Parent );
                ( void )Parent->Dispatch( NewTask );
            }
            else
            {
                TLSContext.DelayedCustomObjectTasks.push( NewTask );
            }
        }

        //Update now
        Now = GetSystemUpTickCount();

        //Static Object tasks
        while( auto* NewTask{ reinterpret_cast<IAODStaticObjectTask*>( Worker.AODStaticObjectDelayedTasks.Pop() ) } )
        {
            if( true == NewTask->IsDue( Now ) )
            {
                auto* Parent{ NewTask->GetParent() };
                SKL_ASSERT( nullptr != Parent );
                ( void )Parent->Dispatch( NewTask );
            }
            else
            {
                TLSContext.DelayedStaticObjectTasks.push( NewTask );
            }
        }

        return HandleAODDelayedTasks_Local( Worker );
    }

    bool WorkerGroup::HandleTimerTasks_Local( Worker& /*Worker*/ ) noexcept
    {
        auto&      TLSContext{ *ServerInstanceTLSContext::GetInstance() };
        const auto Now{ GetSystemUpTickCount() };

        while( false == TLSContext.DelayedTasks.empty() )
        {
            auto* Task{ TLSContext.DelayedTasks.top() };
            SKL_ASSERT( nullptr != Task );

            if( true == Task->IsDue( Now ) )
            {
                // pop the task first so if the dispatch 
                // adds new task to the delayed tasks 
                // we dont remove the new one
                TLSContext.DelayedTasks.pop();

                Task->Dispatch();

                TSharedPtr<ITask>::Static_Reset( Task );
            }
            else
            {
                break;
            }
        }

        return  false;
    }

    bool WorkerGroup::HandleTimerTasks_Global( Worker& Worker ) noexcept
    {
        auto&      TLSContext{ *ServerInstanceTLSContext::GetInstance() };
        const auto Now{ GetSystemUpTickCount() };

        while( auto* NewTask{ Worker.DelayedTasks.Pop() } )
        {
            if( true == NewTask->IsDue( Now ) )
            {
                NewTask->Dispatch();
                TSharedPtr<ITask>::Static_Reset( NewTask );
            }
            else
            {
                TLSContext.DelayedTasks.push( NewTask );
            }
        }

        return HandleTimerTasks_Local( Worker );
    }

    bool WorkerGroup::OnWorkerStarted( Worker& Worker ) noexcept
    {
        const auto NewRunningWorkersCount { RunningWorkers.increment() + 1 };

        if( false == Manager->OnWorkerStarted( Worker, *this ) )
        {
            return false;
        }

        if( false == OnWorkerStartTask.IsNull() )
        {
            if( false == OnWorkerStartTask.Dispatch( Worker, *this ) )
            {
                return false;
            }
        }

        if( TotalWorkers.load_relaxed() == NewRunningWorkersCount )
        {
            if ( false == OnAllWorkersStarted() )
            {
                return false;
            }

            return Manager->OnWorkerGroupStarted( *this );
        }

        return true;
    }

    bool WorkerGroup::OnWorkerStopped( Worker& Worker ) noexcept
    {
        const auto NewRunningWorkersCount { RunningWorkers.decrement() - 1 };
        SKLL_TRACE_MSG_FMT( "NewRunningWorkersCount:%u", NewRunningWorkersCount );

        if ( false == Manager->OnWorkerStopped( Worker, *this ) )
        {
            return false;
        }

        if( false == OnWorkerStopTask.IsNull() )
        {
            if ( false == OnWorkerStopTask.Dispatch( Worker, *this ) )
            {
                return false;
            }       
        }

        if( 0 == NewRunningWorkersCount )
        {
            if( nullptr != GetServerInstance()->SyncWorkerShutdown )
            {
                // wait for all other workers to stop before going further
                GetServerInstance()->SyncWorkerShutdown->arrive_and_wait();
            }

            if ( false == OnAllWorkersStopped() )
            {
                return false;
            }

            return Manager->OnWorkerGroupStopped( *this );
        }
        else
        {
            if( nullptr != GetServerInstance()->SyncWorkerShutdown )
            {
                // wait for all other workers to stop before going further
                GetServerInstance()->SyncWorkerShutdown->arrive_and_wait();
            }
        }

        return true;
    }
    
    bool WorkerGroup::OnAllWorkersStarted() noexcept
    {
        if( false == StartAllTCPAcceptors() )
        {
            return false;
        }
            
        return Manager->OnAllWorkersStarted( *this );
    }

    bool WorkerGroup::OnAllWorkersStopped() noexcept
    {
        return Manager->OnAllWorkersStopped( *this );
    }

    bool WorkerGroup::StartAllTCPAcceptors() noexcept
    {
        for( auto& Acceptor : TCPAcceptors )
        {
            if( nullptr == Acceptor ){ continue; }

            if ( RSuccess != Acceptor->StartAcceptingAsync() )
            {
                SKLL_ERR_FMT( "[WG:%ws] Failed to start async acceptor ip[%d] port[%hu] id[%u]"
                           , GetTag().Name
                           , Acceptor->GetConfig().IpAddress
                           , Acceptor->GetConfig().Port
                           , Acceptor->GetConfig().Id );
            
                // signal to stop the entire management
                Manager->SignalToStop();
            }
        }
    
        SKLL_VER_FMT( "[WG:%ws] Started all tcp async acceptors!", GetTag().Name );

        return true;
    }

    void WorkerGroup::StopAllTCPAcceptors() noexcept
    {
        for( auto& Acceptor : TCPAcceptors )
        {
            if( nullptr == Acceptor ){ continue; }

            Acceptor->StopAcceptingAsync();
        }
    
        SKLL_VER_FMT( "[WG:%ws] Stopped all tcp async acceptors!", GetTag().Name );
    }

    RStatus WorkerGroup::AddNewTCPAcceptor( const TCPAcceptorConfig& Config ) noexcept
    {
        if( false == Tag.bSupportesTCPAsyncAcceptors )
        {
            SKLL_VER_FMT( "WorkerGroup[%ws]::AddNewTCPAcceptor() Async TCP acceptors are not supported on this workers group.[bSupportesTCPAsyncAcceptors == false]!", Tag.Name );
            return RNotSupported;
        }

        if( nullptr != GetTCPAcceptorById( Config.Id ) )
        {
            SKLL_VER_FMT( "WorkerGroup[%ws]::AddNewTCPAcceptor() A tcp async acceptor with same id found id[%u]!", Tag.Name, Config.Id );
            return RInvalidParamters;
        }
        
        if( nullptr != GetTCPAcceptor( Config.IpAddress, Config.Port ) )
        {
            SKLL_VER_FMT( "WorkerGroup[%ws]::AddNewTCPAcceptor() A tcp async acceptor with same ip and port found id[%u] port[%hu]!", Tag.Name, Config.Id, Config.Port );
            return RInvalidParamters;
        }

        auto NewTCPAcceptor = std::make_unique<TCPAcceptor>( Config, &AsyncIOAPI );
        if( nullptr == NewTCPAcceptor ) SKL_UNLIKELY
        {
            SKLL_VER_FMT( "WorkerGroup[%ws]::AddNewTCPAcceptor() Failed to allocate! id[%u] ip[%hu] port[%hu]!", Tag.Name, Config.Id, Config.IpAddress, Config.Port );
            return RInvalidParamters;
        }

        TCPAcceptors.emplace_back( std::move( NewTCPAcceptor ) );

        return RSuccess;
    }

#undef SKL_WORKER_ACTIVE_RUN_VARTIAN
}
