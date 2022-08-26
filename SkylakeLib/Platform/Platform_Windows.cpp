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
#include <mswsock.h>

#pragma comment( lib, "ws2_32.lib" )
#pragma comment( lib, "winmm.lib" )

namespace SKL
{
    static_assert( std::is_same_v<TSocket, SOCKET>, "Invalid Socket type!" );
    static_assert( sizeof( AsyncIOOpaqueType ) == sizeof( OVERLAPPED ), "AsyncIOOpaqueType must be updated!" );
    static_assert( sizeof( int64_t ) == sizeof( LARGE_INTEGER ), "Timer must be updated!" );
    static_assert( std::is_same_v<std::invoke_result_t<decltype(::GetTickCount64)>, TEpochTimePoint>, "TEpochTimePoint must be updated!" );
    static_assert( std::is_same_v<std::invoke_result_t<decltype(::GetTickCount64)>, TSystemTimePoint>, "TSystemTimePoint must be updated!" );
}

namespace SKL
{
    bool Timer::Init() noexcept
    {
        if( FALSE == ::QueryPerformanceFrequency( reinterpret_cast<LARGE_INTEGER*>( &I ) ) ) SKL_UNLIKELY
        {
            return false;
        }

        FrequencySeconds = static_cast< double >( reinterpret_cast<LARGE_INTEGER*>( &I )->QuadPart );
        ::QueryPerformanceCounter( reinterpret_cast<LARGE_INTEGER*>( &I ) );

        Start     = reinterpret_cast<LARGE_INTEGER*>( &I )->QuadPart;
        TotalTime = 0.0;
        Elapsed   = 0.0;

        return true;
    }

    double Timer::Tick() noexcept
    {
        QueryPerformanceCounter( reinterpret_cast<LARGE_INTEGER*>( &I ) );

        Elapsed    = static_cast< double >( reinterpret_cast<LARGE_INTEGER*>( &I )->QuadPart - Start ) / FrequencySeconds;
        Start      = reinterpret_cast<LARGE_INTEGER*>( &I )->QuadPart;
        TotalTime += Elapsed;

        return TotalTime;
    }
}

namespace SKL
{
    TSocket AllocateNewIPv4TCPSocket( bool bAsync ) noexcept
    {
        SKL_ASSERT( true == Skylake_IsTheLibraryInitialize() );

        TSocket Result { ::WSASocketW( AF_INET
                             , SOCK_STREAM
                             , IPPROTO_TCP
                             , nullptr
                             , 0
                             , bAsync ? WSA_FLAG_OVERLAPPED : 0 ) };
        if( Result == INVALID_SOCKET )
        {
            SKL_VER_FMT( "AllocateNewTCPSocket() Failed with WSAError:%d", WSAGetLastError() );
            Result = 0;
        }
        
        return Result;
    }

    TSocket AllocateNewIPv4UDPSocket( bool bAsync ) noexcept
    {
        SKL_ASSERT( true == Skylake_IsTheLibraryInitialize() );

        TSocket Result { ::WSASocketW( AF_INET
                             , SOCK_DGRAM
                             , IPPROTO_UDP
                             , nullptr
                             , 0
                             , bAsync ? WSA_FLAG_OVERLAPPED : 0 ) };
        if( Result == INVALID_SOCKET )
        {
            SKL_VER_FMT( "AllocateNewTCPSocket() Failed with WSAError:%d", WSAGetLastError() );
            Result = 0;
        }
        
        return Result;
    }
}

namespace SKL
{
    GUID GGuidAcceptEx = WSAID_ACCEPTEX;

    LPFN_ACCEPTEX Win32_AcquireAcceptEx( TSocket InSocket ) noexcept
    {
        LPFN_ACCEPTEX Output        { nullptr };
        DWORD         BytesReturned { 0 };
        
        // Load the AcceptEx function into memory using WSAIoctl.
        // The WSAIoctl function is an extension of the ioctlsocket()
        // function that can use overlapped I/O. The function's 3rd
        // through 6th parameters are input and output buffers where
        // we pass the pointer to our AcceptEx function. This is used
        // so that we can call the AcceptEx function directly, rather
        // than refer to the Mswsock.lib library.
        const int32_t Result{ WSAIoctl( InSocket
                                      , SIO_GET_EXTENSION_FUNCTION_POINTER
                                      , &GGuidAcceptEx
                                      , sizeof (GGuidAcceptEx) 
                                      , &Output
                                      , sizeof (Output) 
                                      , &BytesReturned
                                      , nullptr
                                      , nullptr ) };
        if ( SOCKET_ERROR == Result ) 
        {
            return nullptr;
        }
        
        return Output;
    }

