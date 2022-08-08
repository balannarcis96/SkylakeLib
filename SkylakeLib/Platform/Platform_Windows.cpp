//!
//! \file Platform_Windows.cpp
//! 
//! \brief Windows platform abstraction layer for SkylakeLib
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 

#include "../SkylakeLib.h"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <WS2tcpip.h>
#include <Windows.h>
#include <Winsock2.h>
#include <timeapi.h>

#pragma comment( lib, "ws2_32.lib" )
#pragma comment( lib, "winmm.lib" )

namespace SKL
{
    //! WSADATA global instance
    WSADATA GWSAData;

    static_assert( std::is_same_v<TSocket, SOCKET>, "Invalid Socket type!" );
    static_assert( sizeof( AsyncIOOpaqueType ) == sizeof( OVERLAPPED ), "AsyncIOOpaqueType must be updated!" );
    static_assert( sizeof( int64_t ) == sizeof( LARGE_INTEGER ), "Timer must be updated!" );
    static_assert( std::is_same_v<std::invoke_result_t<decltype(::GetTickCount64)>, TEpochTimePoint>, "TEpochTimePoint must be updated!" );
    static_assert( std::is_same_v<std::invoke_result_t<decltype(::GetTickCount64)>, TSystemTimePoint>, "TSystemTimePoint must be updated!" );
    
    TEpochTimePoint GetSystemUpTickCount() noexcept
    {
        return ::GetTickCount64( );
    }

    RStatus SetOsTimeResolution( uint32_t InMilliseconds ) noexcept
    {
        if ( ::timeBeginPeriod( InMilliseconds ) != TIMERR_NOERROR )
        {
            return RFail;
        }
    
        ::Sleep( 128 );  /* wait for it to stabilize */
    
        return RSuccess;
    }

    bool Timer::Init() noexcept
    {
        if( FALSE == ::QueryPerformanceFrequency( reinterpret_cast<LARGE_INTEGER*>( &I ) ) ) SKL_UNLIKELY
        {
            return false;
        }

        FrequencySeconds = static_cast< double >( reinterpret_cast<LARGE_INTEGER*>( &I )->QuadPart );
        ::QueryPerformanceCounter( reinterpret_cast<LARGE_INTEGER*>( &I ) );
        Start      = reinterpret_cast<LARGE_INTEGER*>( &I )->QuadPart;
        TotalTime = 0.0;
        Elapsed      = 0.0;

        return true;
    }

    double Timer::Tick() noexcept
    {
        QueryPerformanceCounter( reinterpret_cast<LARGE_INTEGER*>( &I ) );
        Elapsed = static_cast< double >( reinterpret_cast<LARGE_INTEGER*>( &I )->QuadPart - Start ) / FrequencySeconds;

        Start = reinterpret_cast<LARGE_INTEGER*>( &I )->QuadPart;
        TotalTime += Elapsed;

        return TotalTime;
    }

    RStatus AsyncIO::InitializeSystem() noexcept 
    {
        if ( const int32_t WSAStartupResult = ::WSAStartup( MAKEWORD( 2, 2 ), &GWSAData ); WSAStartupResult )
        {
            printf( "AsyncIO::Initialize() Failed to WSAStartup() returned [%d] WSAERROR: %d", WSAStartupResult, ::WSAGetLastError() );
            return RFail;
        }

        return RSuccess;
    }

    RStatus AsyncIO::ShutdownSystem() noexcept 
    {
        if ( const int32_t WSACleanupResult = ::WSACleanup( ); WSACleanupResult )
        {
            printf( "Win32AsyncIO::SkylakeCore_Shutdown() Failed to WSAStartup() returned [%d] WSAERROR: %d", WSACleanupResult, ::WSAGetLastError() );
            return RFail;
        }

        return RSuccess;
    }

    RStatus AsyncIO::Start( int32_t InThreadsCount ) noexcept
    {
        ThreadsCount = InThreadsCount;
        
        // create the queue
        const auto Result = ::CreateIoCompletionPort( INVALID_HANDLE_VALUE, nullptr, NULL, InThreadsCount );
        if( nullptr == Result ) SKL_UNLIKELY
        {
            const auto LastWSAError = ::WSAGetLastError();
            printf( "Win32AsyncIO::Start() Failed to create IOCP Handle WSAERROR[%d]", LastWSAError );
            return RFail;
        }
    
        const auto OldHandle = QueueHandle.exchange( reinterpret_cast<THandle>( Result ) );
        if( 0 != OldHandle )
        {
            ::CloseHandle( reinterpret_cast<HANDLE>( OldHandle ) );
        }

        return RSuccess;
    }
    
