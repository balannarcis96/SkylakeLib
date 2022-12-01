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

//! ASD (Advanced Single Dispatch)
#include "AdvancedSingleDispatch.h"
#include "ASDForward.h"

//! Macros
#include "Macros.h"

//! Result Status abstraction
#include "RStatus.h"

//! Base Types
#include "SkylakeLibTypes.h"

#if !defined(SKL_BUILD_SHIPPING)
#include "DebugTrap.h"
#define SKL_BREAK() psnip_trap()
#else
#define SKL_BREAK() 
#endif

//! Assert 
#include "SkylakeAssert.h"

//! Atomic value
#include "AtomicValue.h"

//! Stream
#include "Stream.h"

//! EntityId
#include "EntityId.h"

//! Packet
#include "Packet.h"

//! Packet Builder
#include "PacketBuilder.h"

namespace SKL
{
    SKL_FORCEINLINE SKL_NODISCARD TEpochTimePoint GetCurrentEpochTime() noexcept
    {
        return static_cast<TEpochTimePoint>( std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::system_clock::now().time_since_epoch() ).count() );
    }
}

#if defined(SKL_NO_NAMESPACE)
#ifndef SKL_NO_NAMESPACE_STATEMENT
#define SKL_NO_NAMESPACE_STATEMENT
    using namespace SKL;
#endif SKL_NO_NAMESPACE_STATEMENT
#endif
