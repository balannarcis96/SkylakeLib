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
    static_assert( sizeof( DBTimeBase ) == sizeof( MYSQL_TIME ) );

    static_assert( static_cast<int32_t>( ETimestampType::TIMESTAMP_NONE ) == static_cast<int32_t>( MYSQL_TIMESTAMP_NONE ) );
    static_assert( static_cast<int32_t>( ETimestampType::TIMESTAMP_ERROR ) == static_cast<int32_t>( MYSQL_TIMESTAMP_ERROR ) );
    static_assert( static_cast<int32_t>( ETimestampType::TIMESTAMP_DATE ) == static_cast<int32_t>( MYSQL_TIMESTAMP_DATE ) );
    static_assert( static_cast<int32_t>( ETimestampType::TIMESTAMP_DATETIME ) == static_cast<int32_t>( MYSQL_TIMESTAMP_DATETIME ) );
    static_assert( static_cast<int32_t>( ETimestampType::TIMESTAMP_TIME ) == static_cast<int32_t>( MYSQL_TIMESTAMP_TIME ) );
    static_assert( static_cast<int32_t>( ETimestampType::TIMESTAMP_DATETIME_TZ ) == static_cast<int32_t>( MYSQL_TIMESTAMP_DATETIME_TZ ) );

    /*------------------------------------------------------------
     * DBLibGuard
     *------------------------------------------------------------*/

    DBLibGuard DBLibGuard::LibGuard{};

    DBLibGuard::DBLibGuard() noexcept
    {
        //GLOG_DEBUG( "[DBLibGuard] MYSQL library initializing!" );
        bIsValid = 0 == ::mysql_library_init( 0, nullptr, nullptr );
    }
    DBLibGuard::~DBLibGuard() noexcept
    {
        if( true == bIsValid )
        {
            //GLOG_DEBUG( "[DBLibGuard] MYSQL library terminating!" );
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
        Mysql.Reset();
    }
    DBConnection::~DBConnection() noexcept
    {
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
    int64_t DBConnection::Execute( const char *Query ) noexcept
    {
        bool bConnectionReqauiredOnce{ false };
DBConnection_Execute_Start:
        if( 0 != ::mysql_query( reinterpret_cast<::MYSQL*>( &Mysql ), Query ) ) SKL_UNLIKELY
        {
            const auto LastError{ ::mysql_errno( reinterpret_cast<::MYSQL*>( &Mysql ) ) };
            if( false == bConnectionReqauiredOnce && ( CR_SERVER_LOST == LastError || CR_SERVER_GONE_ERROR == LastError ) )
            {
                const AcquireResult ReaquireResult{ TryReacquireConnection() };
                if( ReaquireResult.IsSuccess() )
                {
                    bConnectionReqauiredOnce = true;
                    goto DBConnection_Execute_Start;
                }
                else
                {
                    GLOG_DEBUG( "Failed to reaquire mysql connection" );
                }
            }

            SKLL_DEBUG_BLOCK(
            {
                const char* MysqlErrorString{ GetLastMysqlError() };
                GLOG_ERROR( "MysqlError: %s!", MysqlErrorString );
            } );
  
            return -1;
        }
        
        int64_t NoOfRowsAffected{ 0 };
        ::MYSQL_RES* Result{ ::mysql_store_result( reinterpret_cast<::MYSQL*>( &Mysql ) ) };
        if ( nullptr != Result ) SKL_UNLIKELY
        {
            GTRACE_DEBUG( "Do not use this function for SELECT data queries!" );
            const auto Temp{ ::mysql_num_rows( Result ) };
            SKL_ASSERT( Temp <= INT64_MAX );
            NoOfRowsAffected = static_cast<int64_t>( Temp );
            ::mysql_free_result( Result );
        }
        else
        {
            NoOfRowsAffected = ::mysql_field_count( reinterpret_cast<::MYSQL*>( &Mysql ) );
            if( NoOfRowsAffected == 0 ) SKL_ALLWAYS_LIKELY 
            {
                const auto Temp{ ::mysql_affected_rows( reinterpret_cast<::MYSQL*>( &Mysql ) ) };
                SKL_ASSERT( Temp <= INT64_MAX );
                NoOfRowsAffected = static_cast<int64_t>( Temp );
            }
            else
            {
                GTRACE_DEBUG( "mysql_store_result() should have returned data! MysqlErr:%s", GetLastMysqlError() );
                return -1;
            }
        }

        return NoOfRowsAffected;
    }
    int64_t DBConnection::Execute( const char *Query, uint32_t InQueryLength ) noexcept
    {
        bool bConnectionReqauiredOnce{ false };
DBConnection_Execute_Start:
        if( 0 != ::mysql_real_query( reinterpret_cast<::MYSQL*>( &Mysql ), Query, InQueryLength ) ) SKL_UNLIKELY
        {
            const auto LastError{ ::mysql_errno( reinterpret_cast<::MYSQL*>( &Mysql ) ) };
            if( false == bConnectionReqauiredOnce && ( CR_SERVER_LOST == LastError || CR_SERVER_GONE_ERROR == LastError ) )
            {
                const AcquireResult ReaquireResult{ TryReacquireConnection() };
                if( ReaquireResult.IsSuccess() )
                {
                    bConnectionReqauiredOnce = true;
                    goto DBConnection_Execute_Start;
                }
                else
                {
                    GLOG_DEBUG( "Failed to reaquire mysql connection" );
                }
            }

            SKLL_DEBUG_BLOCK(
            {
                const char* MysqlErrorString{ GetLastMysqlError() };
                GLOG_DEBUG( "MysqlError: %s!", MysqlErrorString );
            } );
  
            return -1;
        }
        
        int64_t NoOfRowsAffected{ 0 };
        ::MYSQL_RES* Result{ ::mysql_store_result( reinterpret_cast<::MYSQL*>( &Mysql ) ) };
        if ( nullptr != Result ) SKL_UNLIKELY
        {
            GLOG_DEBUG( "Do not use this function for SELECT data queries!" );
            const auto Temp{ ::mysql_num_rows( Result ) };
            SKL_ASSERT( Temp <= INT64_MAX );
            NoOfRowsAffected = static_cast<int64_t>( Temp );
            ::mysql_free_result( Result );
        }
        else
        {
            NoOfRowsAffected = ::mysql_field_count( reinterpret_cast<::MYSQL*>( &Mysql ) );
            if( NoOfRowsAffected == 0 ) SKL_ALLWAYS_LIKELY 
            {
                const auto Temp{ ::mysql_affected_rows( reinterpret_cast<::MYSQL*>( &Mysql ) ) };
                SKL_ASSERT( Temp <= INT64_MAX );
                NoOfRowsAffected = static_cast<int64_t>( Temp );
            }
            else
            {
                GLOG_DEBUG( "mysql_store_result() should have returned data! MysqlErr:%s", GetLastMysqlError() );
                return -1;
            }
        }

        return NoOfRowsAffected;
    }
    bool DBConnection::Ping() noexcept
    {
        const auto PingResult{ ::mysql_ping( reinterpret_cast<::MYSQL*>( &Mysql ) ) };
        if( 0 != PingResult ) SKL_LIKELY
        {
            SKLL_DEBUG_BLOCK(
            {
                const char* MysqlErrorString{ GetLastMysqlError() };
                GLOG_DEBUG( "MysqlError: %s!", MysqlErrorString ); 
            } );
        }

        return 0 == PingResult;
    }

    DBConnection::AcquireResult DBConnection::TryReacquireConnection() noexcept
    {
        uint32_t Tries{ Settings.ReacquireConnectionMaxTries };
        do
        {
            const auto ThreadIdBefore{ ::mysql_thread_id( reinterpret_cast<::MYSQL*>( &Mysql ) ) };
            const auto PingResult{ ::mysql_ping( reinterpret_cast<::MYSQL*>( &Mysql ) ) };
            if( 0 == PingResult ) SKL_LIKELY
            {
                const auto ThreadIdAfter{ ::mysql_thread_id( reinterpret_cast<::MYSQL*>( &Mysql ) ) };
                return AcquireResult{ 
                    .bHasError       = false,
                    .bHasReconnected = ThreadIdAfter != ThreadIdBefore
                };
            }

            --Tries;
        } while( 0 < Tries );

        return AcquireResult{
            .bHasError       = true,
            .bHasReconnected = false
        };
    }
    bool DBConnection::OpenConnection() noexcept
    {
        if( true == bIsOpen )
        {
            GLOG_DEBUG( "[DBConnection]::OpenConnection() Already openned!" );
            return true;
        }

        if( nullptr == ::mysql_init( reinterpret_cast<MYSQL*>( &Mysql ) ) )
        {
            GLOG_ERROR( "[DBConnection]::OpenConnection() Failed to init the mysql object erro[%s]!", GetLastMysqlError() );
            return false;
        }

        const uint32_t Flags{/* Settings.bEnableMultistatements ? CLIENT_MULTI_STATEMENTS : 0*/ 0 };
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
            GLOG_ERROR( "[DBConnection]::OpenConnection() Failed with error [%s]!", mysql_error( reinterpret_cast<MYSQL*>( &Mysql ) ) );
            return false;
        }

        if( false == SetOptions() )
        {
            ::mysql_close( reinterpret_cast<MYSQL*>( &Mysql ) );
            Mysql.Reset();
            return false;
        }

        bIsOpen = true;

        GLOG_INFO( "[DBConnection]::OpenConnection() Successfully openned connection to DB[%s]!", Settings.Database.c_str() );

        return true;
    }
    void DBConnection::CloseConnection() noexcept
    {
        if( bIsOpen )
        {
            GLOG_INFO( "[DBConnection]::CloseConnection() Closed connection to DB[%s]!", Settings.Database.c_str() );
        }

        ::mysql_close( reinterpret_cast<MYSQL*>( &Mysql ) );
        Mysql.Reset();
        bIsOpen = false;
        bIsTransactionStarted = false;
    }
    bool DBConnection::SetOptions() noexcept
    {
        const int32_t ProtoType{ MYSQL_PROTOCOL_TCP };
        if( 0 != ::mysql_options( reinterpret_cast<MYSQL*>( &Mysql ), MYSQL_OPT_PROTOCOL, &ProtoType ) )
        {
            GLOG_ERROR( "[DBConnection]::SetOptions() Failed to set protocol! DB[%s]", Settings.Database.c_str() );
            return false;
        }

        // Calls to mysql_ping will re establish a connection if needed. see mysql_ping()
        BOOL Reconnect{ TRUE };
        if( 0 != ::mysql_options( reinterpret_cast<MYSQL*>( &Mysql ), MYSQL_OPT_RECONNECT, &Reconnect ) )
        {
            GLOG_ERROR( "[DBConnection]::SetOptions() Failed to set reconnect! DB[%s]", Settings.Database.c_str() );
            return false;
        }
        
        /*if ( 0 != mysql_options( &Mysql, MYSQL_OPT_CONNECT_TIMEOUT, &ConnectionTimeout ) ) 
        {
            return false;
        }*/

#if defined(SKL_MYSQL_COMPRESS_NET)
        if ( 0 != mysql_options( reinterpret_cast<MYSQL*>( &Mysql ), MYSQL_OPT_COMPRESS, nullptr ) ) 
        {
            return false;
        }
#endif

        if( TRUE == ::mysql_autocommit( reinterpret_cast<MYSQL*>( &Mysql ), Settings.bAutocommit ) )
        {
            GLOG_ERROR( "[DBConnection]::SetOptions() Failed to set autocomit to %s! DB[%s]", Settings.bAutocommit ? "true" : "false" , Settings.Database.c_str() );
            return false;
        }

        if( 0 != ::mysql_set_character_set( reinterpret_cast<MYSQL*>( &Mysql ), "utf8" ) )
        {
            GLOG_ERROR( "[DBConnection]::SetOptions() Failed to set mysql client charset to utf8! DB[%s]", Settings.Database.c_str() );
            return false;
        }

        // client sends data in UTF8, so MySQL must expect UTF8 too
        if( -1 == Execute( "SET NAMES `utf8`" ) )
        {
            GLOG_ERROR( "[DBConnection]::SetOptions() Failed to [SET NAMES `utf8]! DB[%s]", Settings.Database.c_str() );
            return false;
        }
        if( -1 == Execute( "SET CHARACTER SET `utf8`" ) )
        {
            GLOG_ERROR( "[DBConnection]::SetOptions() Failed to [SET CHARACTER SET `utf8`]! DB[%s]", Settings.Database.c_str() );
            return false;
        }

        return true;
    }

    /*------------------------------------------------------------
     * DBConnectionFactory
     *------------------------------------------------------------*/

    std::unique_ptr<DBConnection> DBConnectionFactory::TryOpenNewConnection() const noexcept
    {
        SKL_ASSERT( true == Settings.IsValid() );

        std::unique_ptr<DBConnection> NewConnection{ new DBConnection( Settings ) };

        if( false == NewConnection->OpenConnection() ) SKL_UNLIKELY
        {
            return nullptr;
        }

        return NewConnection;
    }
    
    /*------------------------------------------------------------
     * Parameter
     *------------------------------------------------------------*/
    void Parameter::Reset( void* InBuffer ) noexcept
    {
        SKL_ASSERT( nullptr != InBuffer );

        Bind.Zero();

        ::MYSQL_BIND& MysqlBind{ reinterpret_cast<::MYSQL_BIND&>( Bind ) };
        MysqlBind.buffer        = InBuffer;
        MysqlBind.is_null_value = InBuffer == nullptr;
    }

    void Parameter::Reset( void* InBuffer, uint32_t InBufferLength ) noexcept
    {
        SKL_ASSERT( InBufferLength == 0 || nullptr != InBuffer );

        Bind.Zero();

        ::MYSQL_BIND& MysqlBind{ reinterpret_cast<::MYSQL_BIND&>( Bind ) };
        MysqlBind.buffer        = InBuffer;
        MysqlBind.buffer_length = InBufferLength;
        MysqlBind.is_null_value = InBuffer == nullptr;
    }

    void Parameter::Reset( void* InBuffer, uint32_t InBufferLength, EFieldType InType, bool bIsUnsigned ) noexcept
    {
        SKL_ASSERT( InBufferLength == 0 || nullptr != InBuffer );

        Bind.Zero();

        auto& MysqlBind{ reinterpret_cast<::MYSQL_BIND&>( Bind ) };
        MysqlBind.buffer        = InBuffer;
        MysqlBind.buffer_length = InBufferLength;
        MysqlBind.is_null_value = InBuffer == nullptr;
        MysqlBind.buffer_type   = static_cast<enum_field_types>( InType );
        MysqlBind.is_unsigned   = bIsUnsigned;
    }

    void Parameter::Reset( void* InBuffer, uint32_t InBufferLength, uint32_t* OutFieldLengthDestiantion, EFieldType InType, bool bIsUnsigned ) noexcept
    {
        Bind.Zero();

        auto& MysqlBind{ reinterpret_cast<::MYSQL_BIND&>( Bind ) };
        MysqlBind.buffer        = InBuffer;
        MysqlBind.buffer_length = InBufferLength;
        MysqlBind.is_null_value = InBuffer == nullptr;
        MysqlBind.buffer_type   = static_cast<enum_field_types>( InType );
        MysqlBind.is_unsigned   = bIsUnsigned;
        MysqlBind.length        = reinterpret_cast<unsigned long*>( OutFieldLengthDestiantion );
    }

    void Parameter::SetType( EFieldType InType, bool bIsUnsigned ) noexcept
    {
        auto& MysqlBind{ reinterpret_cast<::MYSQL_BIND&>( Bind ) };
        MysqlBind.buffer_type = static_cast<enum_field_types>( InType );
        MysqlBind.is_unsigned = bIsUnsigned;
    }
}
