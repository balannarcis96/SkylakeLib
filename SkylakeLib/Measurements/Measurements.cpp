//!
//! \file Measurements.cpp
//! 
//! \brief SkylakeLib Measurements Abstractions and Utils
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#include "SkylakeLib.h"

namespace SKL
{
    static_assert( ( CKPIPointsToAverageFrom & ( CKPIPointsToAverageFrom - 1 ) ) == 0 );
}

namespace SKL
{
    RStatus KPIContext::Initialize() noexcept 
	{ 
		return RSuccess; 
	}
}

namespace SKL
{
    RStatus CountersContext::Initialize() noexcept 
	{ 
		return RSuccess; 
	}
}
