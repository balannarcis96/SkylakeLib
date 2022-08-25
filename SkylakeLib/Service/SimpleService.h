//!
//! \file SimpleService.h
//! 
//! \brief Simple service abstraction
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    class SimpleService : public IService
    {
    public:
        SimpleService( uint32_t UID ) noexcept : IService{ UID } {}

        friend ServerInstance;
    };
}    