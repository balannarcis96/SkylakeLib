//!
//! \file SkylakeLibStandalone.cpp
//! 
//! \brief Standalone part of SkylakeLib
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#include "SkylakeLibStandalone.h"

namespace SKL
{
    thread_local std::array<char, 4098> GSklAssertWorkBuffer{};

    #if defined(SKL_USE_SERIALIZED_LOGGER)
    thread_local BinaryStream SerializedSkylakeLogger::WorkingStream{ std::numeric_limits<uint16_t>::max() - 1024U };
    #endif
}
