//!
//! \file Port_Windows.cpp
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

#if !defined(SKL_USE_MIMALLOC)
#include <malloc.h>

void* GAllocAligned( size_t Size, size_t Alignment ) noexcept
{
    return _aligned_malloc( Size, Alignment );
}

void GFreeAligned( void* Ptr ) noexcept
{
    return _aligned_free( Ptr );
}                   
#endif

namespace SKL
{
    static_assert( std::is_same_v<TSocket, SOCKET>, "Invalid Socket type!" );
    static_assert( sizeof( AsyncIOOpaqueType ) == sizeof( OVERLAPPED ), "AsyncIOOpaqueType must be updated!" );
    static_assert( sizeof( AsyncIOOpaqueEntryType ) == sizeof( OVERLAPPED_ENTRY ), "AsyncIOOpaqueEntryType must be updated!" );
    static_assert( sizeof( int64_t ) == sizeof( LARGE_INTEGER ), "Timer must be updated!" );
    static_assert( std::is_same_v<std::invoke_result_t<decltype(::GetTickCount64)>, TEpochTimePoint>, "TEpochTimePoint must be updated!" );
    static_assert( std::is_same_v<std::invoke_result_t<decltype(::GetTickCount64)>, TSystemTimePoint>, "TSystemTimePoint must be updated!" );
    static_assert( CInvalidSocket == INVALID_SOCKET, "Invalid [invalid] socket value!" );
}

namespace SKL
{
    static_assert( OS_ERROR_NET_TIMEOUT == WSAETIMEDOUT, "Invalid [NET_TIMEOUT] value!" );
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
            SKLL_VER_FMT( "AllocateNewTCPSocket() Failed with WSAError:%d", WSAGetLastError() );
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
            SKLL_VER_FMT( "AllocateNewTCPSocket() Failed with WSAError:%d", WSAGetLastError() );
            Result = 0;
        }
        
        return Result;
    }

    bool TCPConnectIPv4( TSocket InSocket, TIPv4Address InAddress, TNetPort InPort ) noexcept
    {
        sockaddr_in Target;
        Target.sin_port           = htons( InPort );
        Target.sin_family       = AF_INET;
        Target.sin_addr.s_addr = InAddress;

        const int32_t ConnectResult{ connect( InSocket
            , reinterpret_cast<const sockaddr*>( &Target )
            , static_cast<int32_t>( sizeof( Target ) ) ) };
        if (SOCKET_ERROR == ConnectResult)
        {
            return false;
        }

        return true;
    }

    uint32_t AsyncIOOpaqueEntryType::GetNoOfBytesTransferred() const noexcept
    {
        return reinterpret_cast<const OVERLAPPED_ENTRY*>( this )->dwNumberOfBytesTransferred;
    }

    TCompletionKey AsyncIOOpaqueEntryType::GetCompletionKey() noexcept
    {
        return reinterpret_cast<TCompletionKey>( reinterpret_cast<OVERLAPPED_ENTRY*>( this )->lpCompletionKey );
    }
    
    AsyncIOOpaqueType* AsyncIOOpaqueEntryType::GetOpaquePtr() noexcept
    {
        return reinterpret_cast<AsyncIOOpaqueType*>( reinterpret_cast<OVERLAPPED_ENTRY*>( this )->lpOverlapped );
    }
}

namespace SKL
{
    GUID GGuidAcceptEx = WSAID_ACCEPTEX;

