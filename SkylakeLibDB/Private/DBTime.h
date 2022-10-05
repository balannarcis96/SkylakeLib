//!
//! \file DBTIme.h
//! 
//! \brief MySQL c++ interface library
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL::DB
{
    enum class ETimestampType : int32_t 
    {
        TIMESTAMP_NONE = -2,
        TIMESTAMP_ERROR = -1,

        /// Stores year, month and day components.
        TIMESTAMP_DATE = 0,

        /**
          Stores all date and time components.
          Value is in UTC for `TIMESTAMP` type.
          Value is in local time zone for `DATETIME` type.
        */
        TIMESTAMP_DATETIME = 1,

        /// Stores hour, minute, second and microsecond.
        TIMESTAMP_TIME = 2,

        /**
          A temporary type for `DATETIME` or `TIMESTAMP` types equipped with time
          zone information. After the time zone information is reconciled, the type is
          converted to MYSQL_TIMESTAMP_DATETIME.
        */
        TIMESTAMP_DATETIME_TZ = 3
    };

    /*
      Structure which is used to represent datetime values inside MySQL.

      We assume that values in this structure are normalized, i.e. year <= 9999,
      month <= 12, day <= 31, hour <= 23, hour <= 59, hour <= 59. Many functions
      in server such as my_system_gmt_sec() or make_time() family of functions
      rely on this (actually now usage of make_*() family relies on a bit weaker
      restriction). Also functions that produce MYSQL_TIME as result ensure this.
      There is one exception to this rule though if this structure holds time
      value (time_type == MYSQL_TIMESTAMP_TIME) days and hour member can hold
      bigger values.
    */
    struct DBTimeBase
    {
        uint32_t       Year                { 0 };
        uint32_t       Month               { 0 };   
        uint32_t       Day                 { 0 };
        uint32_t       Hour                { 0 };
        uint32_t       Minute              { 0 };
        uint32_t       Second              { 0 };
        uint32_t       SecondPart          { 0 } ; //!< microseconds
        bool           bNegative           { false };
        ETimestampType Type                { ETimestampType::TIMESTAMP_NONE };
        int32_t        TimeZoneDisplacement{ 0 };
    };

    template<EFieldType InType>
    struct TDBTime : DBTimeBase
    {
        static_assert( InType == EFieldType::TYPE_DATE 
                    || InType == EFieldType::TYPE_TIME 
                    || InType == EFieldType::TYPE_TIME2 
                    || InType == EFieldType::TYPE_DATETIME 
                    || InType == EFieldType::TYPE_DATETIME2 
                    || InType == EFieldType::TYPE_TIMESTAMP 
                    || InType == EFieldType::TYPE_TIMESTAMP2 );
        static constexpr EFieldType Type = InType;
    };

    using DBDate       = TDBTime<EFieldType::TYPE_DATE>;
    using DBTime       = TDBTime<EFieldType::TYPE_TIME>;
    using DBTime2      = TDBTime<EFieldType::TYPE_TIME2>;
    using DBDateTime   = TDBTime<EFieldType::TYPE_DATETIME>;
    using DBDateTime2  = TDBTime<EFieldType::TYPE_DATETIME2>;
    using DBTimeStamp  = TDBTime<EFieldType::TYPE_TIMESTAMP>;
    using DBTimeStamp2 = TDBTime<EFieldType::TYPE_TIMESTAMP2>;
}