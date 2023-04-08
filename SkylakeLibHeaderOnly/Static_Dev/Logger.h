//!
//! \file Logger.h
//! 
//! \brief Fast and lightweight logger abstraction for SkylakeLib
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

#define SKLL_LOG_LEVEL_DEBUG 1
#define SKLL_LOG_LEVEL_INFO 2
#define SKLL_LOG_LEVEL_WARNING 3
#define SKLL_LOG_LEVEL_ERROR 4
#define SKLL_LOG_LEVEL_FATAL 5
#define SKLL_LOG_LEVEL_MUTE 6

namespace SKL
{
    using TSerializedLogHandler = ASD::TrivialFunctorWrapper<32, void(*)( BinaryStream& ) noexcept>;

    enum ELogType: uint8_t
    {
          ELogDebug
        , ELogInfo
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

    class SerializedSkylakeLogger;

    #if defined(SKL_USE_SERIALIZED_LOGGER)
    class SerializedSkylakeLogger 
    {
    public:
        template<ELogType Type, size_t FormatStringSize, typename ...TArgs>
        SKL_FORCEINLINE void Log( const char( &FormatString )[FormatStringSize], TArgs... Args ) noexcept
        {
            ValidateArgumentTypes<TArgs...>();

            if( Serialize<Type>( WorkingStream, FormatString, Args... ) ) SKL_ALLWAYS_LIKELY
            {
                LogHandler( WorkingStream );
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
        static bool Serialize( BinaryStream& Stream, const char( &FormatString )[FormatStringSize], TArgs... Args ) noexcept
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
        static constexpr void SerializeArgument( BinaryStream& Stream, TArgs... Args ) noexcept
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
        static void SerializeArgumentInternal( BinaryStream& Stream, TArg Arg, TArgs... Args ) noexcept
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
        TSerializedLogHandler            LogHandler{};
        static thread_local BinaryStream WorkingStream;

        SerializedSkylakeLogger() noexcept
        {
            SetLogHandler( []( BinaryStream& /*InStream*/ ) noexcept -> void
            {
                ( void )::printf( "NO HANDLER WAS SET FOR THE GLOBAL LOGGER!\nSee [%s]:[%s:%d]!\n", __FILE__, SKL_FUNCTION_SIG, __LINE__ );
            } );
        }

        template<bool bSerializedOrLocal>
        friend class SkylakeLogger;
    };
    #endif
    
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

        std::relaxed_value<FILE*> Output{ stdout }; //!< Output file stream, defaults to stdout

        template<bool bSerializedOrLocal>
        friend class SkylakeLogger;
    };

    template<bool bSerializedOrLocal = true>
    class SkylakeLogger: public std::conditional_t<bSerializedOrLocal, SerializedSkylakeLogger, LocalSkylakeLogger>
    {
    public:
        using Super = std::conditional_t<bSerializedOrLocal, SerializedSkylakeLogger, LocalSkylakeLogger>;

        SkylakeLogger() noexcept: Super() { }

        template<ELogType Type, size_t FormatStringSize, typename ...TArgs>
        SKL_NOINLINE void Log( const char( &FormatString )[FormatStringSize], TArgs... Args ) noexcept
        {
            if( LogLevel <= Type )
            {
                Super::template Log<Type>( FormatString, Args... );
            }
        }

        SKL_FORCEINLINE SKL_NODISCARD ELogType GetLogLevel() const noexcept { return LogLevel; }

    private:
        std::relaxed_value<ELogType> LogLevel{ ELogType::ELogInfo };
    };