    static LPFN_ACCEPTEX Win32_AcquireAcceptEx( TSocket InSocket ) noexcept
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
            SKLL_VER( "TCPAccepter::StartAcceptingAsync() Already accepting!" );
            return RSuccess;
        }

        if( false == IsValid() )
        {
            SKLL_ERR( "TCPAccepter::StartAcceptingAsync() Failed, invalid config!" );
            return RInvalidParamters;
        }   

        // close potential open socket
        CloseSocket();
    
        // create new socket
        const TSocket NewSocket { AllocateNewIPv4TCPSocket( true ) };
        if( 0 == NewSocket )
        {
            SKLL_ERR( "TCPAccepter::StartAcceptingAsync() Failed to create new tcp socket!" );
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
            SKLL_ERR( "TCPAccepter::StartAcceptingAsync() Failed enable async io on socket!" );
            closesocket( NewSocket );
            Socket.exchange( 0 );
            return RFail;
        }

        LPFN_ACCEPTEX AcceptExPtr { Win32_AcquireAcceptEx( NewSocket ) };
        if( nullptr == AcceptExPtr )
        {
            SKLL_ERR_FMT( "TCPAccepter::StartAcceptingAsync() Failed acquire AcceptEx on address[%08x] port[%hu] WSAErr:%d", Config.IpAddress, Config.Port, WSAGetLastError() );
            CloseSocket();
            return RFail;
        }

        // cache accept ex
        CustomHandle.exchange( reinterpret_cast<void*>( AcceptExPtr ) );

        // set is running 
        bIsRunning.exchange( true );

        if( false == BeginAcceptAsync() )
        {
            SKLL_ERR_FMT( "TCPAccepter::StartAcceptingAsync() Failed start AcceptEx on address[%08x] port[%hu] WSAErr:%d", Config.IpAddress, Config.Port, WSAGetLastError() );
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
                SKLL_VER( "TCPAccepter::BeginAcceptAsync() Failed to allocate task!" );
                return false;
            }
        }
        else
        {
            AcceptTask = reinterpret_cast<AsyncAcceptTask*>( InAcceptTask );
            
            // Increment the reference count for the reused task so it will not be destroyed
            TSharedPtr<AsyncAcceptTask>::Static_IncrementReference( AcceptTask );

            // guard
            SKL_ASSERT_EQUAL( 2U, TSharedPtr<AsyncAcceptTask>::Static_GetReferenceCount( AcceptTask ) );
        }

        // Allocate new tcp socket
        TSocket AcceptSocket { AllocateNewIPv4TCPSocket() };

        AcceptTask->SetCompletionHandler( [ this, AcceptSocket ]( IAsyncIOTask& Self, uint32_t /*NumberOfBytesTransferred*/ ) noexcept -> void
        {
            const TSocket ListenSocket{ GetSocket() };

            const auto UpdateResult { setsockopt( AcceptSocket
                                                , SOL_SOCKET
                                                , SO_UPDATE_ACCEPT_CONTEXT
                                                , reinterpret_cast<const char*>( &ListenSocket )
                                                , sizeof( TSocket ) ) };
            if( SOCKET_ERROR == UpdateResult  ) SKL_UNLIKELY
            {
                SKLL_VER_FMT( "TCPAccepter [AsyncIOCompletionHandler]:: Failed to accept WSAError:%d!", WSAGetLastError() );
                closesocket( AcceptSocket );
                StopAcceptingAsync();
                return;        
            }

            if ( RSuccess != AsyncIOAPI->AssociateToTheAPI( AcceptSocket ) ) SKL_UNLIKELY
            {
                SKLL_VER_FMT( "TCPAccepter [AsyncIOCompletionHandler]:: Failed to associate to the AsyncIO API WSAError:%d!", WSAGetLastError() );
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
                    SKLL_VER_FMT( "TCPAccepter [AsyncIOCompletionHandler]:: Failed to start to accept again WSAError:%d!", WSAGetLastError() );
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
                SKLL_ERR_FMT( "TCPAccepter::BeginAcceptAsync() Failed to AcceptEx WSAError:%d!", WSALastError );
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
            SKLL_VER( "TCPAccepter::StopAcceptingAsync() Already stopped!" );
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
            SKLL_ERR_FMT( "TCPAccepter::Bind() Failed to BIND on address[%08x] port[%hu] WSAErr:%d", Config.IpAddress, Config.Port, WSAGetLastError() );
            return false;
        }
        
        Result = listen( Socket.load_relaxed()
                       , static_cast<int32_t>( Config.Backlog ) );
        if( SOCKET_ERROR == Result )
        {
            SKLL_ERR_FMT( "TCPAccepter::Bind() Failed to LISTEN on address[%08x] port[%hu] WSAErr:%d", Config.IpAddress, Config.Port, WSAGetLastError() );
            return false;
        }

        return true;
    }
}

