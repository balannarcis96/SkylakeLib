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

        DBConnection( DBConnection&& Other ) noexcept = delete;
        DBConnection& operator=( DBConnection&& Other ) noexcept = delete;
        ~DBConnection() noexcept;

        DBConnection( const DBConnection& ) = delete;
        DBConnection& operator=( const DBConnection& ) = delete;

        SKL_FORCEINLINE bool IsOpen() const noexcept { return bIsOpen; }
        SKL_NODISCARD const char* GetStatus() noexcept;
        SKL_NODISCARD const char* GetLastMysqlError() noexcept;
        SKL_NODISCARD bool Ping() noexcept;
        
        SKL_FORCEINLINE SKL_NODISCARD bool HasTransaction( ) const noexcept { return bIsTransactionStarted; }
        SKL_NODISCARD bool StartTransaction( ) noexcept
        {
            if( true == bIsTransactionStarted ) SKL_UNLIKELY
            {
                return false;
            }

            if( false == Execute( "START TRANSACTION", 18 ) ) SKL_UNLIKELY
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

            if( false == Execute( "ROLLBACK", 9 ) ) SKL_UNLIKELY
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

            if( false == Execute( "COMMIT", 7 ) ) SKL_UNLIKELY
            {
                return false;
            }

            bIsTransactionStarted = false;

            return true;
        }
        void CloseConnection() noexcept;

        //! Execute CUD (create/update/delete) queries, if binary data is queried into the db use Execute( Query, InQueryLength ) overload
        //! \returns -1 if the execution failed 
        //! \returns >= 0 (NoOfRowsAffected) on successfull execution
        int64_t Execute( const char *Query ) noexcept;
        
        //! Execute CUD (create/update/delete) queries 
        //! \returns -1 if the execution failed 
        //! \returns >= 0 (NoOfRowsAffected) on successfull execution
        int64_t Execute( const char *Query, uint32_t InQueryLength ) noexcept;

        SKL_FORCEINLINE SKL_NODISCARD MYSQL_Opaque& GetMysqlObject() noexcept { return Mysql; }
        SKL_FORCEINLINE SKL_NODISCARD const MYSQL_Opaque& GetMysqlObject() const noexcept { return Mysql; }

    private:
        DBConnection( const DBConnectionSettings& Settings ) noexcept;

        AcquireResult TryReacquireConnection() noexcept;
        bool OpenConnection() noexcept;
        bool SetOptions() noexcept;

    private:
        bool                                bIsOpen              { false };
        bool                                bIsTransactionStarted{ false };
        MYSQL_Opaque                        Mysql                {};
        DBConnectionSettings                Settings             {};

        friend DBConnectionFactory;
        friend DBStatement;
    };
}