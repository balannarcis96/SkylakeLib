//!
//! \file Measurements.h
//! 
//! \brief SkylakeLib Measurements Abstractions and Utils
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    enum class EKPIValuePoints: int32_t
	{
		  Allocator_OSAllocation
		, Allocator_Pool1
		, Allocator_Pool2
		, Allocator_Pool3
		, Allocator_Pool4
		, Allocator_Pool5
		, Allocator_Pool6
		, Allocator_Pool7
		, Allocator_Pool8
		, Allocator_Pool9

		, Max
	};
}

#include "KPI.h"
