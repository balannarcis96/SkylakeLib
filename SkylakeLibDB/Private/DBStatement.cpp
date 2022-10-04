//!
//! \file DBStatement.h
//! 
//! \brief MySQL c++ interface library
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#include "SkylakeLibDB.h"

#include <mysql.h>

namespace SKL::DB
{
    constexpr auto t = sizeof( ::MYSQL_RES );
    static_assert( CMysqlStmtSize == sizeof( ::MYSQL_STMT ) );
    static_assert( CMysqlBindSize == sizeof( ::MYSQL_BIND ) );
    static_assert( sizeof( uint32_t ) == sizeof( unsigned long ) );

    static_assert( static_cast<size_t>( EFieldType::TYPE_DECIMAL     ) == static_cast<size_t>( ::MYSQL_TYPE_DECIMAL     ) );
    static_assert( static_cast<size_t>( EFieldType::TYPE_TINY        ) == static_cast<size_t>( ::MYSQL_TYPE_TINY        ) );
    static_assert( static_cast<size_t>( EFieldType::TYPE_SHORT       ) == static_cast<size_t>( ::MYSQL_TYPE_SHORT       ) );
    static_assert( static_cast<size_t>( EFieldType::TYPE_LONG        ) == static_cast<size_t>( ::MYSQL_TYPE_LONG        ) );
    static_assert( static_cast<size_t>( EFieldType::TYPE_FLOAT       ) == static_cast<size_t>( ::MYSQL_TYPE_FLOAT       ) );
    static_assert( static_cast<size_t>( EFieldType::TYPE_DOUBLE      ) == static_cast<size_t>( ::MYSQL_TYPE_DOUBLE      ) );
    static_assert( static_cast<size_t>( EFieldType::TYPE_NULL        ) == static_cast<size_t>( ::MYSQL_TYPE_NULL        ) );
    static_assert( static_cast<size_t>( EFieldType::TYPE_TIMESTAMP   ) == static_cast<size_t>( ::MYSQL_TYPE_TIMESTAMP   ) );
    static_assert( static_cast<size_t>( EFieldType::TYPE_LONGLONG    ) == static_cast<size_t>( ::MYSQL_TYPE_LONGLONG    ) );
    static_assert( static_cast<size_t>( EFieldType::TYPE_INT24       ) == static_cast<size_t>( ::MYSQL_TYPE_INT24       ) );
    static_assert( static_cast<size_t>( EFieldType::TYPE_DATE        ) == static_cast<size_t>( ::MYSQL_TYPE_DATE        ) );
    static_assert( static_cast<size_t>( EFieldType::TYPE_TIME        ) == static_cast<size_t>( ::MYSQL_TYPE_TIME        ) );
    static_assert( static_cast<size_t>( EFieldType::TYPE_DATETIME    ) == static_cast<size_t>( ::MYSQL_TYPE_DATETIME    ) );
    static_assert( static_cast<size_t>( EFieldType::TYPE_YEAR        ) == static_cast<size_t>( ::MYSQL_TYPE_YEAR        ) );
    static_assert( static_cast<size_t>( EFieldType::TYPE_NEWDATE     ) == static_cast<size_t>( ::MYSQL_TYPE_NEWDATE     ) );
    static_assert( static_cast<size_t>( EFieldType::TYPE_VARCHAR     ) == static_cast<size_t>( ::MYSQL_TYPE_VARCHAR     ) );
    static_assert( static_cast<size_t>( EFieldType::TYPE_BIT         ) == static_cast<size_t>( ::MYSQL_TYPE_BIT         ) );
    static_assert( static_cast<size_t>( EFieldType::TYPE_TIMESTAMP2  ) == static_cast<size_t>( ::MYSQL_TYPE_TIMESTAMP2  ) );
    static_assert( static_cast<size_t>( EFieldType::TYPE_DATETIME2   ) == static_cast<size_t>( ::MYSQL_TYPE_DATETIME2   ) );
    static_assert( static_cast<size_t>( EFieldType::TYPE_TIME2       ) == static_cast<size_t>( ::MYSQL_TYPE_TIME2       ) );
    static_assert( static_cast<size_t>( EFieldType::TYPE_TYPED_ARRAY ) == static_cast<size_t>( ::MYSQL_TYPE_TYPED_ARRAY ) );
    static_assert( static_cast<size_t>( EFieldType::TYPE_JSON        ) == static_cast<size_t>( ::MYSQL_TYPE_JSON        ) );
    static_assert( static_cast<size_t>( EFieldType::TYPE_NEWDECIMAL  ) == static_cast<size_t>( ::MYSQL_TYPE_NEWDECIMAL  ) );
    static_assert( static_cast<size_t>( EFieldType::TYPE_ENUM        ) == static_cast<size_t>( ::MYSQL_TYPE_ENUM        ) );
    static_assert( static_cast<size_t>( EFieldType::TYPE_SET         ) == static_cast<size_t>( ::MYSQL_TYPE_SET         ) );
    static_assert( static_cast<size_t>( EFieldType::TYPE_TINY_BLOB   ) == static_cast<size_t>( ::MYSQL_TYPE_TINY_BLOB   ) );
    static_assert( static_cast<size_t>( EFieldType::TYPE_MEDIUM_BLOB ) == static_cast<size_t>( ::MYSQL_TYPE_MEDIUM_BLOB ) );
    static_assert( static_cast<size_t>( EFieldType::TYPE_LONG_BLOB   ) == static_cast<size_t>( ::MYSQL_TYPE_LONG_BLOB   ) );
    static_assert( static_cast<size_t>( EFieldType::TYPE_BLOB        ) == static_cast<size_t>( ::MYSQL_TYPE_BLOB        ) );
    static_assert( static_cast<size_t>( EFieldType::TYPE_VAR_STRING  ) == static_cast<size_t>( ::MYSQL_TYPE_VAR_STRING  ) );
    static_assert( static_cast<size_t>( EFieldType::TYPE_STRING      ) == static_cast<size_t>( ::MYSQL_TYPE_STRING      ) );
    static_assert( static_cast<size_t>( EFieldType::TYPE_GEOMETRY    ) == static_cast<size_t>( ::MYSQL_TYPE_GEOMETRY    ) );

