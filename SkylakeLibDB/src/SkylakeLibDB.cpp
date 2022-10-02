//!
//! \file SkylakeLibDB.h
//! 
//! \brief MySQL c++ interface library
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 

#include "SkylakeLibDB.h"

#include <mysql.h>

namespace SKL::DB
{
    constexpr size_t CActualSizeOfMYSQL = sizeof( MYSQL );
    static_assert( CActualSizeOfMYSQL == CSizeOfMYSQL, "Update CSizeOfMYSQL!" );

    /*------------------------------------------------------------
     * DBLibGuard
     *------------------------------------------------------------*/

    DBLibGuard DBLibGuard::LibGuard{};

    DBLibGuard::DBLibGuard() noexcept
    {
        //SKLL_VER( "[DBLibGuard] MYSQL library initializing!" );
        bIsValid = 0 == ::mysql_library_init( 0, nullptr, nullptr );
    }
    DBLibGuard::~DBLibGuard() noexcept
    {
        if( true == bIsValid )
        {
            //SKLL_VER( "[DBLibGuard] MYSQL library terminating!" );
            ::mysql_library_end();
            bIsValid = false;
        }
    }
    bool DBLibGuard::IsValidLib() noexcept
    {
        return LibGuard.bIsValid;
    }

    /*------------------------------------------------------------
     * DBConnection
     *------------------------------------------------------------*/

