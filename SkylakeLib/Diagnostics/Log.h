//!
//! \file Log.h
//! 
//! \brief Simple logging 
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    extern std::relaxed_value<FILE*> GLogOutput;
}

#define SKL_LOG_LEVEL_VERBOSE 0
#define SKL_LOG_LEVEL_DEBUG 1
#define SKL_LOG_LEVEL_ERR 2
#define SKL_LOG_LEVEL_WRN 3
#define SKL_LOG_LEVEL_INF 4

#if SKL_LOG_LEVEL == SKL_LOG_LEVEL_DEBUG 

    #define SKL_INF( InString ) fprintf( SKL::GLogOutput.load(), "\u001b[35m[SkylakeLib::INF] " InString "\n\u001b[37m" )
    #define SKL_WRN( InString ) fprintf( SKL::GLogOutput.load(), "\u001b[33m[SkylakeLib::WRN] " InString "\n\u001b[37m" )
    #define SKL_ERR( InString ) fprintf( SKL::GLogOutput.load(), "\u001b[31m[SkylakeLib::ERR] " InString "\n\u001b[37m" )
    #define SKL_VER( InString ) fprintf( SKL::GLogOutput.load(), "\u001b[37m[SkylakeLib::INF] " InString "\n\u001b[37m" )
    #define SKL_TRACE() fprintf( SKL::GLogOutput.load(), "\u001b[36m[SkylakeLib:: TR] >> %s() Line:%u \n\u001b[37m", __FUNCTION__, static_cast<uint32_t>( __LINE__ ) )
    #define SKL_TRACE_MSG( InString ) fprintf( SKL::GLogOutput.load(), "\u001b[36m[SkylakeLib:: TR] >> %s() Line:%u [" InString "]\n", __FUNCTION__, static_cast<uint32_t>( __LINE__ ) )

    #define SKL_INF_FMT( InFormatString, ... ) fprintf( SKL::GLogOutput.load(), "\u001b[35m[SkylakeLib::INF] " InFormatString "\n\u001b[37m", __VA_ARGS__ )
    #define SKL_WRN_FMT( InFormatString, ... ) fprintf( SKL::GLogOutput.load(), "\u001b[33m[SkylakeLib::WRN] " InFormatString "\n\u001b[37m", __VA_ARGS__ )
    #define SKL_ERR_FMT( InFormatString, ... ) fprintf( SKL::GLogOutput.load(), "\u001b[31m[SkylakeLib::ERR] " InFormatString "\n\u001b[37m", __VA_ARGS__ )
    #define SKL_VER_FMT( InFormatString, ... ) fprintf( SKL::GLogOutput.load(), "\u001b[37m[SkylakeLib::INF] " InFormatString "\n\u001b[37m", __VA_ARGS__ )
    #define SKL_TRACE_MSG_FMT( InFmtStr, ... ) fprintf( SKL::GLogOutput.load(), "\u001b[36m[SkylakeLib:: TR] >> %s() Line:%u [" InFmtStr "]\n\u001b[37m", __FUNCTION__, static_cast<uint32_t>( __LINE__ ), __VA_ARGS__ )

#elif SKL_LOG_LEVEL == SKL_LOG_LEVEL_VERBOSE

    #define SKL_INF( InString ) fprintf( SKL::GLogOutput.load(), "\u001b[35m[SkylakeLib::INF] " InString "\n\u001b[37m" )
    #define SKL_WRN( InString ) fprintf( SKL::GLogOutput.load(), "\u001b[33m[SkylakeLib::WRN] " InString "\n\u001b[37m" )
    #define SKL_ERR( InString ) fprintf( SKL::GLogOutput.load(), "\u001b[31m[SkylakeLib::ERR] " InString "\n\u001b[37m" )
    #define SKL_VER( InString ) fprintf( SKL::GLogOutput.load(), "\u001b[37m[SkylakeLib::INF] " InString "\n\u001b[37m" )
    #define SKL_TRACE() 
    #define SKL_TRACE_MSG( InString )

    #define SKL_INF_FMT( InFormatString, ... ) fprintf( SKL::GLogOutput.load(), "\u001b[35m[SkylakeLib::INF] " InFormatString "\n\u001b[37m", __VA_ARGS__ )
    #define SKL_WRN_FMT( InFormatString, ... ) fprintf( SKL::GLogOutput.load(), "\u001b[33m[SkylakeLib::WRN] " InFormatString "\n\u001b[37m", __VA_ARGS__ )
    #define SKL_ERR_FMT( InFormatString, ... ) fprintf( SKL::GLogOutput.load(), "\u001b[31m[SkylakeLib::ERR] " InFormatString "\n\u001b[37m", __VA_ARGS__ )
    #define SKL_VER_FMT( InFormatString, ... ) fprintf( SKL::GLogOutput.load(), "\u001b[37m[SkylakeLib::INF] " InFormatString "\n\u001b[37m", __VA_ARGS__ )
    #define SKL_TRACE_MSG_FMT( InFmtStr, ... )

