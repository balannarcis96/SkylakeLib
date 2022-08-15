//!
//! \file Worker.cpp
//! 
//! \brief Worker abstraction
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

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
        if( false == IsMaster() )
        {
            // init the SkylakeLib for this thread
            Skylake_InitializeLibrary_Thread();
        }
        
        { 
            // mark as running
            bIsRunning.exchange( true );

            // save approx start time
            StartedAt.exchange( GetSystemUpTickCount() );
        
            // notice the group
            Group->OnWorkerStarted( *this );
        
            // dispatch main task
            SKL_ASSERT_ALLWAYS( false == OnRun.IsNull() );
            OnRun.Dispatch( *this, *Group );
        
            // mark as stopped
            bIsRunning.exchange( false );
        
            // notice the group
            Group->OnWorkerStopped( *this );
        }
        
        if( false == IsMaster() )
        {
            // terminate the SkylakeLib for this thread
            Skylake_TerminateLibrary_Thread();
        }
    }
}