    void DBStatement::Parameter::Reset( void* InBuffer, uint32_t InBufferLength ) noexcept
    {
        SKL_ASSERT( InBufferLength == 0 || nullptr != InBuffer );

        memset( &Bind, 0, sizeof( decltype( Bind ) ) );

        auto& MysqlBind{ reinterpret_cast<::MYSQL_BIND&>( Bind ) };
        MysqlBind.buffer        = InBuffer;
        MysqlBind.buffer_length = InBufferLength;
        MysqlBind.is_null_value = InBuffer == nullptr;
    }

    void DBStatement::Parameter::Reset( void* InBuffer, uint32_t InBufferLength, EFieldType InType, bool bIsUnsigned ) noexcept
    {
        SKL_ASSERT( InBufferLength == 0 || nullptr != InBuffer );

        memset( &Bind, 0, sizeof( decltype( Bind ) ) );

        auto& MysqlBind{ reinterpret_cast<::MYSQL_BIND&>( Bind ) };
        MysqlBind.buffer        = InBuffer;
        MysqlBind.buffer_length = InBufferLength;
        MysqlBind.is_null_value = InBuffer == nullptr;
        MysqlBind.buffer_type   = static_cast<enum_field_types>( InType );
        MysqlBind.is_unsigned   = bIsUnsigned;
    }

    void DBStatement::Parameter::Reset( void* InBuffer, uint32_t InBufferLength, uint32_t* OutFieldLengthDestiantion, EFieldType InType, bool bIsUnsigned ) noexcept
    {
        memset( &Bind, 0, sizeof( decltype( Bind ) ) );

        auto& MysqlBind{ reinterpret_cast<::MYSQL_BIND&>( Bind ) };
        MysqlBind.buffer        = InBuffer;
        MysqlBind.buffer_length = InBufferLength;
        MysqlBind.is_null_value = InBuffer == nullptr;
        MysqlBind.buffer_type   = static_cast<enum_field_types>( InType );
        MysqlBind.is_unsigned   = bIsUnsigned;
        MysqlBind.length        = reinterpret_cast<unsigned long*>( OutFieldLengthDestiantion );
    }

    void DBStatement::Parameter::SetType( EFieldType InType, bool bIsUnsigned ) noexcept
    {
        auto& MysqlBind{ reinterpret_cast<::MYSQL_BIND&>( Bind ) };
        MysqlBind.buffer_type = static_cast<enum_field_types>( InType );
        MysqlBind.is_unsigned = bIsUnsigned;
    }