    RStatus TCPAcceptor::StartAcceptingAsync() noexcept
    {
        if( true == IsAccepting() )
        {
            SKL_VER( "TCPAccepter::StartAcceptingAsync() Already accepting!" );
            return RSuccess;
        }

        if( false == IsValid() )
        {
            SKL_ERR( "TCPAccepter::StartAcceptingAsync() Failed, invalid config!" );
            return RInvalidParamters;
        }   

        // close potential open socket
        CloseSocket();
    
        // create new socket
        const TSocket NewSocket { AllocateNewIPv4TCPSocket( true ) };
        if( 0 == NewSocket )
        {
            SKL_ERR( "TCPAccepter::StartAcceptingAsync() Failed to create new tcp socket!" );
            return RFail;
        }

        // cache socket
        Socket.exchange( NewSocket );

        if ( false == BindAndListen() )
        {
            CloseSocket();
            return RFail;
        }

        if ( RSuccess != AsyncIOAPI->AssociateToTheAPI( NewSocket ) )
        {
            SKL_ERR( "TCPAccepter::StartAcceptingAsync() Failed enable async io on socket!" );
            closesocket( NewSocket );
            Socket.exchange( 0 );
            return RFail;
        }

        LPFN_ACCEPTEX AcceptExPtr { Win32_AcquireAcceptEx( NewSocket ) };
        if( nullptr == AcceptExPtr )
        {
            SKL_ERR_FMT( "TCPAccepter::StartAcceptingAsync() Failed acquire AcceptEx on address[%08x] port[%hu] WSAErr:%d", Config.IpAddress, Config.Port, WSAGetLastError() );
            CloseSocket();
            return RFail;
        }

        // cache accept ex
        CustomHandle.exchange( AcceptExPtr );

        // set is running 
        bIsRunning.exchange( true );

        if( false == BeginAcceptAsync() )
        {
            SKL_ERR_FMT( "TCPAccepter::StartAcceptingAsync() Failed start AcceptEx on address[%08x] port[%hu] WSAErr:%d", Config.IpAddress, Config.Port, WSAGetLastError() );
            CloseSocket();
            CustomHandle.exchange( nullptr );

            // set is not running 
            bIsRunning.exchange( false );

            return RFail;
        }
        
        return RSuccess;
    }
    
