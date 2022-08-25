//!
//! \file ActiveService.h
//! 
//! \brief Active and AOD interfaced service
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    class ActiveService : public AODService
    {
    public:
        ActiveService( uint32_t UID ) noexcept : AODService{ UID } {}

    protected:
        //! [Callback] Each time this service is ticked
        virtual void OnTick() noexcept = 0;

        friend ServerInstance;
    };
}    