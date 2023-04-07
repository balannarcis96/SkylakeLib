//!
//! \file Logger.h
//! 
//! \brief Fast and lightweight logger abstraction for SkylakeLib
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    extern std::relaxed_value<FILE*> GLogOutput;
    using TSerializedLogHandler = ASD::TrivialFunctorWrapper<32, void(*)(BufferStream&) noexcept>;

    enum ELogType: uint8_t
    {
          ELogInfo
        , ELogWarning
        , ELogError
        , ELogFatal
    };

    enum ELogParamType: uint8_t
    {
          None
        , EInt8
        , EInt16
        , EInt32
        , EInt64
        , EUInt8
        , EUInt16
        , EUInt32
        , EUInt64
        , EFloat
        , EDouble
        , EString
        , EWString
    };

    class SerializedSkylakeLogger 
    {
    public:
        template<ELogType Type, size_t FormatStringSize, typename ...TArgs>
        SKL_FORCEINLINE void Log( const char( &FormatString )[FormatStringSize], TArgs... Args ) noexcept
        {
            ValidateArgumentTypes<TArgs...>();

            SKL_ASSERT_MSG( StringUtils::IsValidForCurrentThread(), "In order to use SerializedSkylakeLogger::Log, the calling thread must have a instanciated StringUtils!" );

            auto& Stream = StringUtils::GetBuffer();
            if( Serialize<Type>( Stream, FormatString, Args... ) ) SKL_ALLWAYS_LIKELY
            {
                LogHandler( Stream );
            }
        }

        template<typename TFunctor>
        void SetLogHandler( TFunctor&& InFunctor ) noexcept
        {
            LogHandler.SetFunctor( std::forward<TFunctor>( InFunctor ) );
        }

        SKL_NODISCARD bool HasHandler() const noexcept { return false == LogHandler.IsNull(); }

    private:
        template<typename TType, typename TCharType>
        static consteval bool IsString() noexcept
        {
            return ( std::is_pointer_v<TType> || std::is_array_v<TType> ) && std::is_same_v<std::remove_cvref_t<std::remove_all_extents_t<std::remove_pointer_t<TType>>>, TCharType>;
        }

        template<typename ...TArgs>
        static consteval void ValidateArgumentTypes() noexcept
        {
            if constexpr( sizeof...( TArgs ) != 0 )
            {
                ValidateArgumentTypesInternal<TArgs...>();
            }
        }

        template<typename TArg, typename ...TArgs>
        static consteval void ValidateArgumentTypesInternal() noexcept
        {
            static_assert( ELogParamType::None != GetArgType<TArg>(), "Type not supported!" );
            
            if constexpr( sizeof...( TArgs ) != 0 )
            {
                ValidateArgumentTypesInternal<TArgs...>();
            }
        }

        template<ELogType Type, size_t FormatStringSize, typename ...TArgs>
        static bool Serialize( BufferStream& Stream, const char( &FormatString )[FormatStringSize], TArgs... Args ) noexcept
        {
            constexpr size_t BytesAvailableForBody = std::numeric_limits<uint16_t>::max() - 3;
            static_assert( FormatStringSize <= BytesAvailableForBody );
            
            const size_t BytesNeededForArgs = CalculateBytesNeededForArgs( Args... );
            if ( ( BytesNeededForArgs + FormatStringSize + sizeof( uint16_t ) ) > BytesAvailableForBody )
            {
                SKL_ASSERT( false );
                return false;
            }

            Stream.Reset();

            //1. Header (3 bytes)
            Stream.WriteT<uint16_t>( 0 );   // Size placeholder
            Stream.WriteT<uint8_t>( Type ); //LogType
            
            //1. Format string
            Stream.WriteT<uint16_t>( static_cast<uint16_t>( FormatStringSize ) );
            Stream.WriteString( FormatString );
            
            //2. Write Params if any
            SerializeArgument( Stream, Args... );

            //3. Update size
            const auto TotalSize = Stream.GetPosition();
            *reinterpret_cast<uint16_t*>( Stream.GetBuffer() ) = TotalSize;

            return true;
        }

        template<typename ...TArgs>
        static constexpr void SerializeArgument( BufferStream& Stream, TArgs... Args ) noexcept
        {
            if constexpr( sizeof...( TArgs ) == 0 )
            {
                return;           
            }
            else
            {
                return SerializeArgumentInternal( Stream, Args... );
            }
        }

        template<typename TArg, typename ...TArgs>
        static void SerializeArgumentInternal( BufferStream& Stream, TArg Arg, TArgs... Args ) noexcept
        {
            constexpr bool bIsString = IsString<TArg, char>();
            constexpr bool bIsWString = IsString<TArg, wchar_t>();
            constexpr ELogParamType TArgType = GetArgType<TArg>();

            //1. Write type
            Stream.WriteT<uint8_t>( static_cast<uint8_t>( TArgType ) );

            //2. Write value
            if constexpr( bIsWString )
            {
                const auto bWriteResult = Stream.Write( reinterpret_cast<const uint8_t*>( Arg ), static_cast<uint32_t>( ::wcslen( Arg ) * 2U ) + 2U );
                SKL_ASSERT( bWriteResult );
                ( void )bWriteResult;
            }
            else if constexpr( bIsString )
            {
                const auto bWriteResult = Stream.Write( reinterpret_cast<const uint8_t*>( Arg ), static_cast<uint32_t>( ::strlen( Arg ) ) + 1U );
                SKL_ASSERT( bWriteResult );
                ( void )bWriteResult;
            }
            else 
            {
                Stream.WriteT<std::remove_all_extents_t<TArg>>( Arg );
            }

            // Next arg if any
            SerializeArgument( Stream, Args... );
        }
        
        template<typename ...TArgs>
        static constexpr size_t CalculateBytesNeededForArgs( TArgs... Args ) noexcept
        {
            if constexpr( sizeof...( TArgs ) == 0 )
            {
                return 0U;
            }
            else
            {
                return CalculateBytesNeededForArgsInternal( Args... );
            }
        }

        template<typename TArg, typename ...TArgs>
        static constexpr size_t CalculateBytesNeededForArgsInternal( TArg Arg,  TArgs... Args ) noexcept
        {
            constexpr ELogParamType TArgType = GetArgType<TArg>();

            size_t TArgSize = 1U; // Type byte

            if constexpr( TArgType == ELogParamType::EString )
            {
                TArgSize += ::strnlen_s( Arg, std::numeric_limits<uint16_t>::max() ) + 1U; // Include null-terminator
            }
            else if constexpr( TArgType == ELogParamType::EWString )
            {
                TArgSize += ( ::wcsnlen_s( Arg, std::numeric_limits<uint16_t>::max() ) * 2U ) + 2U;// Include null-terminator
            }
            else
            {
                TArgSize += sizeof( std::remove_all_extents_t<TArg> );
            }

            if constexpr( sizeof...( TArgs ) == 0 )
            {
                return TArgSize;
            }
            else
            {
                return TArgSize + CalculateBytesNeededForArgsInternal( Args... );
            }
        }

        template<typename TArg>
        static consteval ELogParamType GetArgType() noexcept
        {
            using TType = std::remove_all_extents_t<TArg>;

            if constexpr( IsString<TArg, char>() )
            {
                return ELogParamType::EString;
            }
            else if constexpr( IsString<TArg, wchar_t>() )
            {
                return ELogParamType::EWString;
            }
            else if constexpr ( std::is_same_v<TType, int8_t> )
            {
                return ELogParamType::EInt8;
            }
            else if constexpr ( std::is_same_v<TType, uint8_t> )
            {
                return ELogParamType::EUInt8;
            }
            else if constexpr ( std::is_same_v<TType, int16_t> )
            {
                return ELogParamType::EInt16;
            }
            else if constexpr ( std::is_same_v<TType, uint16_t> )
            {
                return ELogParamType::EUInt16;
            }
            else if constexpr ( std::is_same_v<TType, int32_t> )
            {
                return ELogParamType::EInt32;
            }
            else if constexpr ( std::is_same_v<TType, uint32_t> )
            {
                return ELogParamType::EUInt32;
            }
            else if constexpr ( std::is_same_v<TType, int64_t> )
            {
                return ELogParamType::EInt64;
            }
            else if constexpr ( std::is_same_v<TType, uint64_t> )
            {
                return ELogParamType::EUInt64;
            }
            else if constexpr ( std::is_same_v<TType, float> )
            {
                return ELogParamType::EFloat;
            }
            else if constexpr ( std::is_same_v<TType, double> )
            {
                return ELogParamType::EDouble;
            }
            else
            {
                return ELogParamType::None;
            }
        }

    private:
        TSerializedLogHandler LogHandler{};

        SerializedSkylakeLogger() = default;

        template<bool bSerializedOrLocal>
        friend class SkylakeLogger;
    };
    
    class LocalSkylakeLogger 
    {
    public:
        template<ELogType Type, size_t FormatStringSize, typename ...TArgs>
        SKL_FORCEINLINE void Log( const char( &FormatString )[FormatStringSize], TArgs... Args ) noexcept
        {
            if constexpr( sizeof...( TArgs ) == 0 )
            {
                ( void )::fputs( FormatString, Output );
            }
            else
            {
                ( void )::fprintf( Output, FormatString, Args... );
            }

            ( void )::fputs( "\n", Output );
        }

        SKL_FORCEINLINE void SetOutput( FILE* InOutput ) noexcept { ( void )Output.exchange( InOutput ); }

    private:
        LocalSkylakeLogger() noexcept = default;

        std::relaxed_value<FILE*> Output{ GLogOutput.load() };

        template<bool bSerializedOrLocal>
        friend class SkylakeLogger;
    };

    template<bool bSerializedOrLocal = true>
    class SkylakeLogger: public std::conditional_t<bSerializedOrLocal, SerializedSkylakeLogger, LocalSkylakeLogger>
    {
    public:
        using Super = std::conditional_t<bSerializedOrLocal, SerializedSkylakeLogger, LocalSkylakeLogger>;

        SkylakeLogger() noexcept
            : Super()
        {
            
        }

        template<ELogType Type, size_t FormatStringSize, typename ...TArgs>
        SKL_FORCEINLINE void Log( const char( &FormatString )[FormatStringSize], TArgs... Args ) noexcept
        {
            Super::template Log<Type>( FormatString, Args... );
        }
    };

    #if defined(__clang__) || defined(__GNUC__)
        #define SKL_LOGGER_FUNCTION __PRETTY_FUNCTION__
    #elif defined(_MSC_VER)
        #define SKL_LOGGER_FUNCTION __FUNCSIG__
    #else   
        #define SKL_LOGGER_FUNCTION "#FuncSig Not Supported#"
    #endif

    #define SLOG_INFO( Logger, FormatString, ... ) \
        Logger.Log<SKL::ELogInfo>( FormatString, ##__VA_ARGS__ )
        
    #define SLOG_WARNING( Logger, FormatString, ... ) \
        Logger.Log<SKL::ELogWarning>( "[WARNING] " FormatString, ##__VA_ARGS__ )

    #define SLOG_ERROR( Logger, FormatString, ... ) \
        Logger.Log<SKL::ELogError>( "[ERROR] " FormatString, ##__VA_ARGS__ )

    #define SLOG_FATAL( Logger, FormatString, ... ) \
        Logger.Log<SKL::ELogFatal>( "[FATAL] " FormatString, ##__VA_ARGS__ )

    #define STRACE_INFO( Logger, FormatString, ... ) \
        Logger.Log<SKL::ELogInfo>( "[" __FILE__ "][%s:%u] " FormatString, SKL_LOGGER_FUNCTION, __LINE__, ##__VA_ARGS__ )
        
    #define STRACE_WARNING( Logger, FormatString, ... ) \
        Logger.Log<SKL::ELogWarning>( "[" __FILE__ "][%s:%u][WARNING] " FormatString, SKL_LOGGER_FUNCTION, __LINE__, ##__VA_ARGS__ )
        
    #define STRACE_ERROR( Logger, FormatString, ... ) \
        Logger.Log<SKL::ELogError>( "[" __FILE__ "][%s:%u][ERROR] " FormatString, SKL_LOGGER_FUNCTION, __LINE__, ##__VA_ARGS__ )
        
    #define STRACE_FATAL( Logger, FormatString, ... ) \
        Logger.Log<SKL::ELogFatal>( "[" __FILE__ "][%s:%u][FATAL] " FormatString , SKL_LOGGER_FUNCTION, __LINE__, ##__VA_ARGS__ )
}

namespace SKL
{
    extern SkylakeLogger<CSkylakeUserSerializedLogger> GLogger;

    #define GLOG_INFO( FormatString, ... ) SLOG_INFO( GLogger, FormatString, ##__VA_ARGS__ )
        
    #define GLOG_WARNING( FormatString, ... ) SLOG_WARNING( GLogger, FormatString, ##__VA_ARGS__ )

    #define GLOG_ERROR( FormatString, ... ) SLOG_ERROR( GLogger, FormatString, ##__VA_ARGS__ )

    #define GLOG_FATAL( FormatString, ... ) SLOG_FATAL( GLogger, FormatString, ##__VA_ARGS__ )

    #define GTRACE_INFO( FormatString, ... ) STRACE_INFO( GLogger, FormatString, ##__VA_ARGS__ )
        
    #define GTRACE_WARNING( FormatString, ... ) STRACE_WARNING( GLogger, FormatString, ##__VA_ARGS__ )
        
    #define GTRACE_ERROR( FormatString, ... ) STRACE_ERROR( GLogger, FormatString, ##__VA_ARGS__ )
        
    #define GTRACE_FATAL( FormatString, ... ) STRACE_FATAL( GLogger, FormatString, ##__VA_ARGS__ )
}