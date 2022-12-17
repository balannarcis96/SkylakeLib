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

#define SKLL_LOG_LEVEL_VERBOSE 0
#define SKLL_LOG_LEVEL_DEBUG 1
#define SKLL_LOG_LEVEL_ERR 2
#define SKLL_LOG_LEVEL_WRN 3
#define SKLL_LOG_LEVEL_INF 4
#define SKLL_LOG_LEVEL_MUTE 5

#define SKLL_LOG( InString ) fprintf( SKL::GLogOutput.load(),  InString "\n" )
#define SKLL_LOG_FMT( InFormatString, ... ) fprintf( SKL::GLogOutput.load(),  InFormatString "\n", __VA_ARGS__ )

#define SKLL_LOG_PLACE( InString ) fprintf( SKL::GLogOutput.load(), "%s():%u\n\t" InString "\n", __FUNCTION__, static_cast<uint32_t>( __LINE__ ) )
#define SKLL_LOG_PLACE_FMT( InFormatString, ... ) fprintf( SKL::GLogOutput.load(), "%s():%u\n\t" InFormatString "\n", __FUNCTION__, static_cast<uint32_t>( __LINE__ ), __VA_ARGS__ )

#if SKLL_LOG_LEVEL == SKLL_LOG_LEVEL_DEBUG 

    #define SKLL_INF( InString )       fprintf( SKL::GLogOutput.load(), "\u001b[37m[SkylakeLib::INF] " InString "\n\u001b[37m" )
    #define SKLL_TRACE_INF( InString ) fprintf( SKL::GLogOutput.load(), "\u001b[37m[SkylakeLib::INF] %s() Line: %u " InString "\n\u001b[37m", __FUNCTION__, static_cast<uint32_t>( __LINE__ ) )
    #define SKLL_WRN( InString )       fprintf( SKL::GLogOutput.load(), "\u001b[33m[SkylakeLib::WRN] " InString "\n\u001b[37m" )
    #define SKLL_TRACE_WRN( InString ) fprintf( SKL::GLogOutput.load(), "\u001b[33m[SkylakeLib::WRN] %s() Line: %u " InString "\n\u001b[37m", __FUNCTION__, static_cast<uint32_t>( __LINE__ ) )
    #define SKLL_ERR( InString )       fprintf( SKL::GLogOutput.load(), "\u001b[31m[SkylakeLib::ERR] " InString "\n\u001b[37m" )
    #define SKLL_TRACE_ERR( InString ) fprintf( SKL::GLogOutput.load(), "\u001b[31m[SkylakeLib::ERR] %s() Line: %u " InString "\n\u001b[37m", __FUNCTION__, static_cast<uint32_t>( __LINE__ ) )
    #define SKLL_VER( InString )       fprintf( SKL::GLogOutput.load(), "\u001b[37m[SkylakeLib::INF] " InString "\n\u001b[37m" )
    #define SKLL_TRACE()               fprintf( SKL::GLogOutput.load(), "\u001b[36m[SkylakeLib:: TR] >> %s() Line:%u \n\u001b[37m", __FUNCTION__, static_cast<uint32_t>( __LINE__ ) )
    #define SKLL_TRACE_MSG( InString ) fprintf( SKL::GLogOutput.load(), "\u001b[36m[SkylakeLib:: TR] >> %s() Line:%u [" InString "]\n", __FUNCTION__, static_cast<uint32_t>( __LINE__ ) )

    #define SKLL_INF_FMT( InFormatString, ... )       fprintf( SKL::GLogOutput.load(), "\u001b[37m[SkylakeLib::INF] " InFormatString "\n\u001b[37m", __VA_ARGS__ )
    #define SKLL_TRACE_INF_FMT( InFormatString, ... ) fprintf( SKL::GLogOutput.load(), "\u001b[37m[SkylakeLib::INF] %s() Line: %u " InFormatString "\n\u001b[37m", __FUNCTION__, static_cast<uint32_t>( __LINE__ ), __VA_ARGS__ )
    #define SKLL_WRN_FMT( InFormatString, ... )       fprintf( SKL::GLogOutput.load(), "\u001b[33m[SkylakeLib::WRN] " InFormatString "\n\u001b[37m", __VA_ARGS__ )
    #define SKLL_TRACE_WRN_FMT( InFormatString, ... ) fprintf( SKL::GLogOutput.load(), "\u001b[33m[SkylakeLib::WRN] %s() Line: %u " InFormatString "\n\u001b[37m", __FUNCTION__, static_cast<uint32_t>( __LINE__ ), __VA_ARGS__ )
    #define SKLL_ERR_FMT( InFormatString, ... )       fprintf( SKL::GLogOutput.load(), "\u001b[31m[SkylakeLib::ERR] " InFormatString "\n\u001b[37m", __VA_ARGS__ )
    #define SKLL_TRACE_ERR_FMT( InFormatString, ... ) fprintf( SKL::GLogOutput.load(), "\u001b[31m[SkylakeLib::ERR] %s() Line: %u " InFormatString "\n\u001b[37m", __FUNCTION__, static_cast<uint32_t>( __LINE__ ), __VA_ARGS__ )
    #define SKLL_VER_FMT( InFormatString, ... )       fprintf( SKL::GLogOutput.load(), "\u001b[37m[SkylakeLib::INF] " InFormatString "\n\u001b[37m", __VA_ARGS__ )
    #define SKLL_TRACE_MSG_FMT( InFmtStr, ... )       fprintf( SKL::GLogOutput.load(), "\u001b[36m[SkylakeLib:: TR] >> %s() Line:%u [" InFmtStr "]\n\u001b[37m", __FUNCTION__, static_cast<uint32_t>( __LINE__ ), __VA_ARGS__ )
    
    #define SKLL_INF_BLOCK( expr ) expr
    #define SKLL_WRN_BLOCK( expr ) expr
    #define SKLL_ERR_BLOCK( expr ) expr
    #define SKLL_VER_BLOCK( expr ) expr