namespace SKL
{
    WSADATA GWSAData{}; //!< WSADATA global instance

    RStatus AsyncIO::InitializeSystem() noexcept 
    {
        if ( const int32_t WSAStartupResult = ::WSAStartup( MAKEWORD( 2, 2 ), &GWSAData ); WSAStartupResult )
        {
            SKLL_ERR_FMT( "AsyncIO::Initialize() Failed to WSAStartup() returned [%d] WSAERROR: %d", WSAStartupResult, ::WSAGetLastError() );
            return RFail;
        }

        return RSuccess;
    }

    RStatus AsyncIO::ShutdownSystem() noexcept 
    {
        if ( const int32_t WSACleanupResult = ::WSACleanup( ); WSACleanupResult )
        {
            SKLL_ERR_FMT( "Win32AsyncIO::SkylakeCore_Shutdown() Failed to WSAStartup() returned [%d] WSAERROR: %d", WSACleanupResult, ::WSAGetLastError() );
            return RFail;
        }

        return RSuccess;
    }

    RStatus AsyncIO::Start( int32_t InThreadsCount ) noexcept
    {
        ThreadsCount = InThreadsCount;
        
        // create the queue
        const auto Result = ::CreateIoCompletionPort( INVALID_HANDLE_VALUE, nullptr, NULL, static_cast<DWORD>( InThreadsCount ) );
        if( nullptr == Result ) SKL_UNLIKELY
        {
            const auto LastWSAError = ::WSAGetLastError();
            SKLL_ERR_FMT( "Win32AsyncIO::Start() Failed to create IOCP Handle WSAERROR[%d]", LastWSAError );
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

        SKL_ALLWAYS_LIKELY return RSuccess;
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
        if( FALSE == GetResult )
        {
            const auto LastWSAError = ::WSAGetLastError();
            if( WAIT_TIMEOUT == LastWSAError )
            {
                return RTimeout;
            }

            if( WSA_OPERATION_ABORTED == LastWSAError ) SKL_ALLWAYS_UNLIKELY
            {
                /*
                 * Overlapped operation aborted.
                 * This Windows error indicates that an overlapped I/O operation was canceled because of the closure of a socket.
                 * In addition, this error can occur when executing the SIO_FLUSH ioctl command.
                 */

                 SKL_ALLWAYS_UNLIKELY return RSuccessAsyncIORequestCancelled;
            }

            if( ERROR_NETNAME_DELETED == LastWSAError ) SKL_ALLWAYS_UNLIKELY
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

                SKL_ALLWAYS_UNLIKELY return RSuccessAsyncIORequestCancelled;
            }

            SKL_ALLWAYS_UNLIKELY return RSystemFailure;
        }

        SKL_ALLWAYS_LIKELY return RSuccess;
    }
    