    bool TCPAcceptor::BeginAcceptAsync( void* InAcceptTask ) noexcept
    {
        using AsyncAcceptTask = AsyncIOBuffer<64, 16>; // 32bytes should suffice, update otherwise

        AsyncAcceptTask* AcceptTask;
        if ( nullptr == InAcceptTask ) SKL_UNLIKELY
        {
            AcceptTask = MakeSharedRaw<AsyncAcceptTask>();
            if( nullptr == AcceptTask ) SKL_UNLIKELY
            {
                SKL_VER( "TCPAccepter::BeginAcceptAsync() Failed to allocate task!" );
                return false;
            }
        }
        else
        {
            AcceptTask = reinterpret_cast<AsyncAcceptTask*>( InAcceptTask );
            
            // Increment the reference count for the reused task so it will not be destroyed
            TSharedPtr<AsyncAcceptTask>::Static_IncrementReference( AcceptTask );

            // guard
            SKL_ASSERT( 2 == TSharedPtr<AsyncAcceptTask>::Static_GetReferenceCount( AcceptTask ) );
        }

        // Allocate new tcp socket
        TSocket AcceptSocket { AllocateNewIPv4TCPSocket() };

        AcceptTask->SetCompletionHandler( [ this, AcceptSocket ]( IAsyncIOTask& Self, uint32_t NumberOfBytesTransferred ) noexcept -> void
        {
            const TSocket ListenSocket{ GetSocket() };

            const auto UpdateResult { setsockopt( AcceptSocket
                                                , SOL_SOCKET
                                                , SO_UPDATE_ACCEPT_CONTEXT
                                                , reinterpret_cast<const char*>( &ListenSocket )
                                                , sizeof( TSocket ) ) };
            if( SOCKET_ERROR == UpdateResult  ) SKL_UNLIKELY
            {
                SKL_VER_FMT( "TCPAccepter [AsyncIOCompletionHandler]:: Failed to accept WSAError:%d!", WSAGetLastError() );
                closesocket( AcceptSocket );
                StopAcceptingAsync();
                return;        
            }

            if ( RSuccess != AsyncIOAPI->AssociateToTheAPI( AcceptSocket ) ) SKL_UNLIKELY
            {
                SKL_VER_FMT( "TCPAccepter [AsyncIOCompletionHandler]:: Failed to associate to the AsyncIO API WSAError:%d!", WSAGetLastError() );
                closesocket( AcceptSocket );
                StopAcceptingAsync();
                return;
            }

            // dispatch the accept handler
            GetConfig().OnAccept( AcceptSocket );
        
            if ( true == IsAccepting() ) SKL_LIKELY
            {
                // continue to accept
                const auto AcceptResult { BeginAcceptAsync( &Self ) };
                if ( false == AcceptResult )
                {
                    SKL_VER_FMT( "TCPAccepter [AsyncIOCompletionHandler]:: Failed to start to accept again WSAError:%d!", WSAGetLastError() );
                    return;
                }
            }
        } );

        auto    AcceptEx      { reinterpret_cast<LPFN_ACCEPTEX>( CustomHandle.load_relaxed() ) };
        auto    Buffer        { AcceptTask->GetInterface() };
        DWORD   BytesReceived { 0 };
        
        // Prepare the opaque object
        memset( AcceptTask->ToOSOpaqueObject(), 0, sizeof( AsyncIOOpaqueType ) );

        const BOOL AcceptResult { AcceptEx( Socket.load_relaxed() 
                                    , AcceptSocket
                                    , Buffer.Buffer
                                    , 0
                                    , sizeof( sockaddr_in ) + 16
                                    , sizeof( sockaddr_in ) + 16
                                    , &BytesReceived
                                    , reinterpret_cast<OVERLAPPED*>( AcceptTask )
                                 ) };
        if( FALSE == AcceptResult ) SKL_UNLIKELY
        {
            const auto WSALastError{ WSAGetLastError() };
            if( WSA_IO_PENDING != WSALastError ) SKL_UNLIKELY
            {
                SKL_ERR_FMT( "TCPAccepter::BeginAcceptAsync() Failed to AcceptEx WSAError:%d!", WSALastError );
                closesocket( AcceptSocket );
                TSharedPtr<AsyncAcceptTask>::Static_Reset( AcceptTask );
                return false;
            }
        }

        return true;
    }

    void TCPAcceptor::StopAcceptingAsync() noexcept
    {
        if( false == bIsRunning.exchange( false ) )
        {
            SKL_VER( "TCPAccepter::StopAcceptingAsync() Already stopped!" );
            return;
        }

        CloseSocket();
    }

    void TCPAcceptor::CloseSocket() noexcept
    {
        auto ExistingSocket { Socket.exchange( 0 ) };
        if( 0 == ExistingSocket )
        {
            return;
        }

        closesocket( ExistingSocket );
        shutdown( ExistingSocket, SD_BOTH );
    }

    bool TCPAcceptor::BindAndListen() noexcept
    {
        const ::sockaddr_in Address {
            .sin_family = AF_INET,
            .sin_port   = htons( Config.Port ),
            .sin_addr   = { .S_un = { .S_addr = Config.IpAddress } },
            .sin_zero   = { 0, 0, 0, 0, 0, 0, 0, 0 }
        };  

        int32_t Result { ::bind( Socket.load_relaxed()
                              ,  reinterpret_cast<const sockaddr*>( &Address )
                              ,  sizeof( Address ) ) };
        if( SOCKET_ERROR == Result )
        {
            SKL_ERR_FMT( "TCPAccepter::Bind() Failed to BIND on address[%08x] port[%hu] WSAErr:%d", Config.IpAddress, Config.Port, WSAGetLastError() );
            return false;
        }
        
        Result = listen( Socket.load_relaxed()
                       , static_cast<int32_t>( Config.Backlog ) );
        if( SOCKET_ERROR == Result )
        {
            SKL_ERR_FMT( "TCPAccepter::Bind() Failed to LISTEN on address[%08x] port[%hu] WSAErr:%d", Config.IpAddress, Config.Port, WSAGetLastError() );
            return false;
        }

        return true;
    }
}