    DBConnection::DBConnection( const DBConnectionSettings& Settings ) noexcept
        : Settings{ Settings }
    {
        SKLL_VER( "[DBConnection] DBConnection()!" );
    }
    DBConnection::~DBConnection() noexcept
    {
        SKLL_VER( "[DBConnection] ~DBConnection()!" );
        CloseConnection();
    }
    const char* DBConnection::GetStatus() noexcept
    {
        return ::mysql_stat( reinterpret_cast<MYSQL*>( &Mysql ) );
    }
    const char* DBConnection::GetLastMysqlError() noexcept
    {
        return ::mysql_error( reinterpret_cast<MYSQL*>( &Mysql ) );
    }
    bool DBConnection::Execute( const char *Query ) noexcept
    {
        if( 0 != mysql_query( reinterpret_cast<MYSQL*>( &Mysql ), Query ) ) SKL_UNLIKELY
        {
            SKLL_WRN_FMT( "[DBConnection]::Execute() -> Failed with error [%s]", GetLastMysqlError( ) );
            return false;
        }

        return true;
    }
    bool DBConnection::ExecuteUpdateQuery( const char *Query ) noexcept
    {
        const auto AcquireResult = AcquireConnection( );
        if( false == AcquireResult.IsSuccess( ) ) SKL_UNLIKELY
        {
            SKLL_WRN_FMT( "[DBConnection]::ExecuteUpdateQuery() -> Failed to acquire connection. LastMysqlError[%s]", GetLastMysqlError( ) );
            return false;
        }

        return Execute( Query );
    }
    DBConnection::AcquireResult DBConnection::AcquireConnection( bool bOpenConnectionIfClosed ) noexcept
    {
        if( false == bIsOpen ) SKL_UNLIKELY
        {
            if( true == bOpenConnectionIfClosed ) SKL_LIKELY
            {
                if( true == OpenConnection() )
                {
                    return AcquireResult{ .bHasError = false, .bHasReconnected = true };
                }
            }

            return AcquireResult{ .bHasError = true, .bHasReconnected = false };
        }

        auto& MysqlRef = *reinterpret_cast<MYSQL*>( &Mysql );

        const auto BeforeThreadId = MysqlRef.thread_id;

        if( 0 != ::mysql_ping( &MysqlRef ) ) SKL_UNLIKELY
        {
            CloseConnection();
            SKLL_WRN( "[DBConnection]::AcquireConnection() Failed to reaquire the connection!" );
            return AcquireResult{ .bHasError = true, .bHasReconnected = false };
        }

        const bool bConnectionReaquired{ BeforeThreadId != MysqlRef.thread_id };
        if( true == bConnectionReaquired ) SKL_LIKELY
        {
            SKLL_INF( "[DBConnection]::AcquireConnection() Connection reaquired!" );
        }

        return AcquireResult{ .bHasError = false, .bHasReconnected = bConnectionReaquired };
    }
    bool DBConnection::TryReacquireConnection() noexcept
    {
        uint32_t Tries{ Settings.ReacquireConnectionMaxTries };
        do
        {
            const auto AcquireResult{ AcquireConnection( ) };
            if( true == AcquireResult.IsSuccess( ) ) SKL_LIKELY
            {
                break;
            }

            --Tries;
        } while( 0 < Tries ) SKL_UNLIKELY;

        return 0 != Tries;
    }
    bool DBConnection::OpenConnection() noexcept
    {
        if( true == bIsOpen )
        {
            SKLL_VER( "[DBConnection]::OpenConnection() Already openned!" );
            return true;
        }

        if( nullptr == ::mysql_init( reinterpret_cast<MYSQL*>( &Mysql ) ) )
        {
            SKLL_WRN_FMT( "[DBConnection]::OpenConnection() Failed to init the mysql object erro[%s]!", GetLastMysqlError() );
            return false;
        }

        const uint32_t Flags{ Settings.bEnableMultistatements ? CLIENT_MULTI_STATEMENTS : 0 };
        if( const auto Result{ ::mysql_real_connect( 
              reinterpret_cast<MYSQL*>( &Mysql )
            , Settings.Host.c_str()
            , Settings.Username.c_str()
            , Settings.Password.c_str()
            , Settings.Database.c_str()
            , Settings.Port
            , nullptr
            , Flags
        ) }; nullptr == Result)
        {
            SKLL_ERR_FMT( "[DBConnection]::OpenConnection() Failed with error [%s]!", mysql_error( reinterpret_cast<MYSQL*>( &Mysql ) ) );
            return false;
        }

        if( false == SetOptions() )
        {
            ::mysql_close( reinterpret_cast<MYSQL*>( &Mysql ) );
            Mysql.Reset();
            return false;
        }

        bIsOpen = true;

        SKLL_VER_FMT( "[DBConnection]::OpenConnection() Successfully openned connection to DB[%s]!", Settings.Database.c_str() );

        return true;
    }
    void DBConnection::CloseConnection() noexcept
    {
        ::mysql_close( reinterpret_cast<MYSQL*>( &Mysql ) );
        Mysql.Reset();
        bIsOpen = false;
        SKLL_VER_FMT( "[DBConnection]::CloseConnection() Closed connection to DB[%s]!", Settings.Database.c_str() );
    }
    bool DBConnection::SetOptions() noexcept
    {
        const int32_t ProtoType{ MYSQL_PROTOCOL_TCP };
        if( 0 != ::mysql_options( reinterpret_cast<MYSQL*>( &Mysql ), MYSQL_OPT_PROTOCOL, &ProtoType ) )
        {
            SKLL_WRN_FMT( "[DBConnection]::SetOptions() Failed to set protocol! DB[%s]", Settings.Database.c_str() );
            return false;
        }

        // Calls to mysql_ping will re establish a connection if needed. see mysql_ping()
        BOOL Reconnect{ TRUE };
        if( 0 != ::mysql_options( reinterpret_cast<MYSQL*>( &Mysql ), MYSQL_OPT_RECONNECT, &Reconnect ) )
        {
            SKLL_WRN_FMT( "[DBConnection]::SetOptions() Failed to set reconnect! DB[%s]", Settings.Database.c_str() );
            return false;
        }

        /*if ( 0 != mysql_options( &Mysql, MYSQL_OPT_CONNECT_TIMEOUT, &ConnectionTimeout ) ) 
        {
            return false;
        }*/

        if( TRUE == mysql_autocommit( reinterpret_cast<MYSQL*>( &Mysql ), Settings.bAutocommit ) )
        {
            SKLL_WRN_FMT( "[DBConnection]::SetOptions() Failed to set autocomit to %s! DB[%s]", Settings.bAutocommit ? "true" : "false" , Settings.Database.c_str() );
            return false;
        }

        if( 0 != ::mysql_set_character_set( reinterpret_cast<MYSQL*>( &Mysql ), "utf8" ) )
        {
            SKLL_WRN_FMT( "[DBConnection]::SetOptions() Failed to set mysql client charset to utf8! DB[%s]", Settings.Database.c_str() );
            return false;
        }

        // client sends data in UTF8, so MySQL must expect UTF8 too
        if( false == Execute( "SET NAMES `utf8`" ) )
        {
            SKLL_WRN_FMT( "[DBConnection]::SetOptions() Failed to [SET NAMES `utf8]! DB[%s]", Settings.Database.c_str() );
            return false;
        }
        if( false == Execute( "SET CHARACTER SET `utf8`" ) )
        {
            SKLL_WRN_FMT( "[DBConnection]::SetOptions() Failed to [SET CHARACTER SET `utf8`]! DB[%s]", Settings.Database.c_str() );
            return false;
        }

        return true;
    }

    /*------------------------------------------------------------
     * DBConnectionFactory
     *------------------------------------------------------------*/

    DBConnection DBConnectionFactory::TryOpenNewConnection() noexcept
    {
        SKL_ASSERT( true == Settings.IsValid() );

        DBConnection NewConnection{ Settings };

        if( false == NewConnection.OpenConnection() ) SKL_UNLIKELY
        {
            return DBConnection{ DBConnectionSettings{} };
        }

        return NewConnection;
    }
}