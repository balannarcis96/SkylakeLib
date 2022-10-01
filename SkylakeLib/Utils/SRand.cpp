//!
//! \file SRand.cpp
//! 
//! \brief Skylake Random abstractions
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#include "SkylakeLib.h"

namespace SKL
{
    SpinLock      GRand::Lock{};
    Squirrel3Rand GRand::Rand{};
}