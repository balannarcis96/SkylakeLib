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

//! Forward declarations
namespace SKL
{
    struct IAsyncIOTask;
}

//! Std
#include "Std/Std.h"

//! Tuning
#include "Tuning/Tuning.h"

//! Logging
#include "Diagnostics/Log.h"

//! Platform
#include "Platform/Platform.h"

//! Thread Local Storage
#include "ThreadLocalStorage/TLS.h"

//! Spin Lock
#include "Utils/SpinLock.h"

//! Memory abstractions
#include "Memory/Memory.h"

//! Task
#include "Task/Task.h"

//! Threading
#include "Threading/Threading.h"

//! Networking
#include "Networking/Networking.h"

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
