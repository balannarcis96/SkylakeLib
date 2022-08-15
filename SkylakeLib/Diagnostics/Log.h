//!
//! \file Log.h
//! 
//! \brief Simple file logging 
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

#if SKL_LOG_LEVEL == SKL_LOG_LEVEL_VERBOSE

    #define SKL_INF( InString ) fprintf( SKL::GLogOutput.load(), "[SkylakeLib::INF] " InString "\n" )
    #define SKL_WRN( InString ) fprintf( SKL::GLogOutput.load(), "[SkylakeLib::WRN] " InString "\n" )
    #define SKL_ERR( InString ) fprintf( SKL::GLogOutput.load(), "[SkylakeLib::ERR] " InString "\n" )
    #define SKL_VER( InString ) fprintf( SKL::GLogOutput.load(), "[SkylakeLib::INF] " InString "\n" )

    #define SKL_INF_FMT( InFormatString, ... ) fprintf( SKL::GLogOutput.load(), "[SkylakeLib::INF] " InFormatString "\n", __VA_ARGS__ )
    #define SKL_WRN_FMT( InFormatString, ... ) fprintf( SKL::GLogOutput.load(), "[SkylakeLib::WRN] " InFormatString "\n", __VA_ARGS__ )
    #define SKL_ERR_FMT( InFormatString, ... ) fprintf( SKL::GLogOutput.load(), "[SkylakeLib::ERR] " InFormatString "\n", __VA_ARGS__ )
    #define SKL_VER_FMT( InFormatString, ... ) fprintf( SKL::GLogOutput.load(), "[SkylakeLib::INF] " InFormatString "\n", __VA_ARGS__ )

#elif SKL_LOG_LEVEL == SKL_LOG_LEVEL_DEBUG || SKL_LOG_LEVEL == SKL_LOG_LEVEL_INF

    #define SKL_INF( InString ) fprintf( SKL::GLogOutput.load(), "[SkylakeLib::INF] " InString "\n" )
    #define SKL_WRN( InString ) fprintf( SKL::GLogOutput.load(), "[SkylakeLib::WRN] " InString "\n" )
    #define SKL_ERR( InString ) fprintf( SKL::GLogOutput.load(), "[SkylakeLib::ERR] " InString "\n" )
    #define SKL_VER( InString )

    #define SKL_INF_FMT( InFormatString, ... ) fprintf( SKL::GLogOutput.load(), "[SkylakeLib::INF] " InFormatString "\n", __VA_ARGS__ )
    #define SKL_WRN_FMT( InFormatString, ... ) fprintf( SKL::GLogOutput.load(), "[SkylakeLib::WRN] " InFormatString "\n", __VA_ARGS__ )
    #define SKL_ERR_FMT( InFormatString, ... ) fprintf( SKL::GLogOutput.load(), "[SkylakeLib::ERR] " InFormatString "\n", __VA_ARGS__ )
    #define SKL_VER_FMT( InFormatString, ... )

#elif SKL_LOG_LEVEL == SKL_LOG_LEVEL_ERR

    #define SKL_INF( InString ) 
    #define SKL_WRN( InString ) 
    #define SKL_ERR( InString ) fprintf( SKL::GLogOutput.load(), "[SkylakeLib::ERR] " InString "\n" )
    #define SKL_VER( InString )

    #define SKL_INF_FMT( InFormatString, ... ) 
    #define SKL_WRN_FMT( InFormatString, ... ) 
    #define SKL_ERR_FMT( InFormatString, ... ) fprintf( SKL::GLogOutput.load(), "[SkylakeLib::ERR] " InFormatString "\n", __VA_ARGS__ )
    #define SKL_VER_FMT( InFormatString, ... )

#elif SKL_LOG_LEVEL == SKL_LOG_LEVEL_WRN

    #define SKL_INF( InString ) 
    #define SKL_WRN( InString ) fprintf( SKL::GLogOutput.load(), "[SkylakeLib::WRN] " InString "\n" )
    #define SKL_ERR( InString ) fprintf( SKL::GLogOutput.load(), "[SkylakeLib::ERR] " InString "\n" )
    #define SKL_VER( InString )

    #define SKL_INF_FMT( InFormatString, ... ) 
    #define SKL_WRN_FMT( InFormatString, ... ) fprintf( SKL::GLogOutput.load(), "[SkylakeLib::WRN] " InFormatString "\n", __VA_ARGS__ )
    #define SKL_ERR_FMT( InFormatString, ... ) fprintf( SKL::GLogOutput.load(), "[SkylakeLib::ERR] " InFormatString "\n", __VA_ARGS__ )
    #define SKL_VER_FMT( InFormatString, ... )

#else

    #define SKL_INF( InString ) fprintf( SKL::GLogOutput.load(), "[SkylakeLib::INF] " InString "\n" )
    #define SKL_WRN( InString ) fprintf( SKL::GLogOutput.load(), "[SkylakeLib::WRN] " InString "\n" )
    #define SKL_ERR( InString ) fprintf( SKL::GLogOutput.load(), "[SkylakeLib::ERR] " InString "\n" )
    #define SKL_VER( InString )

    #define SKL_INF_FMT( InFormatString, ... ) fprintf( SKL::GLogOutput.load(), "[SkylakeLib::INF] " InFormatString "\n", __VA_ARGS__ )
    #define SKL_WRN_FMT( InFormatString, ... ) fprintf( SKL::GLogOutput.load(), "[SkylakeLib::WRN] " InFormatString "\n", __VA_ARGS__ )
    #define SKL_ERR_FMT( InFormatString, ... ) fprintf( SKL::GLogOutput.load(), "[SkylakeLib::ERR] " InFormatString "\n", __VA_ARGS__ )
    #define SKL_VER_FMT( InFormatString, ... )

#endif
