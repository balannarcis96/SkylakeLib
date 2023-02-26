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
}
