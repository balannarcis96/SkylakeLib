//!
//! \file DBStatement.h
//! 
//! \brief MySQL c++ interface library
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

#include <SkylakeLibStandalone.h>

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

    struct DBConnection;
    
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

    struct DBStatement
    {
        DBStatement() noexcept
        {
            Query.reset( new char[CDBStatementQueryMaxLength] );
            SKL_ASSERT( nullptr != Query );
        }
        ~DBStatement() noexcept;

        DBStatement( const DBStatement& ) = delete;
        DBStatement( DBStatement&& ) = delete;
        DBStatement& operator=( const DBStatement& ) = delete;
        DBStatement& operator=( DBStatement&& ) = delete;

        struct Parameter
        {
            Parameter() noexcept{ Bind.Zero(); }
            ~Parameter() = default;

            Parameter( const Parameter& ) = delete;
            Parameter( Parameter&& ) = delete;
            Parameter& operator=( const Parameter& ) = delete;
            Parameter& operator=( Parameter&& ) = delete;

            void Reset( void* InBuffer, uint32_t InBufferLength ) noexcept;
            void Reset( void* InBuffer, uint32_t InBufferLength, EFieldType InType, bool bIsUnsigned = false ) noexcept;
            void Reset( void* InBuffer, uint32_t InBufferLength, uint32_t* OutFieldLengthDestiantion, EFieldType InType, bool bIsUnsigned = false ) noexcept;
            void SetType( EFieldType InType, bool bIsUnsigned = false ) noexcept;

        private:
            MysqlBindOpaue Bind;

            friend DBStatement;
        };
        struct Result
        {
            ~Result() noexcept
            {
                if( nullptr != ResultMetadata )
                {
                    FreeResultMetadata();
                }
            }

            SKL_FORCEINLINE SKL_NODISCARD uint64_t GetNoOfRows() const noexcept { return NoOfRows; }
            SKL_FORCEINLINE SKL_NODISCARD bool IsEmpty() const noexcept { return 0 == NoOfRows; }
            SKL_FORCEINLINE SKL_NODISCARD bool IsValid() const noexcept { return nullptr != Statement; }
            SKL_FORCEINLINE explicit operator bool() const noexcept { return true == IsValid() && false == IsEmpty(); }
            SKL_NODISCARD bool PrepareResult() const noexcept;
            SKL_NODISCARD bool Next() const noexcept;
            SKL_NODISCARD bool GetOneResult() const noexcept
            {
                if( false == PrepareResult() )
                {
                    return false;
                }

                return Next();
            }


        private:
            Result( DBStatement* Statement, MysqlResOpaue* ResultMetadata, uint64_t NoOfRows ) noexcept
                : GetBind{}
                , Statement{ Statement }
                , ResultMetadata{ ResultMetadata }
                , NoOfRows{ NoOfRows } 
                {}

            Result( Result&& Other ) noexcept
                : GetBind{}
                , Statement{ Other.Statement }
                , ResultMetadata{ Other.ResultMetadata }
                , NoOfRows{ Other.NoOfRows } 
            {
                Other.Statement      = nullptr;
                Other.ResultMetadata = nullptr;
                Other.NoOfRows       = 0;
            }
            Result& operator=( Result&& Other ) noexcept
            {
                SKL_ASSERT( this != &Other );

                if( nullptr != Statement )
                {
                    FreeResultMetadata();
                }

                Statement      = Other.Statement;
                ResultMetadata = Other.ResultMetadata;
                NoOfRows       = Other.NoOfRows;

                Other.Statement      = nullptr;
                Other.ResultMetadata = nullptr;
                Other.NoOfRows       = 0;

                return *this;
            }

            void FreeResultMetadata() noexcept;

        private:
            MysqlBindOpaue GetBind;
            DBStatement*   Statement;
            MysqlResOpaue* ResultMetadata;
            uint64_t       NoOfRows;

            friend DBStatement;
        };

        SKL_FORCEINLINE SKL_NODISCARD std::pair<const char*, size_t> GetQueryBuffer() const noexcept{ SKL_ASSERT( nullptr != Query.get() ); return { Query.get(), CDBStatementQueryMaxLength }; }
        SKL_FORCEINLINE SKL_NODISCARD std::pair<const char*, size_t> GetQuery() const noexcept{ SKL_ASSERT( nullptr != Query.get() ); return { Query.get(), QueryStringLength }; }
        SKL_FORCEINLINE SKL_NODISCARD size_t GetQueryLength() const noexcept{ SKL_ASSERT( nullptr != Query.get() ); return QueryStringLength; }
        SKL_FORCEINLINE SKL_NODISCARD DBConnection* GetConnection() const noexcept { return Connection; }
        SKL_FORCEINLINE SKL_NODISCARD MysqlStmtOpaque* GetMysqlStatement() const noexcept { return Statement; }
        SKL_FORCEINLINE SKL_NODISCARD uint32_t GetQueryParametersCount() const noexcept { return QueryParametersCount; }
        SKL_FORCEINLINE SKL_NODISCARD uint32_t GetBoundInputsCount() const noexcept { return BoundInputsCount; }
        SKL_FORCEINLINE SKL_NODISCARD uint32_t GetBoundOutputsCount() const noexcept { return BoundOutputsCount; }
        SKL_FORCEINLINE SKL_NODISCARD bool IsInitialized() const noexcept { return bIsInitialized; }
        
        //! Set query string 
        SKL_FORCEINLINE void SetQuery( const char* InQueryString, size_t InQueryStringLength ) noexcept 
        { 
            SKL_ASSERT( nullptr != Query.get() );
            SKL_ASSERT( InQueryStringLength < CDBStatementQueryMaxLength );
            SKL_STRCPY( Query.get(), InQueryString, InQueryStringLength ); 
            QueryStringLength = InQueryStringLength;
        }
        
        //! Set query string 
        template<size_t InQueryStringLength>
        SKL_FORCEINLINE void SetQuery( const char( &InQueryString )[InQueryStringLength] ) noexcept
        {
            static_assert( InQueryStringLength < CDBStatementQueryMaxLength, "Query string to long, please resize the statement buffer length!" );
            SetQuery( InQueryString, InQueryStringLength );
        }

        void ReleaseStatement() noexcept;
        SKL_NODISCARD const char* GetError() const noexcept;
        
        //! Execute CRUD(create/read/update/delete) queries, for CUD(create/update/delete) with no output use ExecuteUpdate(...)
        //! \return Result object
        Result Execute() noexcept;

        //! Used for COUNT(*) queries
        //! \returns -1 if the execution failed 
        //! \returns >= 0 (actual count) on successfull execution
        int64_t ExecuteCount() noexcept;

        //! Execute CUD (create/update/delete) queries 
        //! \return <NoOfRowsAffected, bErrorHasOccurred>
        std::pair<uint64_t, bool> ExecuteUpdate() noexcept;

        //! Reset the statement state and clear any intermediate data 
        bool Reset( bool bShouldDoFullReset = false ) noexcept;

        //! Initialize and prepare the statement 
        bool InitializeAndPrepare( DBConnection* InConnection ) noexcept;

        //! Bind value as input/output for query parameter
        //! \tparam TType must be value type
        //! \param InIndex 1-based index of param found in the query string
        //! \param InValue pointer to the value, must be alive until call to Execute
        template<typename TType, bool bIsInput = false>
        void Bind( int32_t InIndex, TType* InValue ) noexcept
        {
            SKL_ASSERT( 0 < InIndex );
            static_assert( std::numeric_limits<uint32_t>::max() >= sizeof( TType ) );

            Parameter* NewParameter;

            if constexpr( true == bIsInput )
            {
                SKL_ASSERT( InIndex < CDBStatementMaxInputParams );

                InputLengths[InIndex - 1] = static_cast<uint32_t>( sizeof( TType ) );
                NewParameter              = &Input[InIndex - 1];
            }
            else
            {
                SKL_ASSERT( InIndex < CDBStatementMaxOutputParams );

                OutputLengths[InIndex - 1] = static_cast<uint32_t>( sizeof( TType ) );
                NewParameter               = &Output[InIndex - 1];
            }

            BindImpl( NewParameter, InValue );
        }

        //! Bind string as input/output for query parameter
        //! \tparam TType must be value type
        //! \param InIndex 1-based index of param found in the query string
        //! \param InValue pointer to the value, must be alive until call to Execute
        template<size_t InStringBufferSize, bool bIsInput = false>
        void BindString( int32_t InIndex, DBString<InStringBufferSize>& InValue ) noexcept
        {
            SKL_ASSERT( 0 < InIndex );

            auto*       Utf8String      { InValue.GetUtf8() };
            const auto  Utf8StringLength{ InValue.GetUtf8Size() };

            SKL_ASSERT( Utf8StringLength < std::numeric_limits<uint32_t>::max() );

            if constexpr( true == bIsInput )
            {
                SKL_ASSERT( InIndex < CDBStatementMaxInputParams );

                InputLengths[InIndex - 1] = Utf8StringLength;
                auto* NewParameter{ &Input[InIndex - 1] };

                ++BoundInputsCount;

                BindStringImpl( NewParameter, Utf8String, &InputLengths[ InIndex - 1 ] );
            }
            else
            {
                SKL_ASSERT( InIndex < CDBStatementMaxOutputParams );

                OutputLengths[InIndex - 1] = Utf8StringLength;
                auto* NewParameter{ &Input[InIndex - 1] };

                ++BoundOutputsCount;

                BindStringImpl( NewParameter, Utf8String, &OutputLengths[ InIndex - 1 ] );
            }
        }

        //! Bind blob as input for query parameter
        //! \tparam TType must be value type
        //! \param InIndex 1-based index of param found in the query string
        void BindInputBlob( int32_t InIndex, char* InBuffer, const uint32_t InBufferSize ) noexcept
        {
            SKL_ASSERT( 0 < InIndex );
            
            InputLengths[InIndex - 1] = InBufferSize;
            auto* NewParameter{ &Input[InIndex - 1] };

            ++BoundInputsCount;

            BindStringImpl( NewParameter, InBuffer, &InputLengths[ InIndex - 1 ] );
        }
        
        //! Bind blob as input for query parameter
        //! \tparam TType must be value type
        //! \param InIndex 1-based index of param found in the query string
        template<size_t InBufferSize>
        SKL_FORCEINLINE void BindInputBlob( const int32_t InIndex, char*( &InBuffer )[InBufferSize] ) noexcept
        {
            return BindInputBlob( InIndex, InBuffer );
        }

        //! Bind value as output for query
        //! \tparam TType must be value type
        //! \param InIndex 1-based index of param found in the query string
        //! \param InValue pointer to the value, must be alive until call to Execute
        template<typename TType>
        SKL_FORCEINLINE void BindOutput( int32_t InIndex, TType* InValue ) noexcept
        {
            return Bind<TType, false>( InIndex, InValue );
        }

        //! Bind string as output for query
        //! \param InIndex 1-based index of param found in the query string
        template<size_t InStringBufferSize>
        SKL_FORCEINLINE void BindOutputString( int32_t InIndex, DBString<InStringBufferSize>& InValue ) noexcept
        {
            BindString<InStringBufferSize, false>( InIndex, InValue );

            InValue.bHasSource    = true;
            InValue.bIsUTF8Source = true;
            InValue.bHasUTF8      = true;
        }

    private:
        bool Initialize( DBConnection* InConnection ) noexcept;
        bool Prepare() noexcept;

        template<typename TType>
        static void BindImpl( Parameter* InParameter, TType *InValue ) noexcept
        {
            static_assert( std::numeric_limits<uint32_t>::max() >= sizeof( TType ) );

            SKL_ASSERT( nullptr != InParameter );
            SKL_ASSERT( nullptr != InValue );

            using BindType = std::decay_t<TType>;

            InParameter->Reset( static_cast<void*>( InParameter ), static_cast<uint32_t>( sizeof( BindType ) ) );
            
            if constexpr( std::is_same_v<BindType, int> || std::is_same_v<BindType, long> )
            {
                InParameter->SetType( EFieldType::TYPE_LONG );
            }
            else if constexpr( std::is_same_v<BindType, unsigned int> || std::is_same_v<BindType, unsigned long> )
            {
                InParameter->SetType( EFieldType::TYPE_LONG, true );
            }
            else if constexpr( std::is_same_v<BindType, short> )
            {
                InParameter->SetType( EFieldType::TYPE_SHORT );
            }
            else if constexpr( std::is_same_v<BindType, unsigned short> )
            {
                InParameter->SetType( EFieldType::TYPE_SHORT, true );
            }
            else if constexpr( std::is_same_v<BindType, long long> )
            {
                InParameter->SetType( EFieldType::TYPE_LONGLONG );
            }
            else if constexpr( std::is_same_v<BindType, unsigned long long> )
            {
                InParameter->SetType( EFieldType::TYPE_LONGLONG, true );
            }
            else if constexpr( std::is_same_v<BindType, ::SKL::TDatabaseId> )
            {
                InParameter->SetType( EFieldType::TYPE_LONGLONG, true );
            }
            else if constexpr( std::is_same_v<BindType, float> )
            {
                InParameter->SetType( EFieldType::TYPE_FLOAT );
            }
            else if constexpr( std::is_same_v<BindType, double> )
            {
                InParameter->SetType( EFieldType::TYPE_DOUBLE );
            }
            else if constexpr( std::is_same_v<BindType, bool> )
            {
                InParameter->SetType( EFieldType::TYPE_BIT );
            }
            else if constexpr( std::is_same_v<BindType, ::SKL::TEntityIdBase> )
            {
                InParameter->SetType( EFieldType::TYPE_LONGLONG, true );
            }
            else
            {
                SKL_ASSERT_ALLWAYS_MSG( false, "Unsupported Bind Type!" );
            }
        }

        SKL_FORCEINLINE static bool BindStringImpl( Parameter* InParameter, char* InString, uint32_t* InOutLength ) noexcept
        {
            SKL_ASSERT( nullptr != InParameter );
            InParameter->Reset( static_cast<void*>( InString ), *InOutLength, InOutLength, EFieldType::TYPE_STRING, false );
        }
        SKL_FORCEINLINE static bool BindBlobImpl( Parameter* InParameter, char* InString, uint32_t* InOutLength ) noexcept
        {
            SKL_ASSERT( nullptr != InParameter );
            InParameter->Reset( static_cast<void*>( InString ), *InOutLength, InOutLength, EFieldType::TYPE_BLOB, false );
        }

    private:
        bool                    bIsInitialized                            { false };
        bool                    bNeedsReinitialization                    { false };
        DBConnection*           Connection                                { nullptr };
        MysqlStmtOpaque*        Statement                                 { nullptr };
        Parameter               Input[CDBStatementMaxInputParams]         {};
        Parameter               Output[CDBStatementMaxOutputParams]       {};
        uint32_t                InputLengths[CDBStatementMaxInputParams]  {};
        uint32_t                OutputLengths[CDBStatementMaxOutputParams]{};
        std::unique_ptr<char[]> Query                                     { nullptr };      
        size_t                  QueryStringLength                         { 0 };
        uint32_t                QueryParametersCount                      { 0 };
        uint32_t                BoundInputsCount                          { 0 };
        uint32_t                BoundOutputsCount                         { 0 };
    };
}