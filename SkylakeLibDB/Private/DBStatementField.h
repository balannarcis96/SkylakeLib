//!
//! \file DBStatementField.h
//! 
//! \brief MySQL c++ interface library
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL::DB
{
    constexpr size_t CTinyBlobMaximumSize   = 255;
    constexpr size_t CBlobMaximumSize       = 65535;
    constexpr size_t CMediumBlobMaximumSize = 16777215;
    constexpr size_t CLongBlobMaximumSize   = 4294967295; // Not Supported as a DBStatementField

    template<EFieldType InFieldType, bool bIsUnsigned, size_t InMaxBufferSize>
    struct DBStatementField;
   
    template<>
    struct DBStatementField<EFieldType::TYPE_DECIMAL, false, sizeof( double )> 
    {
        static constexpr EFieldType FieldType  = EFieldType::TYPE_DECIMAL;
        static constexpr size_t     BufferSize = sizeof( double );
        using ValueType                        = double;

        SKL_FORCEINLINE ValueType& GetValue() noexcept { return Value; }
        SKL_FORCEINLINE const ValueType& GetValue() const noexcept { return Value; }
        SKL_FORCEINLINE void Reset() noexcept { Value = 0; }

        ValueType Value{ 0.0 };
    };

    template<>
    struct DBStatementField<EFieldType::TYPE_NEWDECIMAL, false, sizeof( double )> 
    {
        static constexpr EFieldType FieldType  = EFieldType::TYPE_NEWDECIMAL;
        static constexpr size_t     BufferSize = sizeof( double );
        using ValueType                        = double;

        SKL_FORCEINLINE ValueType& GetValue() noexcept { return Value; }
        SKL_FORCEINLINE const ValueType& GetValue() const noexcept { return Value; }
        SKL_FORCEINLINE void Reset() noexcept { Value = 0; }

        ValueType Value{ 0.0 };
    };
    
    template<bool bIsUnsigned>
    struct DBStatementField<EFieldType::TYPE_TINY, bIsUnsigned, sizeof( uint8_t )> 
    {
        static constexpr EFieldType FieldType  = EFieldType::TYPE_TINY;
        static constexpr size_t     BufferSize = sizeof( uint8_t );
        using ValueType                        = std::conditional_t<bIsUnsigned, uint8_t, int8_t>;

        SKL_FORCEINLINE ValueType& GetValue() noexcept { return Value; }
        SKL_FORCEINLINE const ValueType& GetValue() const noexcept { return Value; }
        SKL_FORCEINLINE void Reset() noexcept { Value = 0; }

        ValueType Value{ 0 };
    };
    
    template<bool bIsUnsigned>
    struct DBStatementField<EFieldType::TYPE_SHORT, bIsUnsigned, sizeof( uint16_t )> 
    {
        static constexpr EFieldType FieldType  = EFieldType::TYPE_SHORT;
        static constexpr size_t     BufferSize = sizeof( uint16_t );
        using ValueType                        = std::conditional_t<bIsUnsigned, uint16_t, int16_t>;

        SKL_FORCEINLINE ValueType& GetValue() noexcept { return Value; }
        SKL_FORCEINLINE const ValueType& GetValue() const noexcept { return Value; }
        SKL_FORCEINLINE void Reset() noexcept { Value = 0; }

        ValueType Value{ 0 };
    };
    
    template<bool bIsUnsigned>
    struct DBStatementField<EFieldType::TYPE_LONG, bIsUnsigned, sizeof( uint32_t )> 
    {
        static constexpr EFieldType FieldType  = EFieldType::TYPE_LONG;
        static constexpr size_t     BufferSize = sizeof( uint32_t );
        using ValueType                        = std::conditional_t<bIsUnsigned, uint32_t, int32_t>;

        SKL_FORCEINLINE ValueType& GetValue() noexcept { return Value; }
        SKL_FORCEINLINE const ValueType& GetValue() const noexcept { return Value; }
        SKL_FORCEINLINE void Reset() noexcept { Value = 0; }

        ValueType Value{ 0 };
    };
    
    template<bool bIsUnsigned>
    struct DBStatementField<EFieldType::TYPE_LONGLONG, bIsUnsigned, sizeof( uint64_t )> 
    {
        static constexpr EFieldType FieldType  = EFieldType::TYPE_LONG;
        static constexpr size_t     BufferSize = sizeof( uint64_t );
        using ValueType                        = std::conditional_t<bIsUnsigned, uint64_t, int64_t>;

        SKL_FORCEINLINE ValueType& GetValue() noexcept { return Value; }
        SKL_FORCEINLINE const ValueType& GetValue() const noexcept { return Value; }
        SKL_FORCEINLINE void Reset() noexcept { Value = 0; }

        ValueType Value{ 0 };
    };
    
    template<>
    struct DBStatementField<EFieldType::TYPE_BIT, false, sizeof( bool )> 
    {
        static constexpr EFieldType FieldType  = EFieldType::TYPE_BIT;
        static constexpr size_t     BufferSize = sizeof( bool );
        using ValueType                        = bool;

        SKL_FORCEINLINE ValueType& GetValue() noexcept { return Value; }
        SKL_FORCEINLINE const ValueType& GetValue() const noexcept { return Value; }
        SKL_FORCEINLINE void Reset() noexcept { Value = false; }

        ValueType Value{ 0.0f };
    };
    
    template<>
    struct DBStatementField<EFieldType::TYPE_FLOAT, false, sizeof( float )> 
    {
        static constexpr EFieldType FieldType  = EFieldType::TYPE_FLOAT;
        static constexpr size_t     BufferSize = sizeof( float );
        using ValueType                        = float;

        SKL_FORCEINLINE ValueType& GetValue() noexcept { return Value; }
        SKL_FORCEINLINE const ValueType& GetValue() const noexcept { return Value; }
        SKL_FORCEINLINE void Reset() noexcept { Value = 0.0f; }

        ValueType Value{ 0.0f };
    };
    
    template<>
    struct DBStatementField<EFieldType::TYPE_DOUBLE, false, sizeof( double )> 
    {
        static constexpr EFieldType FieldType  = EFieldType::TYPE_DOUBLE;
        static constexpr size_t     BufferSize = sizeof( double );
        using ValueType                        = double;

        SKL_FORCEINLINE ValueType& GetValue() noexcept { return Value; }
        SKL_FORCEINLINE const ValueType& GetValue() const noexcept { return Value; }
        SKL_FORCEINLINE void Reset() noexcept { Value = 0.0; }

        ValueType Value{ 0.0 };
    };
    
    template<>
    struct DBStatementField<EFieldType::TYPE_TIMESTAMP, false, sizeof( DBTimeStamp )> 
    {
        static constexpr EFieldType FieldType  = EFieldType::TYPE_TIMESTAMP;
        static constexpr size_t     BufferSize = sizeof( DBTimeStamp );
        using ValueType                        = DBTimeStamp;

        SKL_FORCEINLINE ValueType& GetValue() noexcept { return Value; }
        SKL_FORCEINLINE const ValueType& GetValue() const noexcept { return Value; }
        SKL_FORCEINLINE void Reset() noexcept { memset( &Value, 0, sizeof( ValueType ) ); }

        ValueType Value{};
    };
    
    template<>
    struct DBStatementField<EFieldType::TYPE_DATE, false, sizeof( DBDate )> 
    {
        static constexpr EFieldType FieldType  = EFieldType::TYPE_DATE;
        static constexpr size_t     BufferSize = sizeof( DBDate );
        using ValueType                        = DBDate;

        SKL_FORCEINLINE ValueType& GetValue() noexcept { return Value; }
        SKL_FORCEINLINE const ValueType& GetValue() const noexcept { return Value; }
        SKL_FORCEINLINE void Reset() noexcept { memset( &Value, 0, sizeof( ValueType ) ); }

        ValueType Value{};
    };
    
    template<>
    struct DBStatementField<EFieldType::TYPE_TIME, false, sizeof( DBTime )> 
    {
        static constexpr EFieldType FieldType  = EFieldType::TYPE_TIME;
        static constexpr size_t     BufferSize = sizeof( DBTime );
        using ValueType                        = DBTime;

        SKL_FORCEINLINE ValueType& GetValue() noexcept { return Value; }
        SKL_FORCEINLINE const ValueType& GetValue() const noexcept { return Value; }
        SKL_FORCEINLINE void Reset() noexcept { memset( &Value, 0, sizeof( ValueType ) ); }

        ValueType Value{};
    };
    
    template<>
    struct DBStatementField<EFieldType::TYPE_TIME2, false, sizeof( DBTime2 )> 
    {
        static constexpr EFieldType FieldType  = EFieldType::TYPE_TIME2;
        static constexpr size_t     BufferSize = sizeof( DBTime2 );
        using ValueType                        = DBTime2;

        SKL_FORCEINLINE ValueType& GetValue() noexcept { return Value; }
        SKL_FORCEINLINE const ValueType& GetValue() const noexcept { return Value; }
        SKL_FORCEINLINE void Reset() noexcept { memset( &Value, 0, sizeof( ValueType ) ); }

        ValueType Value{};
    };

    template<>
    struct DBStatementField<EFieldType::TYPE_DATETIME, false, sizeof( DBDateTime )> 
    {
        static constexpr EFieldType FieldType  = EFieldType::TYPE_DATETIME;
        static constexpr size_t     BufferSize = sizeof( DBDateTime );
        using ValueType                        = DBDateTime;

        SKL_FORCEINLINE ValueType& GetValue() noexcept { return Value; }
        SKL_FORCEINLINE const ValueType& GetValue() const noexcept { return Value; }
        SKL_FORCEINLINE void Reset() noexcept { memset( &Value, 0, sizeof( ValueType ) ); }

        ValueType Value{};
    };
    
    template<>
    struct DBStatementField<EFieldType::TYPE_DATETIME2, false, sizeof( DBDateTime2 )> 
    {
        static constexpr EFieldType FieldType  = EFieldType::TYPE_DATETIME2;
        static constexpr size_t     BufferSize = sizeof( DBDateTime2 );
        using ValueType                        = DBDateTime2;

        SKL_FORCEINLINE ValueType& GetValue() noexcept { return Value; }
        SKL_FORCEINLINE const ValueType& GetValue() const noexcept { return Value; }
        SKL_FORCEINLINE void Reset() noexcept { memset( &Value, 0, sizeof( ValueType ) ); }

        ValueType Value{};
    };
    
    template<>
    struct DBStatementField<EFieldType::TYPE_TIMESTAMP2, false, sizeof( DBTimeStamp2 )> 
    {
        static constexpr EFieldType FieldType  = EFieldType::TYPE_TIMESTAMP2;
        static constexpr size_t     BufferSize = sizeof( DBTimeStamp2 );
        using ValueType                        = DBTimeStamp2;

        SKL_FORCEINLINE ValueType& GetValue() noexcept { return Value; }
        SKL_FORCEINLINE const ValueType& GetValue() const noexcept { return Value; }
        SKL_FORCEINLINE void Reset() noexcept { memset( &Value, 0, sizeof( ValueType ) ); }

        ValueType Value{};
    };
    
    template<>
    struct DBStatementField<EFieldType::TYPE_ENUM, false, sizeof( int32_t )> 
    {
        static constexpr EFieldType FieldType  = EFieldType::TYPE_ENUM;
        static constexpr size_t     BufferSize = sizeof( int32_t );
        using ValueType                        = int32_t;

        SKL_FORCEINLINE ValueType& GetValue() noexcept { return Value; }
        SKL_FORCEINLINE const ValueType& GetValue() const noexcept { return Value; }
        SKL_FORCEINLINE void Reset() noexcept { Value = 0; }

        ValueType Value{};
    };
    
    template<>
    struct DBStatementField<EFieldType::TYPE_TINY_BLOB, false, CTinyBlobMaximumSize> 
    {
        static constexpr EFieldType FieldType  = EFieldType::TYPE_TINY_BLOB;
        static constexpr size_t     BufferSize = CTinyBlobMaximumSize;
        using ValueType                        = char*;

        DBStatementField() noexcept
        {
            Value = reinterpret_cast<char*>( SKL_MALLOC_ALIGNED( CTinyBlobMaximumSize, SKL_ALIGNMENT ) );
            SKL_ASSERT( nullptr != Value );
            memset( Value, 0, CTinyBlobMaximumSize );
        }
        ~DBStatementField() noexcept
        {
            if( nullptr != Value )
            {
                SKL_FREE_ALIGNED( Value, SKL_ALIGNMENT );
                Value = nullptr;
            }
        }

        SKL_FORCEINLINE ValueType GetValue() const noexcept { return Value; }
        SKL_FORCEINLINE void Reset() noexcept { memset( Value, 0, CTinyBlobMaximumSize ); }

        ValueType Value{ nullptr };
    };
    
    template<>
    struct DBStatementField<EFieldType::TYPE_BLOB, false, CBlobMaximumSize> 
    {
        static constexpr EFieldType FieldType  = EFieldType::TYPE_BLOB;
        static constexpr size_t     BufferSize = CBlobMaximumSize;
        using ValueType                        = char*;

        DBStatementField() noexcept
        {
            Value = reinterpret_cast<char*>( SKL_MALLOC_ALIGNED( CBlobMaximumSize, SKL_ALIGNMENT ) );
            SKL_ASSERT( nullptr != Value );
            memset( Value, 0, CBlobMaximumSize );
        }
        ~DBStatementField() noexcept
        {
            if( nullptr != Value )
            {
                SKL_FREE_ALIGNED( Value, SKL_ALIGNMENT );
                Value = nullptr;
            }
        }

        SKL_FORCEINLINE ValueType GetValue() const noexcept { return Value; }
        SKL_FORCEINLINE void Reset() noexcept { memset( Value, 0, CBlobMaximumSize ); }

        ValueType Value{ nullptr };
    };
    
    template<>
    struct DBStatementField<EFieldType::TYPE_MEDIUM_BLOB, false, CMediumBlobMaximumSize> 
    {
        static constexpr EFieldType FieldType  = EFieldType::TYPE_MEDIUM_BLOB;
        static constexpr size_t     BufferSize = CMediumBlobMaximumSize;
        using ValueType                        = char*;

        DBStatementField() noexcept
        {
            Value = reinterpret_cast<char*>( SKL_MALLOC_ALIGNED( CMediumBlobMaximumSize, SKL_ALIGNMENT ) );
            SKL_ASSERT( nullptr != Value );
            memset( Value, 0, CMediumBlobMaximumSize );
        }
        ~DBStatementField() noexcept
        {
            if( nullptr != Value )
            {
                SKL_FREE_ALIGNED( Value, SKL_ALIGNMENT );
                Value = nullptr;
            }
        }

        SKL_FORCEINLINE ValueType GetValue() const noexcept { return Value; }
        SKL_FORCEINLINE void Reset() noexcept { memset( Value, 0, CMediumBlobMaximumSize ); }

        ValueType Value{ nullptr };
    };

    template<size_t InMaxBufferSize>
    struct DBStatementField<EFieldType::TYPE_VAR_STRING, false, InMaxBufferSize>
    {
        static constexpr EFieldType FieldType  = EFieldType::TYPE_VAR_STRING;
        static constexpr size_t     BufferSize = InMaxBufferSize;
        using ValueType                        = DBString<InMaxBufferSize>;

        SKL_FORCEINLINE ValueType& GetValue() noexcept { return Value; }
        SKL_FORCEINLINE const ValueType& GetValue() const noexcept { return Value; }
        SKL_FORCEINLINE void Reset() noexcept { Value.Clear(); }

        ValueType Value{};
    };
    
    template<size_t InMaxBufferSize>
    struct DBStatementField<EFieldType::TYPE_STRING, false, InMaxBufferSize>
    {
        static constexpr EFieldType FieldType  = EFieldType::TYPE_STRING;
        static constexpr size_t     BufferSize = InMaxBufferSize;
        using ValueType                        = DBString<InMaxBufferSize>;

        SKL_FORCEINLINE ValueType& GetValue() noexcept { return Value; }
        SKL_FORCEINLINE const ValueType& GetValue() const noexcept { return Value; }
        SKL_FORCEINLINE void Reset() noexcept { Value.Clear(); }

        ValueType Value{};
    };

    using DBDecimalField     = DBStatementField<EFieldType::TYPE_DECIMAL, false, sizeof( double )>;
    using DBInt8Field        = DBStatementField<EFieldType::TYPE_TINY, false, sizeof( int8_t )>;
    using DBUInt8Field       = DBStatementField<EFieldType::TYPE_TINY, true, sizeof( int8_t )>;
    using DBInt16Field       = DBStatementField<EFieldType::TYPE_SHORT, false, sizeof( int16_t )>;
    using DBUInt16Field      = DBStatementField<EFieldType::TYPE_SHORT, true, sizeof( int16_t )>;
    using DBInt32Field       = DBStatementField<EFieldType::TYPE_LONG, false, sizeof( int32_t )>;
    using DBUInt32Field      = DBStatementField<EFieldType::TYPE_LONG, true, sizeof( int32_t )>;
    using DBInt64Field       = DBStatementField<EFieldType::TYPE_LONGLONG, false, sizeof( int64_t )>;
    using DBUInt64Field      = DBStatementField<EFieldType::TYPE_LONGLONG, true, sizeof( int64_t )>;
    using DBBoolField        = DBStatementField<EFieldType::TYPE_BIT, false, sizeof( bool )>;
    using DBFloatField       = DBStatementField<EFieldType::TYPE_FLOAT, false, sizeof( float )>;
    using DBDoubleField      = DBStatementField<EFieldType::TYPE_DOUBLE, false, sizeof( double )>;
    using DBTimeStampField   = DBStatementField<EFieldType::TYPE_TIMESTAMP, false, sizeof( DBTimeStamp )>;
    using DBTimeStamp2Field  = DBStatementField<EFieldType::TYPE_TIMESTAMP2, false, sizeof( DBTimeStamp2 )>;
    using DBDateField        = DBStatementField<EFieldType::TYPE_DATE, false, sizeof( DBDate )>;
    using DBTimeField        = DBStatementField<EFieldType::TYPE_TIME, false, sizeof( DBTime )>;
    using DBTime2Field       = DBStatementField<EFieldType::TYPE_TIME2, false, sizeof( DBTime2 )>;
    using DBDateTimeField    = DBStatementField<EFieldType::TYPE_DATETIME, false, sizeof( DBDateTime )>;
    using DBDateTime2Field   = DBStatementField<EFieldType::TYPE_DATETIME, false, sizeof( DBDateTime2 )>;
    using DBEnumField        = DBStatementField<EFieldType::TYPE_ENUM, false, sizeof( int32_t )>;
    using DBTinyBlobField    = DBStatementField<EFieldType::TYPE_TINY_BLOB, false, CTinyBlobMaximumSize>;
    using DBBlobField        = DBStatementField<EFieldType::TYPE_BLOB, false, CBlobMaximumSize>;
    using DBMediumBlobField  = DBStatementField<EFieldType::TYPE_MEDIUM_BLOB, false, CMediumBlobMaximumSize>;
    template<size_t InMaxBufferSize>
    using DBVarStringField   = DBStatementField<EFieldType::TYPE_VAR_STRING, false, InMaxBufferSize>;
    template<size_t InMaxBufferSize>
    using DBStringField      = DBStatementField<EFieldType::TYPE_STRING, false, InMaxBufferSize>;
}