#elif SKL_LOG_LEVEL == SKL_LOG_LEVEL_INF

    #define SKL_INF( InString ) fprintf( SKL::GLogOutput.load(), "\u001b[35m[SkylakeLib::INF] " InString "\n\u001b[37m" )
    #define SKL_WRN( InString ) fprintf( SKL::GLogOutput.load(), "\u001b[33m[SkylakeLib::WRN] " InString "\n\u001b[37m" )
    #define SKL_ERR( InString ) fprintf( SKL::GLogOutput.load(), "\u001b[31m[SkylakeLib::ERR] " InString "\n\u001b[37m" )
    #define SKL_VER( InString )
    #define SKL_TRACE() 
    #define SKL_TRACE_MSG( InString )

    #define SKL_INF_FMT( InFormatString, ... ) fprintf( SKL::GLogOutput.load(), "\u001b[35m[SkylakeLib::INF] " InFormatString "\n\u001b[37m", __VA_ARGS__ )
    #define SKL_WRN_FMT( InFormatString, ... ) fprintf( SKL::GLogOutput.load(), "\u001b[33m[SkylakeLib::WRN] " InFormatString "\n\u001b[37m", __VA_ARGS__ )
    #define SKL_ERR_FMT( InFormatString, ... ) fprintf( SKL::GLogOutput.load(), "\u001b[31m[SkylakeLib::ERR] " InFormatString "\n\u001b[37m", __VA_ARGS__ )
    #define SKL_VER_FMT( InFormatString, ... )
    #define SKL_TRACE_MSG_FMT( InFmtStr, ... )

#elif SKL_LOG_LEVEL == SKL_LOG_LEVEL_ERR

    #define SKL_INF( InString ) 
    #define SKL_WRN( InString ) 
    #define SKL_ERR( InString ) fprintf( SKL::GLogOutput.load(), "\u001b[31m[SkylakeLib::ERR] " InString "\n\u001b[37m" )
    #define SKL_VER( InString )
    #define SKL_TRACE() 
    #define SKL_TRACE_MSG( InString )

    #define SKL_INF_FMT( InFormatString, ... ) 
    #define SKL_WRN_FMT( InFormatString, ... ) 
    #define SKL_ERR_FMT( InFormatString, ... ) fprintf( SKL::GLogOutput.load(), "\u001b[31m[SkylakeLib::ERR] " InFormatString "\n\u001b[37m", __VA_ARGS__ )
    #define SKL_VER_FMT( InFormatString, ... )
    #define SKL_TRACE_MSG_FMT( InFmtStr, ... )

#elif SKL_LOG_LEVEL == SKL_LOG_LEVEL_WRN

    #define SKL_INF( InString ) 
    #define SKL_WRN( InString ) fprintf( SKL::GLogOutput.load(), "\u001b[33m[SkylakeLib::WRN] " InString "\n\u001b[37m" )
    #define SKL_ERR( InString ) fprintf( SKL::GLogOutput.load(), "\u001b[31m[SkylakeLib::ERR] " InString "\n\u001b[37m" )
    #define SKL_VER( InString )
    #define SKL_TRACE() 
    #define SKL_TRACE_MSG( InString )

    #define SKL_INF_FMT( InFormatString, ... ) 
    #define SKL_WRN_FMT( InFormatString, ... ) fprintf( SKL::GLogOutput.load(), "\u001b[33m[SkylakeLib::WRN] " InFormatString "\n\u001b[37m", __VA_ARGS__ )
    #define SKL_ERR_FMT( InFormatString, ... ) fprintf( SKL::GLogOutput.load(), "\u001b[31m[SkylakeLib::ERR] " InFormatString "\n\u001b[37m", __VA_ARGS__ )
    #define SKL_VER_FMT( InFormatString, ... )
    #define SKL_TRACE_MSG_FMT( InFmtStr, ... )

#else

    #define SKL_INF( InString ) fprintf( SKL::GLogOutput.load(), "\u001b[35m[SkylakeLib::INF] " InString "\n\u001b[37m" )
    #define SKL_WRN( InString ) fprintf( SKL::GLogOutput.load(), "\u001b[33m[SkylakeLib::WRN] " InString "\n\u001b[37m" )
    #define SKL_ERR( InString ) fprintf( SKL::GLogOutput.load(), "\u001b[31m[SkylakeLib::ERR] " InString "\n\u001b[37m" )
    #define SKL_VER( InString )
    #define SKL_TRACE() 
    #define SKL_TRACE_MSG( InString )

    #define SKL_INF_FMT( InFormatString, ... ) fprintf( SKL::GLogOutput.load(), "\u001b[35m[SkylakeLib::INF] " InFormatString "\n\u001b[37m", __VA_ARGS__ )
    #define SKL_WRN_FMT( InFormatString, ... ) fprintf( SKL::GLogOutput.load(), "\u001b[33m[SkylakeLib::WRN] " InFormatString "\n\u001b[37m", __VA_ARGS__ )
    #define SKL_ERR_FMT( InFormatString, ... ) fprintf( SKL::GLogOutput.load(), "\u001b[31m[SkylakeLib::ERR] " InFormatString "\n\u001b[37m", __VA_ARGS__ )
    #define SKL_VER_FMT( InFormatString, ... )
    #define SKL_TRACE_MSG_FMT( InFmtStr, ... )

#endif
