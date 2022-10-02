//!
//! \file DBConnection.h
//! 
//! \brief MySQL c++ interface library
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL::DB
{
    struct DBConnection
    {
        struct AcquireResult
        {
            bool bHasError;
            bool bHasReconnected;

            SKL_FORCEINLINE bool IsSuccess( ) const noexcept
            {
                return false == bHasError;
            }
            SKL_FORCEINLINE bool HasReconnected( ) const noexcept
            {
                return bHasReconnected;
            }
        };

        DBConnection( DBConnection&& Other ) noexcept
            : bIsOpen { Other.bIsOpen }
            , bIsTransactionStarted{ Other.bIsTransactionStarted }
            , Mysql{ std::move( Other.Mysql ) }
            , Settings{ std::move( Other.Settings ) }
        {
            Other.bIsOpen               = false;
            Other.bIsTransactionStarted = false;
        }
        DBConnection& operator=( DBConnection&& Other ) noexcept
        {
            SKL_ASSERT( this != &Other );

            bIsOpen               = Other.bIsOpen;
            bIsTransactionStarted = Other.bIsTransactionStarted;
            Mysql                 = std::move( Other.Mysql );
            Settings              = std::move( Other.Settings );

            Other.bIsOpen               = false;
            Other.bIsTransactionStarted = false;

            return *this;
        }

        DBConnection( const DBConnection& ) = delete;
        DBConnection& operator=( const DBConnection& ) = delete;

        SKL_FORCEINLINE bool IsOpen() const noexcept { return bIsOpen; }
        SKL_NODISCARD const char* GetStatus() noexcept;
        SKL_NODISCARD const char* GetLastMysqlError() noexcept;
        
        SKL_FORCEINLINE bool HasTransaction( ) const noexcept { return bIsTransactionStarted; }
        bool StartTransaction( ) noexcept
        {
            if( true == bIsTransactionStarted ) SKL_UNLIKELY
            {
                return false;
            }

            if( false == ExecuteUpdateQuery( "START TRANSACTION" ) ) SKL_UNLIKELY
            {
                return false;
            }

            bIsTransactionStarted = true;

            return true;
        }
        bool RollbackTransaction( ) noexcept
        {
            if( false == bIsTransactionStarted ) SKL_UNLIKELY
            {
                return false;
            }

            if( false == ExecuteUpdateQuery( "ROLLBACK" ) ) SKL_UNLIKELY
            {
                return false;
            }

            bIsTransactionStarted = false;

            return true;
        }
        bool CommitTransaction( ) noexcept
        {
            if( false == bIsTransactionStarted ) SKL_UNLIKELY
            {
                return false;
            }

            if( false == ExecuteUpdateQuery( "COMMIT" ) ) SKL_UNLIKELY
            {
                return false;
            }

            bIsTransactionStarted = false;

            return true;
        }
        bool ExecuteUpdateQuery( const char *Query ) noexcept;
        void CloseConnection() noexcept;

        SKL_FORCEINLINE SKL_NODISCARD MYSQL_Opaque& GetMysqlObject() noexcept { return Mysql; }
        SKL_FORCEINLINE SKL_NODISCARD const MYSQL_Opaque& GetMysqlObject() const noexcept { return Mysql; }


    private:
        DBConnection( const DBConnectionSettings& Settings ) noexcept;
        ~DBConnection() noexcept;

        AcquireResult AcquireConnection( bool bOpenConnectionIfClosed = true ) noexcept;
        bool TryReacquireConnection() noexcept;
        bool OpenConnection() noexcept;
        bool SetOptions() noexcept;
        bool Execute( const char *Query ) noexcept;

    private:
        bool                                bIsOpen              { false };
        bool                                bIsTransactionStarted{ false };
        MYSQL_Opaque                        Mysql                {};
        DBConnectionSettings                Settings             {};

        friend DBConnectionFactory;
        friend DBStatement;
    };
}