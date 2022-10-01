//!
//! \file Worker.cpp
//! 
//! \brief Worker abstraction
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 

#if !defined(SKL_STANDALONE)
#include "SkylakeLib.h"

namespace SKL
{
    //! Start the worker
    RStatus Worker::Start() noexcept   
    {
        Thread = std::jthread( [ this ]() mutable noexcept -> void
        {
            RunImpl();
        } );

        return RSuccess;
    }
    
    void Worker::RunImpl() noexcept
    {
        SKL_ASSERT( nullptr != Group && true == Group->IsRunning() );
        SKL_ASSERT( true == Group->IsRunning() );

        // init the SkylakeLib for this thread
        Skylake_InitializeLibrary_Thread();
        
        // mark as running
        bIsRunning.exchange( TRUE );

        auto* SInstance{ Group->GetServerInstance() };
        SKL_ASSERT( nullptr != SInstance );
        
        // notice the group
        if ( true == Group->OnWorkerStarted( *this ) )
        {
            // dispatch main task
            SKL_ASSERT_ALLWAYS( false == OnRun.IsNull() );

            if( nullptr != SInstance->SyncWorkerStartup )
            {
                // wait for all other workers to rich this stage
                SInstance->SyncWorkerStartup->arrive_and_wait();
            }

            // save approx start time
            StartedAt.exchange( GetSystemUpTickCount() );
        
            // execute onRun handler
            OnRun.Dispatch( *this, *Group );
        }
        else
        {
            if( nullptr != SInstance->SyncWorkerStartup )
            {
                // wait for all other workers to start before issuing the stop server
                SInstance->SyncWorkerStartup->arrive_and_wait();
            }
            
            Group->GetServerInstance()->SignalToStop( true );

            SKLL_TRACE_MSG_FMT( "Failure WG:%ws", Group->GetTag().Name );
        }
        
        // mark as stopped
        bIsRunning.exchange( FALSE );

        // notice the group
        Group->OnWorkerStopped( *this );
        
        // terminate the SkylakeLib for this thread
        Skylake_TerminateLibrary_Thread();
    }

    void Worker::Clear() noexcept
    {
        // Clear global delayed tasks
        while( auto* Task{ DelayedTasks.Pop() })
        {
            TSharedPtr<ITask>::Static_Reset( Task );
        }

        // Clear AOD shared object delayed tasks
        while( auto* Task{ AODSharedObjectDelayedTasks.Pop() })
        {
            TSharedPtr<IAODSharedObjectTask>::Static_Reset( reinterpret_cast<IAODSharedObjectTask*>( Task ) );
        }
        
        // Clear AOD custom object delayed tasks
        while( auto* Task{ AODCustomObjectDelayedTasks.Pop() })
        {
            TSharedPtr<IAODCustomObjectTask>::Static_Reset( reinterpret_cast<IAODCustomObjectTask*>( Task ) );
        }

        // Clear AOD static object delayed tasks
        while( auto* Task{ AODStaticObjectDelayedTasks.Pop() })
        {
            TSharedPtr<IAODStaticObjectTask>::Static_Reset( reinterpret_cast<IAODStaticObjectTask*>( Task ) );
        }
    }
}

#endif