//!
//! \file SkylakeLibHeaderOnly.h
//! 
//! \brief All in one SkylakeLibHeaderOnly
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

//! Flags
#include "Flags.h"

//! std dependencies
#include "Std.h"

//! Concepts
#include "Concepts.h"

//! ASD (Advanced Single Dispatch)
#include "AdvancedSingleDispatch.h"
#include "ASDForward.h"

//! Macros
#include "Macros.h"

//! Result Status abstraction
#include "RStatus.h"

//! Base Types
#include "Types.h"

//! Assert 
#include "SkylakeAssert.h"

#if !defined(SKL_BUILD_SHIPPING)
#include "DebugTrap.h"
#define SKL_BREAK() psnip_trap()
#else
#define SKL_BREAK() 
#endif

