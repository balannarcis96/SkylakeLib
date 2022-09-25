//!
//! \file Port.h
//! 
//! \brief Platform abstraction layer for SkylakeLib
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

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
    TSocket AllocateNewIPv4TCPSocket( bool bAsync = true ) noexcept;

    //! Allocate new ipv4 udp socket ( returns 0 on failure )
    TSocket AllocateNewIPv4UDPSocket( bool bAsync = true ) noexcept;
    
    //! Perform a TCP connect on socket to address and port
    bool TCPConnectIPv4( TSocket InSocket, TIPv4Address InAddress, TNetPort InPort ) noexcept;
}

#include "TCPAsyncAccepter.h"

namespace SKL
{
    //! Platform specific async IO API
    struct AsyncIO
    {   
        //! \brief Initialize the OS async IO system
        static RStatus InitializeSystem() noexcept;
        
        //! \brief Shutdown the OS async IO system
        static RStatus ShutdownSystem() noexcept;

        //! \brief Start and instance of the OS async IO system
        //! \param InThreadsCount number of threads accessing this instance
        //! \returns RSuccess on success
        //! \returns RFail on failure
        RStatus Start( int32_t InThreadsCount ) noexcept;

        //! \brief Stop this instance of the OS async IO system
        //! \remarks All outstanding async IO requests will be canceled
        //! \returns RSuccess on success
        //! \returns RAlreadyPerformed when Stop() was already called
        //! \returns RFail on failure
        RStatus Stop() noexcept;
        
        //! Get the OS specific handle to the API
        THandle GetOsHandle() const noexcept { return QueueHandle.load_relaxed(); }

        //! Get the max number of threads that can access this api instance at once
        int32_t GetNumberOfThreads() const noexcept { return ThreadsCount.load_relaxed(); }

        //! \brief Attempt to retrieve completed async IO request from the OS 
        //! \remarks Will block
        //! \param OutCompletedRequestOpaqueTypeInstancePtr ptr to ptr that will contain the instance of the opaque type passed to the OS when making the async IO request
        //! \param OutNumberOfBytesTransferred number of bytes transferred for the async IO request (eg. when recv/send/red/write async request is completed)
        //! \param OutCompletionKey key used to identify the completed async IO request
        //! \returns RSuccess when a valid async IO request or custom work item is retrieved
        //! \returns RSuccessAsyncIORequestCancelled when a async IO request was canceled due to the socket being closed
        //! \returns RSystemTerminated when a valid, terminate sentinel is retrieved, signaling the termination of the this async IO system instance
        //! \returns RSystemFailure when a system failure occurs
        RStatus GetCompletedAsyncRequest( AsyncIOOpaqueType** OutCompletedRequestOpaqueTypeInstancePtr, uint32_t* OutNumberOfBytesTransferred, TCompletionKey* OutCompletionKey ) noexcept;

        //! \brief Attempt to retrieve completed async IO request from the OS 
        //! \param OutCompletedRequestOpaqueTypeInstancePtr ptr to ptr that will contain the instance of the opaque type passed to the OS when making the async IO request
        //! \param OutNumberOfBytesTransferred number of bytes transferred for the async IO request (eg. when recv/send/red/write async request is completed)
        //! \param OutCompletionKey key used to identify the completed async IO request
        //! \param InTimeout Time in milliseconds to wait for any async IO request to be completed
        //! \returns RSuccess when a valid async IO request or custom work item is retrieved
        //! \returns RSuccessAsyncIORequestCancelled when a async IO request was canceled due to the socket being closed
        //! \returns RTimeout when reached timeout value with no completed async IO request retrieved
        //! \returns RSystemTerminated when a valid, terminate sentinel is retrieved, signaling the termination of the this async IO system instance
        //! \returns RSystemFailure when a system failure occurs
        RStatus TryGetCompletedAsyncRequest( AsyncIOOpaqueType** OutCompletedRequestOpaqueTypeInstancePtr, uint32_t* OutNumberOfBytesTransferred, TCompletionKey* OutCompletionKey, uint32_t InTimeout ) noexcept;

        //! \brief Attempt to enqueue custom async work
        //! \param InCompletionKey completion key representing the work (eg pointer to object)
        //! \return RSuccess on success
        //! \return RFail on failure
        RStatus QueueAsyncWork( TCompletionKey InCompletionKey ) noexcept;

        //! Associate the socket to this async IO API
        RStatus AssociateToTheAPI( TSocket InSocket ) const noexcept;

        //! \brief Start an async receive request on InSocket
        //! \param InSocket target stream socket to receive from
        //! \param InBuffer buffer to receive into 
        //! \param InOpaqueObject opaque object instance
        //! \return RSuccess on success
        //! \return RFail on failure
        static RStatus ReceiveAsync( TSocket InSocket, IBuffer* InBuffer, AsyncIOOpaqueType* InOpaqueObject ) noexcept;

        //! \brief Start an async send request on InSocket
        //! \param InSocket target stream socket to send to
        //! \param InBuffer buffer to receive into 
        //! \param InOpaqueObject opaque object instance
        //! \return RSuccess on success
        //! \return RFail on failure
        static RStatus SendAsync( TSocket InSocket, IBuffer* InBuffer, AsyncIOOpaqueType* InOpaqueObject ) noexcept;

        //! \brief Start an async send request on InSocket
        //! \param InSocket target stream socket to receive from
        //! \param InAsyncIOTask the send async IO task
        //! \return RSuccess on success
        //! \return RFail on failure
        static RStatus SendAsync( TSocket InSocket, IAsyncIOTask* InAsyncIOTask ) noexcept;