namespace SKL
{
    WSADATA       GWSAData;                 //!< WSADATA global instance

    RStatus AsyncIO::InitializeSystem() noexcept 
    {
        if ( const int32_t WSAStartupResult = ::WSAStartup( MAKEWORD( 2, 2 ), &GWSAData ); WSAStartupResult )
        {
            SKL_ERR_FMT( "AsyncIO::Initialize() Failed to WSAStartup() returned [%d] WSAERROR: %d", WSAStartupResult, ::WSAGetLastError() );
            return RFail;
        }

        return RSuccess;
    }

    RStatus AsyncIO::ShutdownSystem() noexcept 
    {
        if ( const int32_t WSACleanupResult = ::WSACleanup( ); WSACleanupResult )
        {
            SKL_ERR_FMT( "Win32AsyncIO::SkylakeCore_Shutdown() Failed to WSAStartup() returned [%d] WSAERROR: %d", WSACleanupResult, ::WSAGetLastError() );
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
            SKL_ERR_FMT( "Win32AsyncIO::Start() Failed to create IOCP Handle WSAERROR[%d]", LastWSAError );
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
            SKL_ERR_FMT( "AsyncIO::QueueAsyncWork() failed with WSAERROR[%d]", LastWSAError );
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
                SKL_ERR_FMT( "AsyncIO::ReceiveAsync() failed with WSAERROR[%d]", LastWSAError );
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
                SKL_ERR_FMT( "AsyncIO::SendAsync() failed with WSAERROR[%d]", LastWSAError );
                return RFail;
            }
        }

        return RSuccess;
    }

    RStatus AsyncIO::SendAsync( TSocket InSocket, IAsyncIOTask* InAsyncIOTask ) noexcept
    {
        return SendAsync( InSocket, &InAsyncIOTask->GetInterface(), InAsyncIOTask->ToOSOpaqueObject() );
    }

    RStatus AsyncIO::ReceiveAsync( TSocket InSocket, IAsyncIOTask* InAsyncIOTask ) noexcept
    {
        return ReceiveAsync( InSocket, &InAsyncIOTask->GetInterface(), InAsyncIOTask->ToOSOpaqueObject() );
    }

    RStatus AsyncIO::AssociateToTheAPI( TSocket InSocket ) noexcept
    {
        const auto Result = ::CreateIoCompletionPort( reinterpret_cast<HANDLE>( InSocket )
                                                    , reinterpret_cast<HANDLE>( QueueHandle.load_relaxed() )
                                                    , 0
                                                    , 0 );
        if( nullptr == Result ) SKL_UNLIKELY
        {
            const auto LastWSAError = ::WSAGetLastError();
            SKL_ERR_FMT( "Win32AsyncIO::AssociateToTheAPI() Failed to associate socket to the IOCP Handle WSAERROR[%d]", LastWSAError );
            return RFail;
        }

        return RSuccess;
    }
}

namespace SKL
{
    RStatus EnableConsoleANSIColorSupport() noexcept
    {
        // Set output mode to handle virtual terminal sequences
        HANDLE hOut { ::GetStdHandle( STD_OUTPUT_HANDLE ) };
        if( hOut == INVALID_HANDLE_VALUE )
        {
            return RSTATUS_FROM_NUMERIC( ::GetLastError( ) );
        }

        DWORD dwMode { 0 };
        if( FALSE == ::GetConsoleMode( hOut, &dwMode ) )
        {
            return RSTATUS_FROM_NUMERIC( ::GetLastError( ) );
        }

        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
        if( FALSE == ::SetConsoleMode( hOut, dwMode ) )
        {
            return RSTATUS_FROM_NUMERIC( ::GetLastError( ) );
        }

        return RSuccess;
    }

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

    RStatus PreciseSleep_WaitableTimer::Initialize() noexcept
    {
        Timer = CreateWaitableTimer( NULL, FALSE, NULL );
        SKL_ASSERT( nullptr != Timer );
        return RSuccess;
    }

