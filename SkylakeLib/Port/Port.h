//!
//! \file Port.h
//! 
//! \brief Platform abstraction layer for SkylakeLib
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace std
{
    //! Reader/Writer lock
    struct rw_lock;
}

namespace SKL
{
    //! Platform agnostic socket type 
    using TSocket = uint64_t;

    //! Platform specific timer API
    struct Timer;

    //! Platform specific, opaque type, for the async IO API
    struct AsyncIOOpaqueType;

    //! Platform specific frequency based timer
    struct Timer;
    
    //! Platform specific buffer type for async IO requests
    struct IBuffer;
    
    //! Type used as key to identify async IO requests
    using TCompletionKey = void *;
    
    //! Type that can hold a "handle" on any platform
    using THandle = uint64_t;

    //! Type for the TLS slot
    using TLSSlot = uint32_t;

    //! Type for ipv4 address
    using TIPv4Address = uint32_t;

    //! Type for network port
    using TNetPort = uint16_t;

    //! Type for os error value
    using TOSError = int32_t;

    //! Platform specific async IO API
    struct AsyncIO;
}

namespace SKL
{
    constexpr TSocket CInvalidSocket = (TSocket)(~0);

    //! Allocate new ipv4 tcp socket ( returns 0 on failure )
    SKL_NODISCARD TSocket AllocateNewIPv4TCPSocket( bool bAsync = true ) noexcept;

    //! Allocate new ipv4 udp socket ( returns 0 on failure )
    SKL_NODISCARD TSocket AllocateNewIPv4UDPSocket( bool bAsync = true ) noexcept;
    
    //! Perform a TCP connect on socket to address and port
    SKL_NODISCARD bool TCPConnectIPv4( TSocket InSocket, TIPv4Address InAddress, TNetPort InPort ) noexcept;
}

#include "TCPAsyncAccepter.h"

namespace SKL
{
    //! \brief Enable ANSI color support in the main console window
    //! \return RSuccess on success 
    SKL_NODISCARD RStatus EnableConsoleANSIColorSupport() noexcept;

    //! Get the number of milliseconds that have elapsed since the system was started
    SKL_NODISCARD TEpochTimePoint GetSystemUpTickCount() noexcept;

    //! Set the timer resolution of the OS
    SKL_NODISCARD RStatus SetOsTimeResolution( uint32_t InMilliseconds ) noexcept;

    //! Get the system l1 cache line size
    SKL_NODISCARD size_t GetL1CacheLineSize() noexcept;

    struct PlatformTLS final
    {
        static constexpr TLSSlot INVALID_SLOT_ID = 0xFFFFFFFF;

        /**
         * \brief Return false if InSlotIndex is an invalid TLS slot
         * \param InSlotIndex the TLS index to check
         * \return true if InSlotIndex looks like a valid slot
         */
        SKL_FORCEINLINE SKL_NODISCARD static bool IsValidTlsSlot( TLSSlot InSlotIndex ) noexcept
        {
            return InSlotIndex != INVALID_SLOT_ID;
        }
        
        //! \brief Get the calling thread id
        SKL_NODISCARD static uint32_t GetCurrentThreadId() noexcept;

        //! \brief Allocate new thread local storage slot for all threads of the process
        SKL_NODISCARD static TLSSlot AllocTlsSlot() noexcept;

        //! \brief Set the TLS value at InSlot for the calling thread
        static void SetTlsValue( TLSSlot InSlot, void* InValue ) noexcept;

        //! \brief Get the TLS value at InSlot for the calling thread
        SKL_NODISCARD static void* GetTlsValue( TLSSlot InSlot ) noexcept;

        //! \brief Free a previously allocated TLS slot
        SKL_NODISCARD static void FreeTlsSlot( TLSSlot InSlot ) noexcept;
    };

    //! Very precise sleep
    void PreciseSleep( double InSeconds ) noexcept;

    //! Is socket valid
    SKL_NODISCARD bool IsValidSocket( TSocket InSocket ) noexcept;

    //! Close socket
    bool CloseSocket( TSocket InSocket ) noexcept;
    
    //Shutdown socket
    bool ShutdownSocket( TSocket InSocket ) noexcept;
    
    //! Get last OS error code
    SKL_NODISCARD int32_t GGetLastError( ) noexcept;
    
    //! Get last OS network operation related error code
    SKL_NODISCARD int32_t GGetNetworkLastError( ) noexcept;

    //! Convert ip v4 address string to binary
    SKL_NODISCARD uint32_t IPv4FromStringA( const char* InIpString )noexcept;

    //! Convert ip v4 address wide string to binary
    SKL_NODISCARD uint32_t IPv4FromStringW( const wchar_t* InIpString )noexcept;

    //! Get the cwd
    SKL_NODISCARD bool GetCurrentWorkingDirectory( char* OutBuffer, size_t BufferSize ) noexcept;
    