    RStatus AsyncIO::GetMultipleCompletedAsyncRequest( AsyncIOOpaqueEntryType* OutputBuffer, uint32_t OutputBufferCount, uint32_t& OutCount ) noexcept
    {
        // sys call to get completed async IO requests or custom queued work
        const auto GetResult = ::GetQueuedCompletionStatusEx( 
                reinterpret_cast<HANDLE>( QueueHandle.load() ) 
              , reinterpret_cast<LPOVERLAPPED_ENTRY>( OutputBuffer )
              , static_cast<ULONG>( OutputBufferCount )
              , reinterpret_cast<PULONG>( &OutCount )
              , INFINITE
              , FALSE
        );
        if( FALSE == GetResult ) SKL_UNLIKELY
        {
            const auto LastWSAError = ::WSAGetLastError();
            
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

        SKL_ALLWAYS_LIKELY return RSuccess;
    }
    
    RStatus AsyncIO::TryGetMultipleCompletedAsyncRequest( AsyncIOOpaqueEntryType* OutputBuffer, uint32_t OutputBufferCount, uint32_t& OutCount, uint32_t InTimeout ) noexcept
    {
        // sys call to get completed async IO requests or custom queued work
        const auto GetResult = ::GetQueuedCompletionStatusEx( 
                reinterpret_cast<HANDLE>( QueueHandle.load() ) 
              , reinterpret_cast<LPOVERLAPPED_ENTRY>( OutputBuffer )
              , static_cast<ULONG>( OutputBufferCount )
              , reinterpret_cast<PULONG>( &OutCount )
              , static_cast<DWORD>( InTimeout )
              , FALSE
        );
        if( FALSE == GetResult )
        {
            const auto LastWSAError = ::WSAGetLastError();
            if( WAIT_TIMEOUT == LastWSAError )
            {
                return RTimeout;
            }

            if( WSA_OPERATION_ABORTED == LastWSAError ) SKL_ALLWAYS_UNLIKELY
            {
                /*
                 * Overlapped operation aborted.
                 * This Windows error indicates that an overlapped I/O operation was canceled because of the closure of a socket.
                 * In addition, this error can occur when executing the SIO_FLUSH ioctl command.
                 */

                 SKL_ALLWAYS_UNLIKELY return RSuccessAsyncIORequestCancelled;
            }

            if( ERROR_NETNAME_DELETED == LastWSAError ) SKL_ALLWAYS_UNLIKELY
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

                SKL_ALLWAYS_UNLIKELY return RSuccessAsyncIORequestCancelled;
            }

            SKL_ALLWAYS_UNLIKELY return RSystemFailure;
        }

        SKL_ALLWAYS_LIKELY return RSuccess;
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
        if( FALSE == Result ) SKL_ALLWAYS_UNLIKELY
        {
            const auto LastWSAError = ::WSAGetLastError();
            SKLL_ERR_FMT( "AsyncIO::QueueAsyncWork() failed with WSAERROR[%d]", LastWSAError );
            return RFail;
        }

        return RSuccess;
    }
    