#elif SKLL_LOG_LEVEL == SKLL_LOG_LEVEL_VERBOSE

    #define SKLL_INF( InString )       fprintf( SKL::GLogOutput.load(), "\u001b[37m[SkylakeLib::INF] " InString "\n\u001b[37m" )
    #define SKLL_TRACE_INF( InString ) fprintf( SKL::GLogOutput.load(), "\u001b[37m[SkylakeLib::INF] %s() Line: %u " InString "\n\u001b[37m", __FUNCTION__, static_cast<uint32_t>( __LINE__ ) )
    #define SKLL_WRN( InString )       fprintf( SKL::GLogOutput.load(), "\u001b[33m[SkylakeLib::WRN] " InString "\n\u001b[37m" )
    #define SKLL_TRACE_WRN( InString ) fprintf( SKL::GLogOutput.load(), "\u001b[33m[SkylakeLib::WRN] %s() Line: %u " InString "\n\u001b[37m", __FUNCTION__, static_cast<uint32_t>( __LINE__ ) )
    #define SKLL_ERR( InString )       fprintf( SKL::GLogOutput.load(), "\u001b[31m[SkylakeLib::ERR] " InString "\n\u001b[37m" )
    #define SKLL_TRACE_ERR( InString ) fprintf( SKL::GLogOutput.load(), "\u001b[31m[SkylakeLib::ERR] %s() Line: %u " InString "\n\u001b[37m", __FUNCTION__, static_cast<uint32_t>( __LINE__ ) )
    #define SKLL_VER( InString )       fprintf( SKL::GLogOutput.load(), "\u001b[37m[SkylakeLib::INF] " InString "\n\u001b[37m" )
    #define SKLL_TRACE() 
    #define SKLL_TRACE_MSG( InString )

    #define SKLL_INF_FMT( InFormatString, ... )       fprintf( SKL::GLogOutput.load(), "\u001b[37m[SkylakeLib::INF] " InFormatString "\n\u001b[37m", __VA_ARGS__ )
    #define SKLL_TRACE_INF_FMT( InFormatString, ... ) fprintf( SKL::GLogOutput.load(), "\u001b[37m[SkylakeLib::INF] %s() Line: %u " InFormatString "\n\u001b[37m", __FUNCTION__, static_cast<uint32_t>( __LINE__ ), __VA_ARGS__ )
    #define SKLL_WRN_FMT( InFormatString, ... )       fprintf( SKL::GLogOutput.load(), "\u001b[33m[SkylakeLib::WRN] " InFormatString "\n\u001b[37m", __VA_ARGS__ )
    #define SKLL_TRACE_WRN_FMT( InFormatString, ... ) fprintf( SKL::GLogOutput.load(), "\u001b[33m[SkylakeLib::WRN] %s() Line: %u " InFormatString "\n\u001b[37m", __FUNCTION__, static_cast<uint32_t>( __LINE__ ), __VA_ARGS__ )
    #define SKLL_ERR_FMT( InFormatString, ... )       fprintf( SKL::GLogOutput.load(), "\u001b[31m[SkylakeLib::ERR] " InFormatString "\n\u001b[37m", __VA_ARGS__ )
    #define SKLL_TRACE_ERR_FMT( InFormatString, ... ) fprintf( SKL::GLogOutput.load(), "\u001b[31m[SkylakeLib::ERR] %s() Line: %u " InFormatString "\n\u001b[37m", __FUNCTION__, static_cast<uint32_t>( __LINE__ ), __VA_ARGS__ )
    #define SKLL_VER_FMT( InFormatString, ... )       fprintf( SKL::GLogOutput.load(), "\u001b[37m[SkylakeLib::INF] " InFormatString "\n\u001b[37m", __VA_ARGS__ )
    #define SKLL_TRACE_MSG_FMT( InFmtStr, ... )
    
    #define SKLL_INF_BLOCK( expr ) expr
    #define SKLL_WRN_BLOCK( expr ) expr
    #define SKLL_ERR_BLOCK( expr ) expr
    #define SKLL_VER_BLOCK( expr ) expr

