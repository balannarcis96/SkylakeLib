//!
//! \file Port_Windows.h
//! 
//! \brief Windows platform abstraction layer for SkylakeLib
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    struct AsyncIOOpaqueType
    {
        AsyncIOOpaqueType() 
        {
            Reset();    
        }

        uint8_t Body[32]; // sizeof(OVERLAPPED);
        
        //! \brief Reset this instance for reuse 
        SKL_FORCEINLINE void Reset() noexcept
        {
            memset( Body, 0, sizeof( uint8_t ) * std::size( Body ) );
        }
    };

    struct Timer
    {   
        Timer() = default;
        ~Timer() = default;
        
        //! \brief Init the timer
        bool Init() noexcept;

        //! \brief Tick(update) the timer and get the total milliseconds elapsed since the first Tick()
        double Tick() noexcept;        

        //! \brief Get total milliseconds elapsed since the last Tick()
        SKL_FORCEINLINE double GetElapsed() const noexcept { return Elapsed; }

        //! \brief Get total milliseconds elapsed since the first Tick()
        SKL_FORCEINLINE double GetTotalTime() const noexcept { return TotalTime; }

    private:
        double  Elapsed          { 0.0 };
        double  TotalTime        { 0.0 };
        double  FrequencySeconds { 0.0 };
        int64_t I                { 0 };
        int64_t Start            { 0 };
    };
}

namespace std
{
    struct rw_lock
    {
        rw_lock() noexcept;
        ~rw_lock() noexcept = default;

        // std::shared_mutex compatible API
        void lock() noexcept;
        void unlock() noexcept;
        SKL_NODISCARD bool try_lock() noexcept;
        SKL_NODISCARD bool try_lock_shared() noexcept;
        void lock_shared() noexcept;
        void unlock_shared() noexcept;

    private:
        void* LockHandle;
    };
}

namespace SKL
{
    constexpr TOSError OS_ERROR_NET_TIMEOUT = 10060L;
}