    RStatus AsyncIO::ReceiveAsync( TSocket InSocket, IBuffer* InBuffer, TSharedPtr<AsyncIOOpaqueType> InOpaqueObject ) noexcept
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
          , reinterpret_cast<OVERLAPPED*>( InOpaqueObject.get() )
          , nullptr
        );
        if ( SOCKET_ERROR == Result ) SKL_ALLWAYS_UNLIKELY
        {    
            const int32_t LastWSAError{ WSAGetLastError() };
            if( WSA_IO_PENDING != LastWSAError ) SKL_UNLIKELY
            {
                SKLL_ERR_FMT( "AsyncIO::ReceiveAsync() failed with WSAERROR[%d]", LastWSAError );
                return RFail;
            }
        }

        // This reference must be released by the worker that calls GetCompletedAsyncRequest/TryGetCompletedAsyncRequest
        // on the same AsyncIO handle that the socket was associated with
        ( void )InOpaqueObject.ReleaseRawRef();

        return RSuccess;
    }
    
    RStatus AsyncIO::SendAsync( TSocket InSocket, IBuffer* InBuffer, TSharedPtr<AsyncIOOpaqueType> InOpaqueObject ) noexcept
    {
        DWORD NumberOfBytesReceived { 0 };

        // sys call to start the receive async IO request
        const auto Result = ::WSASend(
            static_cast<SOCKET>( InSocket )
          , reinterpret_cast<LPWSABUF>( InBuffer )
          , 1
          , &NumberOfBytesReceived
          , 0
          , reinterpret_cast<OVERLAPPED*>( InOpaqueObject.get() )
          , nullptr
        );
        if ( SOCKET_ERROR == Result ) SKL_ALLWAYS_UNLIKELY
        {    
            const int32_t LastWSAError{ WSAGetLastError() };
            if( WSA_IO_PENDING != LastWSAError ) SKL_UNLIKELY
            {
                SKLL_ERR_FMT( "AsyncIO::SendAsync() failed with WSAERROR[%d]", LastWSAError );
                return RFail;
            }
        }

        // This reference must be released by the worker that calls GetCompletedAsyncRequest/TryGetCompletedAsyncRequest
        // on the same AsyncIO handle that the socket was associated with
        ( void )InOpaqueObject.ReleaseRawRef();

        return RSuccess;
    }

    RStatus AsyncIO::SendAsync( TSocket InSocket, TSharedPtr<IAsyncIOTask> InAsyncIOTask ) noexcept
    {
        IBuffer* BufferInterface{ &InAsyncIOTask->GetInterface() };
        return SendAsync( InSocket, BufferInterface, InAsyncIOTask.ReinterpretCastMoveTo<AsyncIOOpaqueType>() );
    }

    RStatus AsyncIO::ReceiveAsync( TSocket InSocket, TSharedPtr<IAsyncIOTask> InAsyncIOTask ) noexcept
    {
        IBuffer* BufferInterface{ &InAsyncIOTask->GetInterface() };
        return ReceiveAsync( InSocket, BufferInterface, InAsyncIOTask.ReinterpretCastMoveTo<AsyncIOOpaqueType>() );
    }

    RStatus AsyncIO::AssociateToTheAPI( TSocket InSocket ) const noexcept
    {
        const auto Result = ::CreateIoCompletionPort( reinterpret_cast<HANDLE>( InSocket )
                                                    , reinterpret_cast<HANDLE>( QueueHandle.load_relaxed() )
                                                    , 0
                                                    , 0 );
        if( nullptr == Result ) SKL_ALLWAYS_UNLIKELY
        {
            const auto LastWSAError = ::WSAGetLastError();
            SKLL_ERR_FMT( "Win32AsyncIO::AssociateToTheAPI() Failed to associate socket to the IOCP Handle WSAERROR[%d]", LastWSAError );
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
        return ::GetTickCount64();
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
            Timer->Mean += Delta / static_cast<double>( Timer->Count );
            Timer->M2   += Delta * ( Error - Timer->Mean );
            double Stddev = sqrt( Timer->M2 / static_cast<double>( Timer->Count - 1 ) );
            Timer->Estimate = Timer->Mean + Stddev;
        }

        // Spin lock
        const auto Start{ high_resolution_clock::now() };
        while ( static_cast<double>( ( high_resolution_clock::now() - Start ).count() ) / 1e9 < InSeconds );
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

    int32_t GGetLastError() noexcept
    {
        return ( int32_t )GetLastError();
    }
    
    int32_t GGetNetworkLastError() noexcept
    {
        return ( int32_t )WSAGetLastError();
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

    std::vector<std::string> ScanForFilesInDirectory( const char* RootDirectory, size_t& OutMaxFileSize, const std::vector<std::string>& extensions ) noexcept
    {
        std::vector<std::string> result;
	    WIN32_FIND_DATAA ffd;
	    HANDLE hFind = INVALID_HANDLE_VALUE;
	    std::string cwd = RootDirectory;
	    std::string searchPattern = RootDirectory;
	    searchPattern += "*";
	    OutMaxFileSize = 0;

	    hFind = FindFirstFileA( searchPattern.c_str( ), &ffd );
	    if( INVALID_HANDLE_VALUE == hFind )
	    {
	    	return result;
	    }

	    do
	    {
	    	cwd = RootDirectory;
	    	cwd += ffd.cFileName;

	    	if( ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
	    	{
	    		if( 0 == strcmp( ffd.cFileName, "." ) || 0 == strcmp( ffd.cFileName, ".." ) )
	    		{
	    			continue;
	    		}

	    		cwd += "\\";

	    		auto temp = ScanForFilesInDirectory( cwd.c_str( ), OutMaxFileSize, extensions );

	    		for( auto& t : temp )
	    		{
	    			result.push_back( std::move( t ) );
	    		}
	    	}
	    	else
	    	{
	    		bool found = false;
	    		for( const auto& t : extensions )
	    		{
	    			if( cwd.find( t ) != std::string::npos )
	    			{
	    				found = true;
	    				break;
	    			}
	    		}
	    		if( !found )
	    		{
	    			continue;
	    		}

	    		size_t FileSize = static_cast<size_t>( ffd.nFileSizeHigh ) * (size_t)( MAXDWORD + 1 ) +
	    			(size_t)ffd.nFileSizeLow;
	    		//FileSize <<= sizeof(ffd.nFileSizeHigh) * 8; // Push by count of bits
	    		//FileSize |= ffd.nFileSizeLow;

	    		if( FileSize > OutMaxFileSize )
	    		{
	    			OutMaxFileSize = FileSize;
	    		}

	    		result.push_back( cwd );
	    	}
	    }
	    while( FindNextFileA( hFind, &ffd ) != 0 );

	    FindClose( hFind );

	    return result;
    }

    std::vector<std::wstring> ScanForFilesInDirectoryW( const wchar_t* RootDirectory, size_t& OutMaxFileSize, const std::vector<std::wstring>& extensions ) noexcept
    {
        std::vector<std::wstring> result;
	    WIN32_FIND_DATAW ffd;
	    HANDLE hFind = INVALID_HANDLE_VALUE;
	    std::wstring cwd = RootDirectory;
	    std::wstring searchPattern = RootDirectory;
	    searchPattern += L"*";
	    OutMaxFileSize = 0;

	    hFind = FindFirstFileW( searchPattern.c_str( ), &ffd );
	    if( INVALID_HANDLE_VALUE == hFind )
	    {
	    	return result;
	    }

	    do
	    {
	    	cwd = RootDirectory;
	    	cwd += ffd.cFileName;

	    	if( ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
	    	{
	    		if( 0 == wcscmp( ffd.cFileName, L"." ) || 0 == wcscmp( ffd.cFileName, L".." ) )
	    		{
	    			continue;
	    		}

	    		cwd += L"\\";

	    		auto temp = ScanForFilesInDirectoryW( cwd.c_str( ), OutMaxFileSize, extensions );

	    		for( auto& t : temp )
	    		{
	    			result.push_back( std::move( t ) );
	    		}
	    	}
	    	else
	    	{
	    		bool found = false;
	    		for( const auto& t : extensions )
	    		{
	    			if( cwd.find( t ) != std::wstring::npos )
	    			{
	    				found = true;
	    				break;
	    			}
	    		}
	    		if( !found )
	    		{
	    			continue;
	    		}

	    		size_t FileSize = static_cast<size_t>( ffd.nFileSizeHigh ) * (size_t)( MAXDWORD + 1 ) +
	    			(size_t)ffd.nFileSizeLow;
	    		//FileSize <<= sizeof(ffd.nFileSizeHigh) * 8; // Push by count of bits
	    		//FileSize |= ffd.nFileSizeLow;

	    		if( FileSize > OutMaxFileSize )
	    		{
	    			OutMaxFileSize = FileSize;
	    		}

	    		result.push_back( cwd );
	    	}
	    }
	    while( FindNextFileW( hFind, &ffd ) != 0 );

	    FindClose( hFind );

	    return result;
    }

    bool GetCurrentWorkingDirectory( char* OutBuffer, size_t BufferSize ) noexcept
    {
        return 0 != GetCurrentDirectory( static_cast<DWORD>( BufferSize ), OutBuffer );
    }

    void SetConsoleWindowTitleText( const char* InText ) noexcept
    {
        ( void )SetWindowTextA( GetConsoleWindow(), InText );
    }
}

// String Utils
namespace SKL
{
    const char* StringUtils::IpV4AddressToString( TIPv4Address InAddress ) noexcept
    {
        auto* Instance{ StringUtils::GetInstance() };
        SKL_ASSERT( nullptr != Instance );

        auto& Buffer{ Instance->WorkBenchBuffer };

        if ( nullptr != InetNtopA( AF_INET
            , &InAddress
            , reinterpret_cast<PSTR>( Buffer.GetBuffer() )
            , Buffer.GetBufferSize() ) )
        {
            return reinterpret_cast<const char*>( Buffer.GetBuffer() );
        }

        return "[Invalid IPv4Address]";
    }

    const wchar_t* StringUtils::IpV4AddressToWString( TIPv4Address InAddress ) noexcept
    {
        auto* Instance{ StringUtils::GetInstance() };
        SKL_ASSERT( nullptr != Instance );

        auto& Buffer{ Instance->WorkBenchBuffer };

        if ( nullptr != InetNtopW( AF_INET
            , &InAddress
            , reinterpret_cast<PWSTR>( Buffer.GetBuffer() )
            , Buffer.GetBufferSize() ) )
        {
            return reinterpret_cast<const wchar_t*>( Buffer.GetBuffer() );
        }

        return L"[Invalid IPv4Address]";
    }

    const char* StringUtils::ConvertUtf16ToUtf8( const wchar_t* InWString, size_t MaxCharCountInString ) noexcept
    {
        auto* Instance{ StringUtils::GetInstance() };
        SKL_ASSERT( nullptr != Instance );
        
        SKL::BufferStream& Buffer{ Instance->WorkBenchBuffer };

        if( false == GWideCharToMultiByte( InWString
                                         , MaxCharCountInString
                                         , reinterpret_cast<char*>( Buffer.GetBuffer() )
                                         , static_cast<int32_t>( Buffer.GetBufferSize() ) ) ) SKL_UNLIKELY
        {
            SKL_STRCPY( reinterpret_cast<char*>( Buffer.GetBuffer() ), "[U16-U8-CONVERSATION-FAILED]", 30 );
        }

        return reinterpret_cast<const char*>( Buffer.GetBuffer() );
    }
    
    const wchar_t* StringUtils::ConvertUtf8ToUtf16( const char* InString, size_t MaxCharCountInString ) noexcept
    {
        auto* Instance{ StringUtils::GetInstance() };
        SKL_ASSERT( nullptr != Instance );
        
        SKL::BufferStream& Buffer{ Instance->WorkBenchBuffer };

        if( false == GMultiByteToWideChar( InString
                                         , MaxCharCountInString
                                         , reinterpret_cast<wchar_t*>( Buffer.GetBuffer() )
                                         , static_cast<int32_t>( Buffer.GetBufferSize() / sizeof( wchar_t ) ) ) ) SKL_UNLIKELY
        {
            SKL_WSTRCPY( reinterpret_cast<wchar_t*>( Buffer.GetBuffer() ), L"[U8-U16-CONVERSATION-FAILED]", 30 );
        }

        return reinterpret_cast<const wchar_t*>( Buffer.GetBuffer() );
    }
}

namespace std
{
    static_assert( sizeof( PSRWLOCK ) == sizeof( void * ) );
    static_assert( sizeof( PSRWLOCK ) == sizeof( rw_lock ) );

    rw_lock::rw_lock() noexcept
        : LockHandle SRWLOCK_INIT { }

    // std::shared_mutex compatible API
    void rw_lock::lock() noexcept
    {
        ( void )AcquireSRWLockExclusive( reinterpret_cast<PSRWLOCK>( &LockHandle ) );
    }

    void rw_lock::unlock() noexcept
    {
        ReleaseSRWLockExclusive( reinterpret_cast<PSRWLOCK>( &LockHandle ) );
    }

    bool rw_lock::try_lock() noexcept
    {
        return static_cast<bool>( TryAcquireSRWLockExclusive( reinterpret_cast<PSRWLOCK>( &LockHandle ) ) );
    }

    bool rw_lock::try_lock_shared() noexcept
    {
        return static_cast<bool>( TryAcquireSRWLockShared( reinterpret_cast<PSRWLOCK>( &LockHandle ) ) );
    }

    void rw_lock::lock_shared() noexcept
    {
        AcquireSRWLockShared( reinterpret_cast<PSRWLOCK>( &LockHandle ) );
    }

    void rw_lock::unlock_shared() noexcept
    {
        ReleaseSRWLockShared( reinterpret_cast<PSRWLOCK>( &LockHandle ) );
    }
}
