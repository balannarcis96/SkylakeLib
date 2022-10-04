//!
//! \file DBConnectionFactory.h
//! 
//! \brief MySQL c++ interface library
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL::DB
{
    struct DBConnectionFactory
    {
        bool Initialize( DBConnectionSettings&& InSettings ) noexcept
        {
            if( false == InSettings.Validate() )
            {
                SKLL_WRN( "[DBConnectionFactory]::Initialize() Invalid settings!" );
                return false;
            }

            Settings = std::forward<DBConnectionSettings>( InSettings );

            return true;
        }
        
        std::unique_ptr<DBConnection> TryOpenNewConnection() noexcept;

    private:
        DBConnectionSettings Settings{};
    };
}