    void DBStatement::Result::FreeResultMetadata() noexcept
    {
        SKL_ASSERT( nullptr != ResultMetadata );
        ::mysql_free_result( reinterpret_cast<::MYSQL_RES*>( ResultMetadata ) );
        ResultMetadata = nullptr;
    }

    bool DBStatement::Result::PrepareResult() const noexcept
    {
        if( true == ::mysql_stmt_bind_result( reinterpret_cast<::MYSQL_STMT*>( Statement->Statement ), reinterpret_cast<::MYSQL_BIND*>( Statement->Output ) ) ) SKL_UNLIKELY
        {
            const char* MysqlErrorString{ ::mysql_stmt_error( reinterpret_cast<::MYSQL_STMT*>( Statement->Statement ) ) };
            SKLL_TRACE_MSG_FMT( "MysqlError: %s!", MysqlErrorString );
            return false;
        }

        return true;
    }

    bool DBStatement::Result::Next() const noexcept
    {
        const auto Status{ ::mysql_stmt_fetch( reinterpret_cast<::MYSQL_STMT*>( Statement->Statement ) ) };
        if( 1 == Status )
        {
            return false;    
        }

        return MYSQL_NO_DATA != Status;
    }
    
    bool DBStatement::Result::FetchColumn( int32_t InIndex ) noexcept
    {
        int32_t Result{ ::mysql_stmt_fetch_column( 
                  reinterpret_cast<::MYSQL_STMT*>( Statement->Statement )
                , reinterpret_cast<::MYSQL_BIND*>( &GetBind )
                , InIndex - 1
                , 0 ) };
        if( 0 != Result ) SKL_UNLIKELY
        {
            const char* MysqlErrorString{ ::mysql_stmt_error( reinterpret_cast<::MYSQL_STMT*>( Statement->Statement ) ) };
            SKLL_TRACE_MSG_FMT( "MysqlError: %s!", MysqlErrorString );
        }

        return 0 == Result;
    }

    DBStatement::~DBStatement() noexcept
    {
        Reset( true );
        ReleaseStatement();
        Connection = nullptr;
    }

    bool DBStatement::Initialize( DBConnection* InConnection ) noexcept
    {
        SKL_ASSERT( nullptr != InConnection );

        auto* NewStatement{ ::mysql_stmt_init( reinterpret_cast<::MYSQL*>( &InConnection->GetMysqlObject() ) ) };
        if( nullptr == NewStatement ) SKL_UNLIKELY
        {
            const char* MysqlErrorString{ ::mysql_stmt_error( reinterpret_cast<::MYSQL_STMT*>( Statement ) ) };
            SKLL_TRACE_MSG_FMT( "MysqlError: %s!", MysqlErrorString );
            return false;
        }

        Connection = InConnection;
        Statement  = reinterpret_cast<MysqlStmtOpaque*>( NewStatement );

        return true;
    }

    bool DBStatement::Prepare() noexcept
    {
        const int32_t Result{ ::mysql_stmt_prepare( reinterpret_cast<::MYSQL_STMT*>( Statement ), Query.get(), static_cast<unsigned long>( QueryStringLength ) ) };
        if( 0 != Result ) SKL_UNLIKELY
        {
            const char* MysqlErrorString{ ::mysql_stmt_error( reinterpret_cast<::MYSQL_STMT*>( Statement ) ) };
            SKLL_TRACE_MSG_FMT( "MysqlError: %s!", MysqlErrorString );
            return false;
        }

        QueryParametersCount = ::mysql_stmt_param_count( reinterpret_cast<::MYSQL_STMT*>( Statement ) );

        return true;
    }

    void DBStatement::ReleaseStatement() noexcept
    {
        if( nullptr != Statement )
        {
            ::mysql_stmt_free_result( reinterpret_cast<::MYSQL_STMT*>( Statement ) );
            ::mysql_stmt_close( reinterpret_cast<::MYSQL_STMT*>( Statement ) );

            Statement = nullptr;
        }

        bIsInitialized = false;
        bNeedsReinitialization = true;
    }
    
    const char* DBStatement::GetError() const noexcept
    {
        SKL_ASSERT( nullptr != Statement );
        return ::mysql_stmt_error( reinterpret_cast<::MYSQL_STMT*>( Statement ) );
    }

