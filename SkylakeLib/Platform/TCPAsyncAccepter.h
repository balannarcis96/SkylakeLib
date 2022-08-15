//!
//! \file TCPAsyncAccepter.h
//! 
//! \brief Tcp async acceptor abstraction
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    struct TCPAcceptorConfig
    {
        using AcceptTask = ASD::CopyFunctorWrapper<32, void( SKL_CDECL* )( TSocket ) noexcept>;

        //! Set the functor to be executed when a new tcp connection is successfully accepted
        template<typename TFunctor>
        void SetOnAcceptHandler( const TFunctor& InFunctor ) noexcept
        {
            OnAccept += InFunctor;
        }

        //! Set the functor to be executed when a new tcp connection is successfully accepted
        void SetOnAcceptHandler( const AcceptTask& InFunctor ) noexcept
        {
            OnAccept = InFunctor;
        }

        //! Is valid config
        SKL_FORCEINLINE bool IsValid() const noexcept { return false == OnAccept.IsNull() && 0 != Id && 0 != IpAddress && 0 != Port; }

        uint32_t   Id           { 0 };       //!< UID
        uint32_t   IpAddress    { 0 };       //!< The address to listen for connections on
        uint16_t   Port         { 0 };       //!< The port to listen for connections on
        uint16_t   Backlog      { 0 };       //!< Number of pending connections to keep (queue size)
        AcceptTask OnAccept     {};          //!< Task dispatched when a new tcp connection is successfully accepted
    };

    struct TCPAcceptor
    {
        TCPAcceptor( const TCPAcceptorConfig& Config, AsyncIO* AsyncIOAPI ) noexcept : Config{ Config }, AsyncIOAPI{ AsyncIOAPI } {}
        ~TCPAcceptor() noexcept = default;

        //! Get the config
        SKL_FORCEINLINE const TCPAcceptorConfig& GetConfig() const noexcept { return Config; }

        //! Get the socket
        SKL_FORCEINLINE TSocket GetSocket() const noexcept { return Socket.load(); }

        //! Is valid acceptor, ready to use
        SKL_FORCEINLINE bool IsValid() const noexcept { return true == Config.IsValid(); }
    
        //! Start accepting tcp connections async
        RStatus StartAcceptingAsync() noexcept;

        //! Stop accepting tcp connections
        void StopAcceptingAsync() noexcept;

        //! Is the acceptor accepting connections
        bool IsAccepting() const noexcept { return TRUE == bIsRunning.load_relaxed(); }
    
    private:
        void CloseSocket() noexcept;

        bool BindAndListen() noexcept;

        bool BeginAcceptAsync() noexcept;

        std::relaxed_value<TSocket>   Socket       { 0 };       //!< Socket to listen on
        std::relaxed_value<void*>     CustomHandle { nullptr }; //!< 8 bytes for custom use by specific OS implementation
        std::relaxed_value<uint32_t>  bIsRunning   { FALSE };   //!< Is the acceptor accepting connections
        AsyncIO*                      AsyncIOAPI   { nullptr }; //!< AsyncIO API to use for the async IO accept requests
        TCPAcceptorConfig             Config       {};          //!< Config
    };
}