//!
//! \file Worker.cpp
//! 
//! \brief Worker abstraction
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 

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
        if( nullptr == Group || false == Group->IsRunning() )
        {
            SKL_WRN_FMT( "[WG:%ws] Worker::RunImpl() Can't RUN!", Group ? Group->GetTag().Name : L"UNKNOWN" );
            return;
        }

        if( false == IsMaster() )
        {
            // init the SkylakeLib for this thread
            Skylake_InitializeLibrary_Thread();
        }
        
        { 
            // mark as running
            bIsRunning.exchange( TRUE );

            // save approx start time
            StartedAt.exchange( GetSystemUpTickCount() );
        
            if ( true == Group->IsRunning() )
            {
                // notice the group
                if ( true == Group->OnWorkerStarted(*this) )
                {
                    // dispatch main task
                    SKL_ASSERT_ALLWAYS( false == OnRun.IsNull() );
                    OnRun.Dispatch( *this, *Group );
                }
                else
                {
                    Group->GetServerInstance()->SignalToStop( true );
                }
        
                // mark as stopped
                bIsRunning.exchange( FALSE );

                // notice the group
                Group->OnWorkerStopped( *this );
            }
            else
            {
                bIsRunning.exchange( FALSE );
                SKL_VER_FMT( "[WG:%ws] Worker::RunImpl() Early stopp!", Group->GetTag().Name );
            }
        }
        
        if( false == IsMaster() )
        {
            // terminate the SkylakeLib for this thread
            Skylake_TerminateLibrary_Thread();
        }
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

        // Clear AOD static object delayed tasks
        while( auto* Task{ AODStaticObjectDelayedTasks.Pop() })
        {
            TSharedPtr<IAODStaticObjectTask>::Static_Reset( reinterpret_cast<IAODStaticObjectTask*>( Task ) );
        }
    }
}