#elif SKLL_LOG_LEVEL == SKLL_LOG_LEVEL_INF

    #define SKLL_INF( InString )       fprintf( SKL::GLogOutput.load(), "\u001b[37m[SkylakeLib::INF] " InString "\n\u001b[37m" )
    #define SKLL_TRACE_INF( InString ) fprintf( SKL::GLogOutput.load(), "\u001b[37m[SkylakeLib::INF] %s() Line: %u " InString "\n\u001b[37m", __FUNCTION__, static_cast<uint32_t>( __LINE__ ) )
    #define SKLL_WRN( InString )       fprintf( SKL::GLogOutput.load(), "\u001b[33m[SkylakeLib::WRN] " InString "\n\u001b[37m" )
    #define SKLL_TRACE_WRN( InString ) fprintf( SKL::GLogOutput.load(), "\u001b[33m[SkylakeLib::WRN] %s() Line: %u " InString "\n\u001b[37m", __FUNCTION__, static_cast<uint32_t>( __LINE__ ) )
    #define SKLL_ERR( InString )       fprintf( SKL::GLogOutput.load(), "\u001b[31m[SkylakeLib::ERR] " InString "\n\u001b[37m" )
    #define SKLL_TRACE_ERR( InString ) fprintf( SKL::GLogOutput.load(), "\u001b[31m[SkylakeLib::ERR] %s() Line: %u " InString "\n\u001b[37m", __FUNCTION__, static_cast<uint32_t>( __LINE__ ) )
    #define SKLL_VER( InString )
    #define SKLL_TRACE() 
    #define SKLL_TRACE_MSG( InString )

    #define SKLL_INF_FMT( InFormatString, ... )       fprintf( SKL::GLogOutput.load(), "\u001b[37m[SkylakeLib::INF] " InFormatString "\n\u001b[37m", __VA_ARGS__ )
    #define SKLL_TRACE_INF_FMT( InFormatString, ... ) fprintf( SKL::GLogOutput.load(), "\u001b[37m[SkylakeLib::INF] %s() Line: %u " InFormatString "\n\u001b[37m", __FUNCTION__, static_cast<uint32_t>( __LINE__ ), __VA_ARGS__ )
    #define SKLL_WRN_FMT( InFormatString, ... )       fprintf( SKL::GLogOutput.load(), "\u001b[33m[SkylakeLib::WRN] " InFormatString "\n\u001b[37m", __VA_ARGS__ )
    #define SKLL_TRACE_WRN_FMT( InFormatString, ... ) fprintf( SKL::GLogOutput.load(), "\u001b[33m[SkylakeLib::WRN] %s() Line: %u " InFormatString "\n\u001b[37m", __FUNCTION__, static_cast<uint32_t>( __LINE__ ), __VA_ARGS__ )
    #define SKLL_ERR_FMT( InFormatString, ... )       fprintf( SKL::GLogOutput.load(), "\u001b[31m[SkylakeLib::ERR] " InFormatString "\n\u001b[37m", __VA_ARGS__ )
    #define SKLL_TRACE_ERR_FMT( InFormatString, ... ) fprintf( SKL::GLogOutput.load(), "\u001b[31m[SkylakeLib::ERR] %s() Line: %u " InFormatString "\n\u001b[37m", __FUNCTION__, static_cast<uint32_t>( __LINE__ ), __VA_ARGS__ )
    #define SKLL_VER_FMT( InFormatString, ... )
    #define SKLL_TRACE_MSG_FMT( InFmtStr, ... )
    
    #define SKLL_INF_BLOCK( expr ) expr
    #define SKLL_WRN_BLOCK( expr ) expr
    #define SKLL_ERR_BLOCK( expr ) expr
    #define SKLL_VER_BLOCK( expr )