    //! source: https://blat-blatnik.github.io/computerBear/making-accurate-sleep-function/
    void PreciseSleep( double InSeconds ) noexcept
    {
        using namespace std::chrono;

        auto* Timer{ PreciseSleep_WaitableTimer::GetInstance() };
        SKL_ASSERT( nullptr != Timer );        

        while( InSeconds - Timer->Estimate > 1e-7 ) 
        {
            double        ToWait{ InSeconds - Timer->Estimate };
            LARGE_INTEGER Due   { .QuadPart = -int64_t( ToWait * 1e7 ) };
            const auto    Start { high_resolution_clock::now() };

            ( void )::SetWaitableTimerEx( Timer->Timer, &Due, 0, NULL, NULL, NULL, 0 );
            ( void )::WaitForSingleObject( Timer->Timer, INFINITE );

            const auto   End     { high_resolution_clock::now() };
            const double Observed{ (End - Start).count() / 1e9 };

            InSeconds -= Observed;

            ++Timer->Count;
            double Error = Observed - ToWait;
            double Delta = Error - Timer->Mean;
            Timer->Mean += Delta / Timer->Count;
            Timer->M2   += Delta * ( Error - Timer->Mean );
            double Stddev = sqrt( Timer->M2 / (Timer->Count - 1) );
            Timer->Estimate = Timer->Mean + Stddev;
        }

        // Spin lock
        const auto Start{ high_resolution_clock::now() };
        while ( ( high_resolution_clock::now() - Start ).count() / 1e9 < InSeconds );
    }

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

    int32_t GGetLastError( ) noexcept
    {
        return ( int32_t )GetLastError( );
    }

    bool IsValidSocket( TSocket InSocket ) noexcept
    {
        return INVALID_SOCKET != InSocket &&
               0              != InSocket;
    }

    bool CloseSocket( TSocket InSocket ) noexcept   
    {
        return 0 == closesocket( InSocket );
    }

    bool ShutdownSocket( TSocket InSocket ) noexcept
    {
        return 0 == shutdown( InSocket, SD_BOTH );
    }

    uint32_t IPv4FromStringA( const char* IpString )noexcept
    {
        in_addr addr;
        if ( ::InetPtonA( AF_INET, IpString, &addr ) != 1 )
        {
            return 0;
        }

        return addr.S_un.S_addr;
    }

    uint32_t IPv4FromStringW( const wchar_t* IpString )noexcept
    {
        in_addr addr;
        if ( ::InetPtonW( AF_INET, IpString, &addr ) != 1 )
        {
            return 0;
        }

        return addr.S_un.S_addr;
    }

    bool GWideCharToMultiByte( const wchar_t * InBuffer, size_t InBufferSize, char* OutBuffer, int32_t InOutBufferSize ) noexcept
    {
        int32_t result = 0;
        if ( ( result = ::WideCharToMultiByte( CP_UTF8, 0, InBuffer, ( int32_t )wcsnlen_s( InBuffer, InBufferSize ), OutBuffer, InOutBufferSize, nullptr, nullptr ) ) == 0 )
        {
            return false;
        }

        OutBuffer [ result ] = '\0';

        return true;
    }

    bool GMultiByteToWideChar( const char * InBuffer, size_t InBufferSize, wchar_t* OutBuffer, int32_t InOutBufferSize ) noexcept
    {
        int32_t result = 0;
        if ( ( result = ::MultiByteToWideChar( CP_UTF8, 0, InBuffer, ( int32_t )strnlen_s( InBuffer, InBufferSize ), OutBuffer, InOutBufferSize ) ) == 0 )
        {
            return false;
        }

        OutBuffer [ result ] = '\0';

        return true;
    }

    size_t GetL1CacheLineSize() noexcept
    {
        //source: https://github.com/NickStrupat/CacheLineSize/blob/master/CacheLineSize.c

        size_t lineSize = 0;
        DWORD bufferSize = 0;
        DWORD i = 0;
        SYSTEM_LOGICAL_PROCESSOR_INFORMATION * buffer = 0;

        GetLogicalProcessorInformation(0, &bufferSize);
        buffer = (SYSTEM_LOGICAL_PROCESSOR_INFORMATION *) malloc(bufferSize);
        SKL_ASSERT(nullptr != buffer);
        GetLogicalProcessorInformation(&buffer[0], &bufferSize);

        for (i = 0; i != bufferSize / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION); ++i) {
            if (buffer[i].Relationship == RelationCache && buffer[i].Cache.Level == 1) {
                lineSize = buffer[i].Cache.LineSize;
                break;
            }
        }

        free(buffer);
        return lineSize;
    }
}