    RStatus AsyncIO::Stop() noexcept
    {
        auto ExistingHandler = QueueHandle.exchange( 0 );
        if( 0 == ExistingHandler )
        {
            return RAlreadyPerformed;
        }

        if( FALSE == ::CloseHandle( reinterpret_cast<HANDLE>( ExistingHandler ) ) )
        {
            return RFail;
        }

        return RSuccess;
    }
    
    RStatus AsyncIO::GetCompletedAsyncRequest( AsyncIOOpaqueType** OutCompletedRequestOpaqueTypeInstancePtr, uint32_t* OutNumberOfBytesTransferred, TCompletionKey* OutCompletionKey ) noexcept
    {
        // sys call to get completed async IO requests or custom queued work
        const auto GetResult = ::GetQueuedCompletionStatus( 
                reinterpret_cast<HANDLE>( QueueHandle.load() ) 
              , reinterpret_cast<LPDWORD>( OutNumberOfBytesTransferred )
              , reinterpret_cast<PULONG_PTR>( OutCompletionKey )
              , reinterpret_cast<OVERLAPPED**>( OutCompletedRequestOpaqueTypeInstancePtr )
              , INFINITE
        );

        if( FALSE == GetResult ) SKL_UNLIKELY
        {
            const auto LastWSAError = ::WSAGetLastError( );
            
            if( WSA_OPERATION_ABORTED == LastWSAError ) SKL_LIKELY
            {
                /*
                 * Overlapped operation aborted.
                 * This Windows error indicates that an overlapped I/O operation was canceled because of the closure of a socket.
                 * In addition, this error can occur when executing the SIO_FLUSH ioctl command.
                 */

                return RSuccessAsyncIORequestCancelled;
            }

            if( ERROR_NETNAME_DELETED == LastWSAError ) SKL_UNLIKELY
            {
                /*
                 * NTSTATUS -> WINAPI Error mappings in this case
                 * 
                 * STATUS_NETWORK_NAME_DELETED      ->  ERROR_NETNAME_DELETED
                 * STATUS_LOCAL_DISCONNECT          ->  ERROR_NETNAME_DELETED
                 * STATUS_REMOTE_DISCONNECT         ->  ERROR_NETNAME_DELETED
                 * STATUS_ADDRESS_CLOSED            ->  ERROR_NETNAME_DELETED
                 * STATUS_CONNECTION_DISCONNECTED   ->  ERROR_NETNAME_DELETED
                 * STATUS_CONNECTION_RESET          ->  ERROR_NETNAME_DELETED
                 */

                return RSuccessAsyncIORequestCancelled;
            }

            return RSystemFailure;
        }

        return RSuccess;
    }
    
    RStatus AsyncIO::TryGetCompletedAsyncRequest( AsyncIOOpaqueType** OutCompletedRequestOpaqueTypeInstancePtr, uint32_t* OutNumberOfBytesTransferred, TCompletionKey* OutCompletionKey, uint32_t InTimeout ) noexcept
    {
        // sys call to get completed async IO requests or custom queued work
        const auto GetResult = ::GetQueuedCompletionStatus( 
                reinterpret_cast<HANDLE>( QueueHandle.load() ) 
              , reinterpret_cast<LPDWORD>( OutNumberOfBytesTransferred )
              , reinterpret_cast<PULONG_PTR>( OutCompletionKey )
              , reinterpret_cast<OVERLAPPED**>( OutCompletedRequestOpaqueTypeInstancePtr )
              , static_cast<DWORD>( InTimeout )
        );

        if( FALSE == GetResult ) SKL_LIKELY
        {
            const auto LastWSAError = ::WSAGetLastError( );
            
            if( WAIT_TIMEOUT == LastWSAError ) SKL_LIKELY
            {
                return RTimeout;
            }

            if( WSA_OPERATION_ABORTED == LastWSAError )
            {
                /*
                 * Overlapped operation aborted.
                 * This Windows error indicates that an overlapped I/O operation was canceled because of the closure of a socket.
                 * In addition, this error can occur when executing the SIO_FLUSH ioctl command.
                 */

                return RSuccessAsyncIORequestCancelled;
            }

            if( ERROR_NETNAME_DELETED == LastWSAError ) SKL_UNLIKELY
            {
                /*
                 * NTSTATUS -> WINAPI Error mappings in this case
                 * 
                 * STATUS_NETWORK_NAME_DELETED      ->  ERROR_NETNAME_DELETED
                 * STATUS_LOCAL_DISCONNECT          ->  ERROR_NETNAME_DELETED
                 * STATUS_REMOTE_DISCONNECT         ->  ERROR_NETNAME_DELETED
                 * STATUS_ADDRESS_CLOSED            ->  ERROR_NETNAME_DELETED
                 * STATUS_CONNECTION_DISCONNECTED   ->  ERROR_NETNAME_DELETED
                 * STATUS_CONNECTION_RESET          ->  ERROR_NETNAME_DELETED
                 */

                return RSuccessAsyncIORequestCancelled;
            }

            return RSystemFailure;
        }

        return RSuccess;
    }
    
