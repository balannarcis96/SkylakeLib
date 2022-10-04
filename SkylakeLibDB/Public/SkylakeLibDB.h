//!
//! \file SkylakeLibDB.h
//! 
//! \brief MySQL c++ interface library
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

#include <SkylakeLib.h>

namespace SKL::DB
{
    constexpr size_t CSizeOfMYSQL = 1096;
    struct alignas( SKL_ALIGNMENT ) MYSQL_Opaque
    {
        MYSQL_Opaque() noexcept { ::memset( Buffer, CSizeOfMYSQL, 0 ); }
        ~MYSQL_Opaque() = default;
        
        MYSQL_Opaque( const MYSQL_Opaque& Other ) = delete;
        MYSQL_Opaque& operator=( const MYSQL_Opaque& Other ) = delete;

        MYSQL_Opaque( MYSQL_Opaque&& Other ) noexcept
        {
            ::memcpy( Buffer, Other.Buffer, CSizeOfMYSQL );
            ::memset( Other.Buffer, 0, CSizeOfMYSQL );
        }
        MYSQL_Opaque& operator=( MYSQL_Opaque&& Other ) noexcept
        {
            SKL_ASSERT( this != &Other );
            ::memcpy( Buffer, Other.Buffer, CSizeOfMYSQL );
            ::memset( Other.Buffer, 0, CSizeOfMYSQL );
        }

        SKL_FORCEINLINE void Reset() noexcept
        {
            ::memset( Buffer, 0, CSizeOfMYSQL );
        }

        uint8_t Buffer[ CSizeOfMYSQL ];
    };

    struct DBLibGuard
    {
        DBLibGuard() noexcept;
        ~DBLibGuard() noexcept;

        SKL_NODISCARD static bool IsValidLib() noexcept;

    private:
        bool bIsValid{ false };
        static DBLibGuard LibGuard;
    };

    struct DBConnectionSettings
    {
        std::string Username                    {};
        std::string Password                    {};
        std::string Database                    {};
        std::string Host                        {};
        uint32_t    Port                        { 0 };
        uint32_t    ReacquireConnectionMaxTries { 3 };
        uint32_t    ConnectionTimeoutMs         { 500 };
        bool        bEnableMultistatements      { false };
        bool        bIsValid                    { false };
        bool        bAutocommit                 { true };

        bool Validate() noexcept
        {
            if( true == Database.empty() 
               || true == Host.empty() 
               || 0 == Port
            )
            {
                bIsValid = false;
            }
            else
            {
                bIsValid = true;
            }

            return bIsValid;
        }
        SKL_FORCEINLINE bool IsValid() const noexcept { return bIsValid; }
    };

    struct DBConnection;
    struct DBConnectionFactory;
}

#include "../Private/DBString.h"
#include "../Private/DBConnection.h"
#include "../Private/DBConnectionFactory.h"
#include "../Private/DBStatement.h"
#include "../Private/DbTransaction.h"