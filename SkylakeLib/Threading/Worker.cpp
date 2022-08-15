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
        else
        {
            //The master thread must have the library initialized before run
            SKL_ASSERT_ALLWAYS( IsMaster() && Skylake_IsTheLibraryInitialize_Thread() );
        }
        
        { 
            // mark as running
            bIsRunning.exchange( true );
        
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
        
        if( true == IsMaster() )
        {
            // notice group of mater termination 
            SKL_ASSERT_ALLWAYS( false == OnMasterTermianted.IsNull() );
            OnMasterTermianted( *this, *Group );
        
            SKL_INF_FMT( "WorkerGroup[%hu] Master termianted!", Group->GetTag().Id );
        }
        else
        {
            SKL_INF_FMT( "WorkerGroup[%hu] Slave termianted!", Group->GetTag().Id );
        }
        
        if( false == IsMaster() )
        {
            // terminate the SkylakeLib for this thread
            Skylake_TerminateLibrary_Thread();
        }
    }
    

    //! Signal the worker to stop
    void Worker::SignalStop() noexcept  
    {
        if( false == bIsRunning.exchange( false ) )
        {
            SKL_INF_FMT( "[Worker::SignalStop()][Group:%hu] Worker already signaled to stop!", Group->GetTag( ).Id );
        }
    }
}