#elif SKLL_LOG_LEVEL == SKLL_LOG_LEVEL_ERR

    #define SKLL_INF( InString )
    #define SKLL_TRACE_INF( InString )
    #define SKLL_WRN( InString )
    #define SKLL_TRACE_WRN( InString )
    #define SKLL_ERR( InString )       fprintf( SKL::GLogOutput.load(), "\u001b[31m[SkylakeLib::ERR] " InString "\n\u001b[37m" )
    #define SKLL_TRACE_ERR( InString ) fprintf( SKL::GLogOutput.load(), "\u001b[31m[SkylakeLib::ERR] %s() Line: %u " InString "\n\u001b[37m", __FUNCTION__, static_cast<uint32_t>( __LINE__ ) )
    #define SKLL_VER( InString )
    #define SKLL_TRACE() 
    #define SKLL_TRACE_MSG( InString )

    #define SKLL_INF_FMT( InFormatString, ... )
    #define SKLL_TRACE_INF_FMT( InFormatString, ... )
    #define SKLL_WRN_FMT( InFormatString, ... )
    #define SKLL_TRACE_WRN_FMT( InFormatString, ... )
    #define SKLL_ERR_FMT( InFormatString, ... )       fprintf( SKL::GLogOutput.load(), "\u001b[31m[SkylakeLib::ERR] " InFormatString "\n\u001b[37m", __VA_ARGS__ )
    #define SKLL_TRACE_ERR_FMT( InFormatString, ... ) fprintf( SKL::GLogOutput.load(), "\u001b[31m[SkylakeLib::ERR] %s() Line: %u " InFormatString "\n\u001b[37m", __FUNCTION__, static_cast<uint32_t>( __LINE__ ), __VA_ARGS__ )
    #define SKLL_VER_FMT( InFormatString, ... )
    #define SKLL_TRACE_MSG_FMT( InFmtStr, ... )
    
    #define SKLL_INF_BLOCK( expr )
    #define SKLL_WRN_BLOCK( expr ) expr
    #define SKLL_ERR_BLOCK( expr ) expr
    #define SKLL_VER_BLOCK( expr )

