//!
//! \file AOD_Object.cpp
//! 
//! \brief Async Object bound Dispatcher Task abstraction for SkylakeLib
//! 
//! \reference https://github.com/balannarcis96/Dispatcher (G.O.D: Grand Object-bound Dispatcher)
//!
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 

#include "SkylakeLib.h"

namespace SKL
{
    void AODObject::Flush() noexcept
    {

    }

    bool AODObject::Dispatch( IAODTask* InTask ) noexcept
    {
        SKL_ASSERT( nullptr != InTask );
        SKL_ASSERT( false == InTask->IsNull() );

        TaskQueue.Push( InTask );
        
        if( RemainingTasksCount.increment() != 1 )
        {
            // There is a consumer present just add the task for it to be dispatched
            return false;
        }

        // We are the new consumer for this AODObject instance, dispatch all available tasks.

        // Increment ref count for self
        TSharedPtr<AODObject>::Static_IncrementReference( this );

        
    }
}