    DBStatement::Result DBStatement::Execute() noexcept
    {
        DBStatement::Result OutResult{ this, nullptr, 0 };
        bool bHasReaquiredConnectionOnce{ false };
        bool bSuccess{ true };

        if( 0 != BoundInputsCount )
        {
            if( true == ::mysql_stmt_bind_param( reinterpret_cast<::MYSQL_STMT*>( Statement ), reinterpret_cast<::MYSQL_BIND*>( Input ) ) ) SKL_UNLIKELY
            {
                const char* MysqlErrorString{ ::mysql_stmt_error( reinterpret_cast<::MYSQL_STMT*>( Statement ) ) };
                SKLL_TRACE_MSG_FMT( "MysqlError: %s!", MysqlErrorString );
                bSuccess = false;
            }
        }

        if( true == bSuccess && 0 != BoundOutputsCount )
        {
            if( true == ::mysql_stmt_bind_result( reinterpret_cast<::MYSQL_STMT*>( Statement ), reinterpret_cast<::MYSQL_BIND*>( Output ) ) )  SKL_UNLIKELY
            {
                const char* MysqlErrorString{ ::mysql_stmt_error( reinterpret_cast<MYSQL_STMT*>( Statement ) ) };
                SKLL_TRACE_MSG_FMT( "MysqlError: %s!", MysqlErrorString );
                bSuccess = false;
            }
        }

DBStatement_Execute_Start:
        if( true == bSuccess ) SKL_LIKELY
        {
            int32_t Result{ ::mysql_stmt_execute( reinterpret_cast<::MYSQL_STMT*>( Statement ) ) };
            if( 0 != Result ) SKL_UNLIKELY
            {
                const auto LastErrorNo{ reinterpret_cast<::MYSQL_STMT*>( Statement )->last_errno };
                if( false == bHasReaquiredConnectionOnce && ( CR_SERVER_LOST == LastErrorNo || CR_SERVER_GONE_ERROR == LastErrorNo ) )
                {
                    const auto AcquireResult{ Connection->TryReacquireConnection() };
                    if( true == AcquireResult.IsSuccess() )
                    {
                        bHasReaquiredConnectionOnce = true;
                        goto DBStatement_Execute_Start;
                    }
                    else
                    {
                        SKLL_TRACE_MSG( "Failed to reaquire mysql connection" );
                    }
                }

                const char* MysqlErrorString{ ::mysql_stmt_error( reinterpret_cast<MYSQL_STMT*>( Statement ) ) };
                SKLL_TRACE_MSG_FMT( "MysqlError: %s!", MysqlErrorString );
            }
            else
            {
                MYSQL_RES* ResultMetadata{ ::mysql_stmt_result_metadata( reinterpret_cast<::MYSQL_STMT*>( Statement ) ) };

                Result = ::mysql_stmt_store_result( reinterpret_cast<::MYSQL_STMT*>( Statement ) ); 
                if( 0 != Result ) SKL_UNLIKELY
                {
                    const char* MysqlErrorString{ ::mysql_stmt_error( reinterpret_cast<::MYSQL_STMT*>( Statement ) ) };
                    SKLL_TRACE_MSG_FMT( "MysqlError: %s!", MysqlErrorString );
                }
                else
                {
                    // Success
                    const auto NoOfRowsAffected{ mysql_stmt_num_rows( reinterpret_cast<::MYSQL_STMT*>( Statement ) ) };
                    OutResult.Statement      = this;
                    OutResult.ResultMetadata = reinterpret_cast<MysqlResOpaue*>( ResultMetadata );
                    OutResult.NoOfRows       = NoOfRowsAffected;
                }
            }
        }

        return OutResult;
    }

    int64_t DBStatement::ExecuteCount() noexcept
    {
        int64_t Count{ 0 };
        BindOutput( 1, &Count );

        const auto Result{ Execute() };
        if( false == static_cast<bool>( Result ) )
        {
            return -1;
        }

        if( false == Result.Next() )
        {
            return -1;
        }

        return Count;
    }

