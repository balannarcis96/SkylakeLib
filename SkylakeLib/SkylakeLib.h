//!
//! \file SkylakeLib.h
//! 
//! \brief Interface for the SkylakeLib
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

//! The standalone part
#include <SkylakeLibStandalone.h>

//#define SKL_MEM_MANAGER_DECAY_TO_GLOBAL

//! Std
#include "Std/Std.h"

//! Flags
#include "Flags.h"

//! Tuning
#include "Tuning/Tuning.h"

#if defined(SKL_DEBUG)
    extern void _mi_assert_fail(const char* assertion, const char* fname, unsigned line, const char* func );
    #define mi_assert(expr)     ((expr) ? (void)0 : _mi_assert_fail(#expr,__FILE__,__LINE__,__func__))
#else
    #define mi_assert(expr)
#endif

#include "Heading.h"

//! Os specific port
#include "Port/Port.h"

//! Measurements Abstractions and Utils
#include "Measurements/Measurements.h"

//! Skylake Random
#include "Utils/SRand.h"

//! Short-GUID
#include "Utils/SGUID.h"

//! String BufferStream
#include "Utils/BufferStream.h"

//! String Stream
#include "Utils/StringUtils.h"

//! Logger
#include "Utils/Logger.h"

//! Memory abstractions
#include "Memory/Memory.h"

//! Async IO API
#include "Port/AsyncIO.h"

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

//! ECS MultiArray
#include "ECS/MultiArray.h"

//! ECS DODSymmetricStore
#include "ECS/DODSymmetricStore.h"

//! ECS EntityStore
#include "ECS/EntityStore.h"

#if defined(SKL_MATH)
    //! Math
    #include "Math/Math.h"
#endif

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

    //! Is the Skylake library init and ready to use
    bool Skylake_IsTheLibraryInitialize() noexcept;

    //! Is the Skylake library init and ready to use for the calling thread
    bool Skylake_IsTheLibraryInitialize_Thread() noexcept;
}

#if defined(SKL_NO_NAMESPACE)
#ifndef SKL_NO_NAMESPACE_STATEMENT
#define SKL_NO_NAMESPACE_STATEMENT
    using namespace SKL;
#endif SKL_NO_NAMESPACE_STATEMENT
#endif