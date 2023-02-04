//!
//! \file StdDependencies.h
//! 
//! \brief Std library dependencies
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//!
#pragma once

using TString = std::wstring;

static_assert( sizeof( int ) == sizeof( long ), "Unsupported platform!" );
