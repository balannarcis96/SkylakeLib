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

//! Platform
#include "Platform/Platform.h"

//! Thread Local Storage
#include "ThreadLocalStorage/TLS.h"

//! Task
#include "Task/Task.h"

//! Threading
#include "Threading/Threading.h"

//! Memory abstractions
#include "Memory/Memory.h"

namespace SKL
{
    //!
    //! \brief Initialize the SkylakeLibrary
    //!
    RStatus InitializeLibrary( int32_t Argc, char** Argv, FILE* InLogOutput ) noexcept;

    //!
    //! \brief Terminate the SkylakeLibrary
    //!
    RStatus TerminateLibrary() noexcept;

    //!
    //! \brief Initialize the SkylakeLibrary per thread
    //!
    RStatus InitializeLibrary_Thread() noexcept;

    //!
    //! \brief Terminate the SkylakeLibrary per thread
    //!
    RStatus TerminateLibrary_Thread() noexcept;
}
