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
#ifndef SKL_DB_LIB_STATEMENT_MAX_INPUT_PARAMS
    #define SKL_DB_LIB_STATEMENT_MAX_INPUT_PARAMS 128
#endif

#ifndef SKL_DB_LIB_STATEMENT_MAX_OUTPUT_PARAMS
    #define SKL_DB_LIB_STATEMENT_MAX_OUTPUT_PARAMS 128
#endif

#ifndef SKL_DB_LIB_STATEMENT_QUERY_MAX_LENGTH
    #define SKL_DB_LIB_STATEMENT_QUERY_MAX_LENGTH 4096
#endif

    constexpr size_t CDBStatementMaxInputParams = SKL_DB_LIB_STATEMENT_MAX_INPUT_PARAMS;
    constexpr size_t CDBStatementMaxOutputParams = SKL_DB_LIB_STATEMENT_MAX_OUTPUT_PARAMS;
    constexpr uint32_t CDBStatementQueryMaxLength = SKL_DB_LIB_STATEMENT_QUERY_MAX_LENGTH;

    enum class EFieldType : int32_t
    { 
        TYPE_DECIMAL,
        TYPE_TINY,
        TYPE_SHORT,
        TYPE_LONG,
        TYPE_FLOAT,
        TYPE_DOUBLE,
        TYPE_NULL,
        TYPE_TIMESTAMP,
        TYPE_LONGLONG,
        TYPE_INT24,
        TYPE_DATE,
        TYPE_TIME,
        TYPE_DATETIME,
        TYPE_YEAR,
        TYPE_NEWDATE, /**< Internal to MySQL. Not used in protocol */
        TYPE_VARCHAR,
        TYPE_BIT,
        TYPE_TIMESTAMP2,
        TYPE_DATETIME2,   /**< Internal to MySQL. Not used in protocol */
        TYPE_TIME2,       /**< Internal to MySQL. Not used in protocol */
        TYPE_TYPED_ARRAY, /**< Used for replication only */
        TYPE_JSON = 245,
        TYPE_NEWDECIMAL = 246,
        TYPE_ENUM = 247,
        TYPE_SET = 248,
        TYPE_TINY_BLOB = 249,
        TYPE_MEDIUM_BLOB = 250,
        TYPE_LONG_BLOB = 251,
        TYPE_BLOB = 252,
        TYPE_VAR_STRING = 253,
        TYPE_STRING = 254,
        TYPE_GEOMETRY = 255 
    };

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
    
    constexpr size_t CMysqlStmtSize = 696;
    struct MysqlStmtOpaque
    {
        MysqlStmtOpaque() noexcept { Zero(); }
        SKL_FORCEINLINE void Zero() noexcept { memset( Body,0 , CMysqlStmtSize ); }
        uint8_t Body[CMysqlStmtSize];
    };

    constexpr size_t CMysqlBindSize = 104;
    struct MysqlBindOpaue
    {
        MysqlBindOpaue() noexcept { Zero(); }
        SKL_FORCEINLINE void Zero() noexcept { memset( Body,0 , CMysqlBindSize ); }
        uint8_t Body[CMysqlBindSize];
    };

    constexpr size_t CMysqlResSize = 104;
    struct MysqlResOpaue
    {
        MysqlResOpaue() noexcept { Zero(); }
        SKL_FORCEINLINE void Zero() noexcept { memset( Body,0 , CMysqlResSize ); }
        uint8_t Body[CMysqlResSize];
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
        bool        bAutocommit                 { true };
        bool        bIsValid                    { false };

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
#include "../Private/DBTime.h"
#include "../Private/DBConnection.h"
#include "../Private/DBConnectionFactory.h"
#include "../Private/DBStatement.h"
//#include "../Private/DBStatementField.h"
//#include "../Private/DBStaticStatement.h"
#include "../Private/DBTransaction.h"