#elif SKLL_LOG_LEVEL == SKLL_LOG_LEVEL_WRN

    #define SKLL_INF( InString )
    #define SKLL_TRACE_INF( InString )
    #define SKLL_WRN( InString )       fprintf( SKL::GLogOutput.load(), "\u001b[33m[SkylakeLib::WRN] " InString "\n\u001b[37m" )
    #define SKLL_TRACE_WRN( InString ) fprintf( SKL::GLogOutput.load(), "\u001b[33m[SkylakeLib::WRN] %s() Line: %u " InString "\n\u001b[37m", __FUNCTION__, static_cast<uint32_t>( __LINE__ ) )
    #define SKLL_ERR( InString )       fprintf( SKL::GLogOutput.load(), "\u001b[31m[SkylakeLib::ERR] " InString "\n\u001b[37m" )
    #define SKLL_TRACE_ERR( InString ) fprintf( SKL::GLogOutput.load(), "\u001b[31m[SkylakeLib::ERR] %s() Line: %u " InString "\n\u001b[37m", __FUNCTION__, static_cast<uint32_t>( __LINE__ ) )
    #define SKLL_VER( InString )
    #define SKLL_TRACE() 
    #define SKLL_TRACE_MSG( InString )

    #define SKLL_INF_FMT( InFormatString, ... )
    #define SKLL_TRACE_INF_FMT( InFormatString, ... )
    #define SKLL_WRN_FMT( InFormatString, ... )       fprintf( SKL::GLogOutput.load(), "\u001b[33m[SkylakeLib::WRN] " InFormatString "\n\u001b[37m", __VA_ARGS__ )
    #define SKLL_TRACE_WRN_FMT( InFormatString, ... ) fprintf( SKL::GLogOutput.load(), "\u001b[33m[SkylakeLib::WRN] %s() Line: %u " InFormatString "\n\u001b[37m", __FUNCTION__, static_cast<uint32_t>( __LINE__ ), __VA_ARGS__ )
    #define SKLL_ERR_FMT( InFormatString, ... )       fprintf( SKL::GLogOutput.load(), "\u001b[31m[SkylakeLib::ERR] " InFormatString "\n\u001b[37m", __VA_ARGS__ )
    #define SKLL_TRACE_ERR_FMT( InFormatString, ... ) fprintf( SKL::GLogOutput.load(), "\u001b[31m[SkylakeLib::ERR] %s() Line: %u " InFormatString "\n\u001b[37m", __FUNCTION__, static_cast<uint32_t>( __LINE__ ), __VA_ARGS__ )
    #define SKLL_VER_FMT( InFormatString, ... )
    #define SKLL_TRACE_MSG_FMT( InFmtStr, ... )

    #define SKLL_INF_BLOCK( expr )
    #define SKLL_WRN_BLOCK( expr ) expr
    #define SKLL_ERR_BLOCK( expr ) expr
    #define SKLL_VER_BLOCK( expr )

#elif SKLL_LOG_LEVEL == SKLL_LOG_LEVEL_MUTE

    #define SKLL_INF( InString ) 
    #define SKLL_TRACE_INF( InString )
    #define SKLL_WRN( InString ) 
    #define SKLL_TRACE_WRN( InString )
    #define SKLL_ERR( InString ) 
    #define SKLL_TRACE_ERR( InString )
    #define SKLL_VER( InString )
    #define SKLL_TRACE() 
    #define SKLL_TRACE_MSG( InString )

    #define SKLL_INF_FMT( InFormatString, ... ) 
    #define SKLL_TRACE_INF_FMT( InFormatString, ... )
    #define SKLL_WRN_FMT( InFormatString, ... ) 
    #define SKLL_TRACE_WRN_FMT( InFormatString, ... )
    #define SKLL_ERR_FMT( InFormatString, ... ) 
    #define SKLL_TRACE_ERR_FMT( InFormatString, ... )
    #define SKLL_VER_FMT( InFormatString, ... )
    #define SKLL_TRACE_MSG_FMT( InFmtStr, ... )
    
    #define SKLL_INF_BLOCK( expr )
    #define SKLL_WRN_BLOCK( expr )
    #define SKLL_ERR_BLOCK( expr )
    #define SKLL_VER_BLOCK( expr )