        //! \brief Start an async receive request on InSocket
        //! \param InSocket target stream socket to receive from
        //! \param InAsyncIOTask the send async IO task
        //! \return RSuccess on success
        //! \return RFail on failure
        static RStatus ReceiveAsync(TSocket InSocket, IAsyncIOTask* InAsyncIOTask ) noexcept;

    private:
        std::relaxed_value<THandle> QueueHandle  { 0 };
        std::relaxed_value<int32_t> ThreadsCount { 0 };
    };

    //! \brief Enable ANSI color support in the main console window
    //! \return RSuccess on success 
    RStatus EnableConsoleANSIColorSupport() noexcept;

    //! Get the number of milliseconds that have elapsed since the system was started
    TEpochTimePoint GetSystemUpTickCount() noexcept;

    //! Set the timer resolution of the OS
    RStatus SetOsTimeResolution( uint32_t InMilliseconds ) noexcept;

    //! Get the system l1 cache line size
    size_t GetL1CacheLineSize() noexcept;

    struct PlatformTLS
    {
        static constexpr TLSSlot INVALID_SLOT_ID = 0xFFFFFFFF;

        /**
         * \brief Return false if InSlotIndex is an invalid TLS slot
         * \param InSlotIndex the TLS index to check
         * \return true if InSlotIndex looks like a valid slot
         */
        SKL_FORCEINLINE static bool IsValidTlsSlot( TLSSlot InSlotIndex ) noexcept
        {
            return InSlotIndex != INVALID_SLOT_ID;
        }
        
        //! \brief Get the calling thread id
        static uint32_t GetCurrentThreadId() noexcept;

        //! \brief Allocate new thread local storage slot for all threads of the process
        static TLSSlot AllocTlsSlot() noexcept;

        //! \brief Set the TLS value at InSlot for the calling thread
        static void SetTlsValue( TLSSlot InSlot, void* InValue ) noexcept;

        //! \brief Get the TLS value at InSlot for the calling thread
        static void* GetTlsValue( TLSSlot InSlot ) noexcept;

        //! \brief Free a previously allocated TLS slot
        static void FreeTlsSlot( TLSSlot InSlot ) noexcept;
    };

    //! Very precise sleep
    void PreciseSleep( double InSeconds ) noexcept;

    //! Is socket valid
    bool IsValidSocket( TSocket InSocket ) noexcept;

    //! Close socket
    bool CloseSocket( TSocket InSocket ) noexcept;
    
    //Shutdown socket
    bool ShutdownSocket( TSocket InSocket ) noexcept;
    
    //! Get last OS error code
    int32_t GGetLastError( ) noexcept;
    
    //! Get last OS network operation related error code
    int32_t GGetNetworkLastError( ) noexcept;

    //! Convert ip v4 address string to binary
    uint32_t IPv4FromStringA( const char* InIpString )noexcept;

    //! Convert ip v4 address wide string to binary
    uint32_t IPv4FromStringW( const wchar_t* InIpString )noexcept;

    //! UTF16 -> UTF8
    bool GWideCharToMultiByte( const wchar_t * InBuffer, size_t InBufferSize, char* OutBuffer, int32_t InOutBufferSize ) noexcept;

    //! UTF8 -> UTF16
    bool GMultiByteToWideChar( const char * InBuffer, size_t InBufferSize, wchar_t* OutBuffer, int32_t InOutBufferSize ) noexcept;

    //! UTF16 -> UTF8
    template<size_t N>
    bool GWideCharToMultiByte( const wchar_t( &InBuffer ) [ N ], char* OutBuffer, int32_t OutBufferSize ) noexcept
    {
        return GWideCharToMultiByte( InBuffer, N, OutBuffer, OutBufferSize );
    }

    //! UTF8 -> UTF16
    template<size_t N>
    bool GMultiByteToWideChar( const char( &InBuffer ) [ N ], wchar_t* OutBuffer, int32_t OutBufferSize ) noexcept
    {
        return GMultiByteToWideChar( InBuffer, N, OutBuffer, OutBufferSize );
    }

    //! UTF16 -> UTF8
    template<size_t N, size_t M>
    bool GWideCharToMultiByte( const wchar_t( &InBuffer ) [ N ], char( &OutBuffer ) [ M ] ) noexcept
    {
        return GWideCharToMultiByte( InBuffer, N, OutBuffer, M );
    }

    //! UTF8 -> UTF16
    template<size_t N, size_t M>
    bool GMultiByteToWideChar( const char( &InBuffer ) [ N ], wchar_t( &OutBuffer ) [ M ] ) noexcept
    {
        return GMultiByteToWideChar( InBuffer, N, OutBuffer, M );
    }
}

#include "TLSValue.h"
#include "TLSSingleton.h"

namespace SKL
{
    struct PreciseSleep_WaitableTimer: public ITLSSingleton<PreciseSleep_WaitableTimer>
    {
        PreciseSleep_WaitableTimer() noexcept = default;
        ~PreciseSleep_WaitableTimer() noexcept = default;

        void*   Timer   { nullptr };
        double  Estimate{ 5e-3 };
        double  Mean    { 5e-3 };
        double  M2      { 0.0 };
        int64_t Count   { 1 };
        
        RStatus Initialize() noexcept override;
        const char *GetName( ) const noexcept override { return "[PreciseSleep_WaitableTimer]"; }
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