    std::pair<uint64_t, bool> DBStatement::ExecuteUpdate() noexcept
    {
        bool bHasReaquiredConnectionOnce{ false };
        bool bSuccess{ true };

        if( 0 != BoundInputsCount )
        {
            if( true == ::mysql_stmt_bind_param( reinterpret_cast<::MYSQL_STMT*>( Statement ), reinterpret_cast<::MYSQL_BIND*>( Input ) ) ) SKL_UNLIKELY
            {
                const char* MysqlErrorString{ ::mysql_stmt_error( reinterpret_cast<::MYSQL_STMT*>( Statement ) ) };
                SKLL_TRACE_MSG_FMT( "MysqlError: %s!", MysqlErrorString );
                bSuccess = false;
            }
        }

        if( true == bSuccess && 0 != BoundOutputsCount )
        {
            if( true == ::mysql_stmt_bind_result( reinterpret_cast<::MYSQL_STMT*>( Statement ), reinterpret_cast<::MYSQL_BIND*>( Input ) ) )  SKL_UNLIKELY
            {
                const char* MysqlErrorString{ ::mysql_stmt_error( reinterpret_cast<MYSQL_STMT*>( Statement ) ) };
                SKLL_TRACE_MSG_FMT( "MysqlError: %s!", MysqlErrorString );
                bSuccess = false;
            }
        }

DBStatement_ExecuteUpdate_Start:
        if( true == bSuccess ) SKL_LIKELY
        {
            int32_t Result{ ::mysql_stmt_execute( reinterpret_cast<::MYSQL_STMT*>( Statement ) ) };
            if( 0 != Result ) SKL_UNLIKELY
            {
                const auto LastErrorNo{ reinterpret_cast<::MYSQL_STMT*>( Statement )->last_errno };
                if( false == bHasReaquiredConnectionOnce && ( CR_SERVER_LOST == LastErrorNo || CR_SERVER_GONE_ERROR == LastErrorNo ) )
                {
                    const auto AcquireResult{ Connection->TryReacquireConnection() };
                    if( true == AcquireResult.IsSuccess() )
                    {
                        bHasReaquiredConnectionOnce = true;
                        goto DBStatement_ExecuteUpdate_Start;
                    }
                    else
                    {
                        SKLL_TRACE_MSG( "Failed to reaquire mysql connection" );
                    }
                }

                const char* MysqlErrorString{ ::mysql_stmt_error( reinterpret_cast<MYSQL_STMT*>( Statement ) ) };
                SKLL_TRACE_MSG_FMT( "MysqlError: %s!", MysqlErrorString );
            }
            else
            {
                // Success
                const auto NoOfRowsAffected{ mysql_stmt_affected_rows( reinterpret_cast<MYSQL_STMT*>( Statement ) ) };
                return { NoOfRowsAffected , false };
            }
        }

        return { 0 , true };
    }

    bool DBStatement::Reset( bool bShouldDoFullReset ) noexcept
    {
        BoundInputsCount = 0;
        BoundOutputsCount = 0;

        if( nullptr != Statement ) SKL_LIKELY
        {
            if( true == bShouldDoFullReset )
            {
                if( true == ::mysql_stmt_reset( reinterpret_cast<::MYSQL_STMT*>( Statement ) ) ) SKL_UNLIKELY
                {
                    const char* MysqlErrorString{ ::mysql_stmt_error( reinterpret_cast<::MYSQL_STMT*>( Statement ) ) };
                    SKLL_TRACE_MSG_FMT( "MysqlError: %s!", MysqlErrorString );
                    return false;
                }
            }
            else
            {
               if( true == ::mysql_stmt_free_result( reinterpret_cast<::MYSQL_STMT*>( Statement ) ) ) SKL_UNLIKELY
               {
                    const char* MysqlErrorString{ ::mysql_stmt_error( reinterpret_cast<::MYSQL_STMT*>( Statement ) ) };
                    SKLL_TRACE_MSG_FMT( "MysqlError: %s!", MysqlErrorString );
                    return false;
               }
            }
        }

        return true;
    }

    bool DBStatement::InitializeAndPrepare( DBConnection* InConnection ) noexcept
    {
        if( 0 == GetQueryLength() )
        {
            SKLL_TRACE_MSG( "No query string was sent! Set the query string before calling this method!" );
            return false;
        }

        if( false == Initialize( InConnection ) )
        {
            SKLL_TRACE_MSG( "Failed to Initialize()!" );
            return false;
        }
        
        if( false == Prepare() )
        {
            SKLL_TRACE_MSG( "Failed to Initialize()!" );
            return false;
        }

        bIsInitialized = true;

        return true;
    }
}