#else

    #define SKLL_INF( InString )       fprintf( SKL::GLogOutput.load(), "\u001b[37m[SkylakeLib::INF] " InString "\n\u001b[37m" )
    #define SKLL_TRACE_INF( InString ) fprintf( SKL::GLogOutput.load(), "\u001b[37m[SkylakeLib::INF] %s() Line: %u " InString "\n\u001b[37m", __FUNCTION__, static_cast<uint32_t>( __LINE__ ) )
    #define SKLL_WRN( InString )       fprintf( SKL::GLogOutput.load(), "\u001b[33m[SkylakeLib::WRN] " InString "\n\u001b[37m" )
    #define SKLL_TRACE_WRN( InString ) fprintf( SKL::GLogOutput.load(), "\u001b[33m[SkylakeLib::WRN] %s() Line: %u " InString "\n\u001b[37m", __FUNCTION__, static_cast<uint32_t>( __LINE__ ) )
    #define SKLL_ERR( InString )       fprintf( SKL::GLogOutput.load(), "\u001b[31m[SkylakeLib::ERR] " InString "\n\u001b[37m" )
    #define SKLL_TRACE_ERR( InString ) fprintf( SKL::GLogOutput.load(), "\u001b[31m[SkylakeLib::ERR] %s() Line: %u " InString "\n\u001b[37m", __FUNCTION__, static_cast<uint32_t>( __LINE__ ) )
    #define SKLL_VER( InString )
    #define SKLL_TRACE() 
    #define SKLL_TRACE_MSG( InString )

    #define SKLL_INF_FMT( InFormatString, ... )       fprintf( SKL::GLogOutput.load(), "\u001b[37m[SkylakeLib::INF] " InFormatString "\n\u001b[37m", __VA_ARGS__ )
    #define SKLL_TRACE_INF_FMT( InFormatString, ... ) fprintf( SKL::GLogOutput.load(), "\u001b[37m[SkylakeLib::INF] %s() Line: %u " InFormatString "\n\u001b[37m", __FUNCTION__, static_cast<uint32_t>( __LINE__ ), __VA_ARGS__ )
    #define SKLL_WRN_FMT( InFormatString, ... )       fprintf( SKL::GLogOutput.load(), "\u001b[33m[SkylakeLib::WRN] " InFormatString "\n\u001b[37m", __VA_ARGS__ )
    #define SKLL_TRACE_WRN_FMT( InFormatString, ... ) fprintf( SKL::GLogOutput.load(), "\u001b[33m[SkylakeLib::WRN] %s() Line: %u " InFormatString "\n\u001b[37m", __FUNCTION__, static_cast<uint32_t>( __LINE__ ), __VA_ARGS__ )
    #define SKLL_ERR_FMT( InFormatString, ... )       fprintf( SKL::GLogOutput.load(), "\u001b[31m[SkylakeLib::ERR] " InFormatString "\n\u001b[37m", __VA_ARGS__ )
    #define SKLL_TRACE_ERR_FMT( InFormatString, ... ) fprintf( SKL::GLogOutput.load(), "\u001b[31m[SkylakeLib::ERR] %s() Line: %u " InFormatString "\n\u001b[37m", __FUNCTION__, static_cast<uint32_t>( __LINE__ ), __VA_ARGS__ )
    #define SKLL_VER_FMT( InFormatString, ... )
    #define SKLL_TRACE_MSG_FMT( InFmtStr, ... )
    
    #define SKLL_INF_BLOCK( expr ) expr
    #define SKLL_WRN_BLOCK( expr ) expr
    #define SKLL_ERR_BLOCK( expr ) expr
    #define SKLL_VER_BLOCK( expr ) expr

#endif