    //! Get last OS error code
    void SetConsoleWindowTitleText( const char* InText ) noexcept;
    
    //! UTF16 -> UTF8
    SKL_NODISCARD bool GWideCharToMultiByte( const wchar_t * InBuffer, size_t InBufferSize, char* OutBuffer, int32_t InOutBufferSize ) noexcept;

    //! UTF8 -> UTF16
    SKL_NODISCARD bool GMultiByteToWideChar( const char * InBuffer, size_t InBufferSize, wchar_t* OutBuffer, int32_t InOutBufferSize ) noexcept;

    //! UTF16 -> UTF8
    template<size_t N>
    SKL_NODISCARD inline bool GWideCharToMultiByte( const wchar_t( &InBuffer ) [ N ], char* OutBuffer, int32_t OutBufferSize ) noexcept
    {
        return GWideCharToMultiByte( InBuffer, N, OutBuffer, OutBufferSize );
    }

    //! UTF8 -> UTF16
    template<size_t N>
    SKL_NODISCARD inline bool GMultiByteToWideChar( const char( &InBuffer ) [ N ], wchar_t* OutBuffer, int32_t OutBufferSize ) noexcept
    {
        return GMultiByteToWideChar( InBuffer, N, OutBuffer, OutBufferSize );
    }

    //! UTF16 -> UTF8
    template<size_t N, size_t M>
    SKL_NODISCARD inline bool GWideCharToMultiByte( const wchar_t( &InBuffer ) [ N ], char( &OutBuffer ) [ M ] ) noexcept
    {
        return GWideCharToMultiByte( InBuffer, N, OutBuffer, M );
    }

    //! UTF8 -> UTF16
    template<size_t N, size_t M>
    SKL_NODISCARD inline bool GMultiByteToWideChar( const char( &InBuffer ) [ N ], wchar_t( &OutBuffer ) [ M ] ) noexcept
    {
        return GMultiByteToWideChar( InBuffer, N, OutBuffer, M );
    }

    //! Get the current thread id as u32 
    SKL_FORCEINLINE inline uint32_t GetCurrentThreadId() noexcept
    {
        const auto ThreadId { std::this_thread::get_id() };
        static_assert( sizeof( ThreadId ) >= sizeof( uint32_t ) );
        return *reinterpret_cast<const uint32_t*>( &ThreadId );
    }
    
    //! Scan directory recursively and collect file names
    SKL_NODISCARD std::vector<std::string> ScanForFilesInDirectory( const char* RootDirectory, size_t& OutMaxFileSize, const std::vector<std::string>& extensions ) noexcept;

    //! Scan directory recursively and collect file names
    SKL_NODISCARD std::vector<std::wstring> ScanForFilesInDirectoryW( const wchar_t* RootDirectory, size_t& OutMaxFileSize, const std::vector<std::wstring>& extensions ) noexcept;

    //! Issue __rdtsc() intrinsic
    SKL_NODISCARD uint64_t GetTimeStampCounter() noexcept;
    
    SKL_NODISCARD int64_t GetPerformanceCounter() noexcept;

    SKL_NODISCARD int64_t GetPerformanceFrequency() noexcept;
    
    SKL_NODISCARD void LoadPerformanceCounter( int64_t& Out ) noexcept;

    SKL_NODISCARD void LoadPerformanceFrequency( int64_t& Out ) noexcept;
}

#include "TLSValue.h"
#include "TLSSingleton.h"

namespace SKL
{
    struct PreciseSleep_WaitableTimer final: public ITLSSingleton<PreciseSleep_WaitableTimer>
    {
        PreciseSleep_WaitableTimer() noexcept = default;
        ~PreciseSleep_WaitableTimer() noexcept = default;

        void*   Timer   { nullptr };
        double  Estimate{ 5e-3 };
        double  Mean    { 5e-3 };
        double  M2      { 0.0 };
        int64_t Count   { 1 };
        
        RStatus Initialize() noexcept override;
        SKL_NODISCARD const char *GetName( ) const noexcept override { return "[PreciseSleep_WaitableTimer]"; }
    };
}

#if defined(SKL_BUILD_WINDOWS)
    #define SKL_PLATFORM_NAME "Windows"
    #define SKL_WIN32_PLATFROM 1
    #include "Port_Windows.h"
#elif defined(SKL_BUILD_FREEBSD)
    #define SKL_PLATFORM_NAME "FreeBSD"
    #define SKL_FREEBSD_PLATFROM 1
    #include "Port_Unix.h"
    #include "Port_FreeBSD.h"
#elif defined(SKL_BUILD_UBUNTU)
    #define SKL_PLATFORM_NAME "Ubuntu"
    #define SKL_UBUNTU_PLATFROM 1
    #include "Port_Unix.h"
    #include "Port_Ubuntu.h"
#else
    #error "Unsupported platform!"
#endif
