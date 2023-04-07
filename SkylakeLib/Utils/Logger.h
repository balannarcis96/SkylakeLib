//!
//! \file Logger.h
//! 
//! \brief Fast and lightweight logger abstraction for SkylakeLib
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

#include <SkylakeLibStandalone.h>

#include <fmt/format.h>

namespace SKL
{
    class ILoggerImpl
    {
    public:
        virtual ~ILoggerImpl() noexcept = default;
    };

    class SkylakeLogger
    {
    public:
        SkylakeLogger() noexcept = default;




    private:
        ILoggerImpl* Impl{ nullptr };
    };
}
