//!
//! \file SkylakeLib.h
//! 
//! \brief Interface for the SkylakeLib
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

//! The header only part
#include <SkylakeLibHeaderOnly.h>

//! Std
#include "Std/Std.h"

//! Tuning
#include "Tuning/Tuning.h"

//! Logging
#include "Diagnostics/Log.h"

extern void _mi_assert_fail(const char* assertion, const char* fname, unsigned line, const char* func );
#define mi_assert(expr)     ((expr) ? (void)0 : _mi_assert_fail(#expr,__FILE__,__LINE__,__func__))

#include "Heading.h"

//! Os specific port
#include "Port/Port.h"

//! Spin Lock
#include "Utils/SpinLock.h"

//! Skylake Random
#include "Utils/SRand.h"

//! Short-GUID
#include "Utils/SGUID.h"

//! BinaryStream
#include "Utils/BinaryStream.h"

//! Memory abstractions
#include "Memory/Memory.h"

//! TLS Sync System
#include "TLSSync/TLSSync.h"

//! Task
#include "Task/Task.h"
#include "Task/TaskQueue.h"

//! AOD [Task, Queue, TLSContext]
#include "AOD/AOD_Task.h"
#include "AOD/AOD_Queue.h"
#include "AOD/AOD_TLS.h"

//! Threading
#include "Threading/Threading.h"

//! Networking
#include "Networking/Networking.h"

//! AOD
#include "AOD/AOD.h"

//! Service
#include "Service\Service.h"

//! Application
#include "Application/Application.h"

//! ECS
#include "ECS/ECS.h"

namespace SKL
{
    //! Initialize the SkylakeLibrary
    RStatus Skylake_InitializeLibrary( int32_t Argc, char** Argv, FILE* InLogOutput ) noexcept;

    //! Terminate the SkylakeLibrary
    RStatus Skylake_TerminateLibrary() noexcept;

    //! Initialize the SkylakeLibrary per thread
    RStatus Skylake_InitializeLibrary_Thread() noexcept;

    //! Terminate the SkylakeLibrary per thread
    RStatus Skylake_TerminateLibrary_Thread() noexcept;

    //! Is the skylake library init and ready to use
    bool Skylake_IsTheLibraryInitialize() noexcept;

    //! Is the skylake library init and ready to use for the calling thread
    bool Skylake_IsTheLibraryInitialize_Thread() noexcept;
}