    RStatus AsyncIO::QueueAsyncWork( TCompletionKey InCompletionKey ) noexcept
    {
        // sys call to enqueue custom work request
        const auto Result = ::PostQueuedCompletionStatus( 
            reinterpret_cast<HANDLE>( QueueHandle.load() ) 
          , sizeof( TCompletionKey )
          , reinterpret_cast<ULONG_PTR>( InCompletionKey )
          , nullptr
        );
        if( FALSE == Result ) SKL_UNLIKELY
        {
            const auto LastWSAError = ::WSAGetLastError( );
            printf( "AsyncIO::QueueAsyncWork() failed with WSAERROR[%d]\n", LastWSAError );
            return RFail;
        }

        return RSuccess;
    }
    
    RStatus AsyncIO::ReceiveAsync( TSocket InSocket, IBuffer* InBuffer, AsyncIOOpaqueType* InOpaqueObject ) noexcept
    {
        DWORD NumberOfBytesReceived { 0 };
        DWORD Flags { 0 };

        // sys call to start the receive async IO request
        const auto Result = ::WSARecv(
            static_cast<SOCKET>( InSocket )
          , reinterpret_cast<LPWSABUF>( InBuffer )
          , 1
          , &NumberOfBytesReceived
          , &Flags
          , reinterpret_cast<OVERLAPPED*>( InOpaqueObject )
          , nullptr
        );
        if ( SOCKET_ERROR == Result ) SKL_LIKELY
        {    
            const auto LastWSAError = WSAGetLastError( );
            if( LastWSAError != WSA_IO_PENDING ) SKL_UNLIKELY
            {
                printf( "AsyncIO::ReceiveAsync() failed with WSAERROR[%d]\n", LastWSAError );
                return RFail;
            }
        }

        return RSuccess;
    }
    
    RStatus AsyncIO::SendAsync( TSocket InSocket, IBuffer* InBuffer, AsyncIOOpaqueType* InOpaqueObject ) noexcept
    {
        DWORD NumberOfBytesReceived { 0 };

        // sys call to start the receive async IO request
        const auto Result = ::WSASend(
            static_cast<SOCKET>( InSocket )
          , reinterpret_cast<LPWSABUF>( InBuffer )
          , 1
          , &NumberOfBytesReceived
          , 0
          , reinterpret_cast<OVERLAPPED*>( InOpaqueObject )
          , nullptr
        );
        if ( SOCKET_ERROR == Result ) SKL_LIKELY
        {    
            const auto LastWSAError = WSAGetLastError( );
            if( LastWSAError != WSA_IO_PENDING ) SKL_UNLIKELY
            {
                printf( "AsyncIO::SendAsync() failed with WSAERROR[%d]\n", LastWSAError );
                return RFail;
            }
        }

        return RSuccess;
    }
}

namespace SKL
{
    uint32_t PlatformTLS::GetCurrentThreadId() noexcept
    {
        return ::GetCurrentThreadId( );
    }
    
    TLSSlot PlatformTLS::AllocTlsSlot() noexcept
    {
        return static_cast<TLSSlot>( ::TlsAlloc() );
    }
    
    void PlatformTLS::SetTlsValue( TLSSlot InSlot, void* InValue ) noexcept
    {
        ::TlsSetValue( static_cast<DWORD>( InSlot ), InValue );
    }
    
    void* PlatformTLS::GetTlsValue( TLSSlot InSlot ) noexcept
    {
        return ::TlsGetValue( static_cast<DWORD>( InSlot ) );
    }
    
    void PlatformTLS::FreeTlsSlot( TLSSlot InSlot ) noexcept
    {
        ::TlsFree( static_cast<DWORD>( InSlot ) );
    }
}

