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
    
    extern FILE* GLogOutput; //!< File handler for the log

    #define SKL_INF( InString ) fprintf( GLogOutput, "[SkylakeLib::INF]" InString )
    #define SKL_WRN( InString ) fprintf( GLogOutput, "[SkylakeLib::WRN]" InString )
    #define SKL_ERR( InString ) fprintf( GLogOutput, "[SkylakeLib::ERR]" InString )

    #define SKL_INF_FMT( InFormatString, ... ) fprintf( GLogOutput, "[SkylakeLib::INF]" InFormatString, __VA_ARGS__ )
    #define SKL_WRN_FMT( InFormatString, ... ) fprintf( GLogOutput, "[SkylakeLib::WRN]" InFormatString, __VA_ARGS__ )
    #define SKL_ERR_FMT( InFormatString, ... ) fprintf( GLogOutput, "[SkylakeLib::ERR]" InFormatString, __VA_ARGS__ )
}