    #define _SLOG_DEBUG( Logger, FormatString, ... ) \
        Logger.Log<SKL::ELogDebug>( FormatString, ##__VA_ARGS__ )
        
    #define _SLOG_INFO( Logger, FormatString, ... ) \
        Logger.Log<SKL::ELogInfo>( FormatString, ##__VA_ARGS__ )
        
    #define _SLOG_WARNING( Logger, FormatString, ... ) \
        Logger.Log<SKL::ELogWarning>( "[WARNING] " FormatString, ##__VA_ARGS__ )

    #define _SLOG_ERROR( Logger, FormatString, ... ) \
        Logger.Log<SKL::ELogError>( "[ERROR] " FormatString, ##__VA_ARGS__ )

    #define _SLOG_FATAL( Logger, FormatString, ... ) \
        Logger.Log<SKL::ELogFatal>( "[FATAL] " FormatString, ##__VA_ARGS__ )
        
    #define _STRACE_DEBUG( Logger, FormatString, ... ) \
        Logger.Log<SKL::ELogDebug>( "[" __FILE__ "][%s:%u] " FormatString, SKL_FUNCTION_SIG, __LINE__, ##__VA_ARGS__ )
        
    #define _STRACE_INFO( Logger, FormatString, ... ) \
        Logger.Log<SKL::ELogInfo>( "[" __FILE__ "][%s:%u] " FormatString, SKL_FUNCTION_SIG, __LINE__, ##__VA_ARGS__ )
        
    #define _STRACE_WARNING( Logger, FormatString, ... ) \
        Logger.Log<SKL::ELogWarning>( "[" __FILE__ "][%s:%u][WARNING] " FormatString, SKL_FUNCTION_SIG, __LINE__, ##__VA_ARGS__ )
        
    #define _STRACE_ERROR( Logger, FormatString, ... ) \
        Logger.Log<SKL::ELogError>( "[" __FILE__ "][%s:%u][ERROR] " FormatString, SKL_FUNCTION_SIG, __LINE__, ##__VA_ARGS__ )
        
    #define _STRACE_FATAL( Logger, FormatString, ... ) \
        Logger.Log<SKL::ELogFatal>( "[" __FILE__ "][%s:%u][FATAL] " FormatString , SKL_FUNCTION_SIG, __LINE__, ##__VA_ARGS__ )
        #define SKLL_LOG_LEVEL_DEBUG 1
        

#if SKLL_LOG_LEVEL == SKLL_LOG_LEVEL_DEBUG
    #define SLOG_DEBUG( Logger, FormatString, ... )      _SLOG_DEBUG( Logger, FormatString, ##__VA_ARGS__ )
    #define SLOG_INFO( Logger, FormatString, ... )       _SLOG_INFO( Logger, FormatString, ##__VA_ARGS__ )
    #define SLOG_WARNING( Logger, FormatString, ... )    _SLOG_WARNING( Logger, FormatString, ##__VA_ARGS__ )
    #define SLOG_ERROR( Logger, FormatString, ... )      _SLOG_ERROR( Logger, FormatString, ##__VA_ARGS__ )
    #define SLOG_FATAL( Logger, FormatString, ... )      _SLOG_FATAL( Logger, FormatString, ##__VA_ARGS__ )
    #define STRACE_DEBUG( Logger, FormatString, ... )    _STRACE_DEBUG( Logger, FormatString, ##__VA_ARGS__ )
    #define STRACE_INFO( Logger, FormatString, ... )     _STRACE_INFO( Logger, FormatString, ##__VA_ARGS__ )
    #define STRACE_WARNING( Logger, FormatString, ... )  _STRACE_WARNING( Logger, FormatString, ##__VA_ARGS__ )
    #define STRACE_ERROR( Logger, FormatString, ... )    _STRACE_ERROR( Logger, FormatString, ##__VA_ARGS__ )
    #define STRACE_FATAL( Logger, FormatString, ... )    _STRACE_FATAL( Logger, FormatString, ##__VA_ARGS__ )
    
    #define SKLL_DEBUG_BLOCK( expr )    expr
    #define SKLL_INFO_BLOCK( expr )     expr
    #define SKLL_WARNING_BLOCK( expr ) expr
    #define SKLL_ERROR_BLOCK( expr )    expr
    #define SKLL_FATAL_BLOCK( expr )    expr
#elif SKLL_LOG_LEVEL == SKLL_LOG_LEVEL_INFO
    #define SLOG_DEBUG( Logger, FormatString, ... )      
    #define SLOG_INFO( Logger, FormatString, ... )       _SLOG_INFO( Logger, FormatString, ##__VA_ARGS__ )
    #define SLOG_WARNING( Logger, FormatString, ... )    _SLOG_WARNING( Logger, FormatString, ##__VA_ARGS__ )
    #define SLOG_ERROR( Logger, FormatString, ... )      _SLOG_ERROR( Logger, FormatString, ##__VA_ARGS__ )
    #define SLOG_FATAL( Logger, FormatString, ... )      _SLOG_FATAL( Logger, FormatString, ##__VA_ARGS__ )
    #define STRACE_DEBUG( Logger, FormatString, ... )    
    #define STRACE_INFO( Logger, FormatString, ... )     _STRACE_INFO( Logger, FormatString, ##__VA_ARGS__ )
    #define STRACE_WARNING( Logger, FormatString, ... )  _STRACE_WARNING( Logger, FormatString, ##__VA_ARGS__ )
    #define STRACE_ERROR( Logger, FormatString, ... )    _STRACE_ERROR( Logger, FormatString, ##__VA_ARGS__ )
    #define STRACE_FATAL( Logger, FormatString, ... )    _STRACE_FATAL( Logger, FormatString, ##__VA_ARGS__ )
    
    #define SKLL_DEBUG_BLOCK( expr )   
    #define SKLL_INFO_BLOCK( expr )     expr
    #define SKLL_WARNING_BLOCK( expr ) expr
    #define SKLL_ERROR_BLOCK( expr )    expr
    #define SKLL_FATAL_BLOCK( expr )    expr
#elif SKLL_LOG_LEVEL == SKLL_LOG_LEVEL_WARNING
    #define SLOG_DEBUG( Logger, FormatString, ... )     
    #define SLOG_INFO( Logger, FormatString, ... )      
    #define SLOG_WARNING( Logger, FormatString, ... )    _SLOG_WARNING( Logger, FormatString, ##__VA_ARGS__ )
    #define SLOG_ERROR( Logger, FormatString, ... )      _SLOG_ERROR( Logger, FormatString, ##__VA_ARGS__ )
    #define SLOG_FATAL( Logger, FormatString, ... )      _SLOG_FATAL( Logger, FormatString, ##__VA_ARGS__ )
    #define STRACE_DEBUG( Logger, FormatString, ... )   
    #define STRACE_INFO( Logger, FormatString, ... )    
    #define STRACE_WARNING( Logger, FormatString, ... )  _STRACE_WARNING( Logger, FormatString, ##__VA_ARGS__ )
    #define STRACE_ERROR( Logger, FormatString, ... )    _STRACE_ERROR( Logger, FormatString, ##__VA_ARGS__ )
    #define STRACE_FATAL( Logger, FormatString, ... )    _STRACE_FATAL( Logger, FormatString, ##__VA_ARGS__ )
    
    #define SKLL_DEBUG_BLOCK( expr )    
    #define SKLL_INFO_BLOCK( expr )     
    #define SKLL_WARNING_BLOCK( expr ) expr
    #define SKLL_ERROR_BLOCK( expr )    expr
    #define SKLL_FATAL_BLOCK( expr )    expr
#elif SKLL_LOG_LEVEL == SKLL_LOG_LEVEL_ERROR
    #define SLOG_DEBUG( Logger, FormatString, ... )      
    #define SLOG_INFO( Logger, FormatString, ... )       
    #define SLOG_WARNING( Logger, FormatString, ... )    
    #define SLOG_ERROR( Logger, FormatString, ... )      _SLOG_ERROR( Logger, FormatString, ##__VA_ARGS__ )
    #define SLOG_FATAL( Logger, FormatString, ... )      _SLOG_FATAL( Logger, FormatString, ##__VA_ARGS__ )
    #define STRACE_DEBUG( Logger, FormatString, ... )    
    #define STRACE_INFO( Logger, FormatString, ... )     
    #define STRACE_WARNING( Logger, FormatString, ... )  
    #define STRACE_ERROR( Logger, FormatString, ... )    _STRACE_ERROR( Logger, FormatString, ##__VA_ARGS__ )
    #define STRACE_FATAL( Logger, FormatString, ... )    _STRACE_FATAL( Logger, FormatString, ##__VA_ARGS__ )
    
    #define SKLL_DEBUG_BLOCK( expr )   
    #define SKLL_INFO_BLOCK( expr )    
    #define SKLL_WARNING_BLOCK( expr )
    #define SKLL_ERROR_BLOCK( expr )    expr
    #define SKLL_FATAL_BLOCK( expr )    expr
#elif SKLL_LOG_LEVEL == SKLL_LOG_LEVEL_FATAL
    #define SLOG_DEBUG( Logger, FormatString, ... )      
    #define SLOG_INFO( Logger, FormatString, ... )       
    #define SLOG_WARNING( Logger, FormatString, ... )    
    #define SLOG_ERROR( Logger, FormatString, ... )      
    #define SLOG_FATAL( Logger, FormatString, ... )      _SLOG_FATAL( Logger, FormatString, ##__VA_ARGS__ )
    #define STRACE_DEBUG( Logger, FormatString, ... )   
    #define STRACE_INFO( Logger, FormatString, ... )    
    #define STRACE_WARNING( Logger, FormatString, ... ) 
    #define STRACE_ERROR( Logger, FormatString, ... )   
    #define STRACE_FATAL( Logger, FormatString, ... )    _STRACE_FATAL( Logger, FormatString, ##__VA_ARGS__ )
    
    #define SKLL_DEBUG_BLOCK( expr )    
    #define SKLL_INFO_BLOCK( expr )     
    #define SKLL_WARNING_BLOCK( expr ) 
    #define SKLL_ERROR_BLOCK( expr )    
    #define SKLL_FATAL_BLOCK( expr )    expr
#else
    // Muted
    #define SLOG_DEBUG( Logger, FormatString, ... )
    #define SLOG_INFO( Logger, FormatString, ... ) 
    #define SLOG_WARNING( Logger, FormatString, ... ) 
    #define SLOG_ERROR( Logger, FormatString, ... ) 
    #define SLOG_FATAL( Logger, FormatString, ... ) 
    #define STRACE_DEBUG( Logger, FormatString, ... ) 
    #define STRACE_INFO( Logger, FormatString, ... ) 
    #define STRACE_WARNING( Logger, FormatString, ... ) 
    #define STRACE_ERROR( Logger, FormatString, ... ) 
    #define STRACE_FATAL( Logger, FormatString, ... )
    
    #define SKLL_DEBUG_BLOCK( expr )    
    #define SKLL_INFO_BLOCK( expr )     
    #define SKLL_WARNING_BLOCK( expr ) 
    #define SKLL_ERROR_BLOCK( expr )    
    #define SKLL_FATAL_BLOCK( expr )  
#endif
}

namespace SKL
{
#if SKLL_LOG_LEVEL < SKLL_LOG_LEVEL_MUTE
    extern SkylakeLogger<CSkylakeUserSerializedLogger> GLogger;
#endif
}

#if SKLL_LOG_LEVEL < SKLL_LOG_LEVEL_MUTE

    #define GLOG_DEBUG( FormatString, ... ) SLOG_DEBUG( SKL::GLogger, FormatString, ##__VA_ARGS__ )
    #define GLOG_INFO( FormatString, ... ) SLOG_INFO( SKL::GLogger, FormatString, ##__VA_ARGS__ )
    #define GLOG_WARNING( FormatString, ... ) SLOG_WARNING( SKL::GLogger, FormatString, ##__VA_ARGS__ )
    #define GLOG_ERROR( FormatString, ... ) SLOG_ERROR( SKL::GLogger, FormatString, ##__VA_ARGS__ )
    #define GLOG_FATAL( FormatString, ... ) SLOG_FATAL( SKL::GLogger, FormatString, ##__VA_ARGS__ )
    #define GTRACE_DEBUG( FormatString, ... ) STRACE_DEBUG( SKL::GLogger, FormatString, ##__VA_ARGS__ )
    #define GTRACE_INFO( FormatString, ... ) STRACE_INFO( SKL::GLogger, FormatString, ##__VA_ARGS__ )
    #define GTRACE_WARNING( FormatString, ... ) STRACE_WARNING( SKL::GLogger, FormatString, ##__VA_ARGS__ )
    #define GTRACE_ERROR( FormatString, ... ) STRACE_ERROR( SKL::GLogger, FormatString, ##__VA_ARGS__ )
    #define GTRACE_FATAL( FormatString, ... ) STRACE_FATAL( SKL::GLogger, FormatString, ##__VA_ARGS__ )
    #define GTRACE() STRACE_DEBUG( SKL::GLogger, "" )
#else
    #define GLOG_DEBUG( FormatString, ... ) 
    #define GLOG_INFO( FormatString, ... ) 
    #define GLOG_WARNING( FormatString, ... ) 
    #define GLOG_ERROR( FormatString, ... ) 
    #define GLOG_FATAL( FormatString, ... ) 
    #define GTRACE_DEBUG( FormatString, ... )
    #define GTRACE_INFO( FormatString, ... ) 
    #define GTRACE_WARNING( FormatString, ... ) 
    #define GTRACE_ERROR( FormatString, ... ) 
    #define GTRACE_FATAL( FormatString, ... ) 
    #define GTRACE() 
#endif
