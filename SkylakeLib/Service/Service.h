//!
//! \file Service.h
//! 
//! \brief Server instance service abstraction
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    class SimpleService;
    class AODService;
    class ActiveService;
    class WorkerService;
}    

namespace SKL
{
    class IService
    {
    public:
        IService( uint32_t UID ) noexcept : UID{ UID } { SKL_ASSERT( 0 != UID ); }
        virtual ~IService() noexcept = default;

        //! Get the owning server instance 
        ServerInstance& GetServerInstance() const noexcept 
        { 
            SKL_ASSERT( nullptr != MyServerInstance );
            return *MyServerInstance; 
        }

        //! Get the Unique Identifier
        uint32_t GetUID() const noexcept{ return UID; }

    protected:
        //! Initialize the service
        virtual RStatus Initialize() noexcept = 0;        

        //! [Callback] When server started
        virtual void OnServerStarted() noexcept = 0;

        //! [Callback] When server shutdown
        virtual void OnServerStopped() noexcept = 0;

        //! [Callback] When service is signaled to shutdown
        virtual RStatus OnStopService() noexcept = 0;

        //! [Callback] When service is finally shutdown
        void OnServiceStopped( RStatus InStatus ) noexcept;

    private:
        //! [Callback] When server is signaled to shutdown
        void OnServerStopSignaled() noexcept;

    private:
        uint32_t        UID{ 0 };
        ServerInstance* MyServerInstance{ nullptr };

        friend ServerInstance;
    };  
}  

#include "SimpleService.h"
#include "AODService.h"
#include "ActiveService.h"
#include "WorkerService.h"

namespace SKL
{
    template<typename TService, typename ...TArgs>
    inline TService* CreateService( TArgs... Args ) noexcept
    {
        auto* Block{ SKL_MALLOC_ALIGNED( sizeof( TService ), SKL_CACHE_LINE_SIZE ) };

        if( nullptr != Block ) SKL_LIKELY
        {
            GConstructNothrow<TService>( Block, std::forward<TArgs>( Args )... );
        }
        
        return reinterpret_cast<TService*>( Block );
    }

    template<typename TService>
    SKL_FORCEINLINE void DeleteService( TService* InService ) noexcept
    {
        SKL_ASSERT( nullptr != InService && 0 == ( reinterpret_cast<uint64_t>( InService ) % SKL_CACHE_LINE_SIZE ) );
        SKL_FREE_ALIGNED( InService, SKL_CACHE_LINE_SIZE );
    }

    template<typename TService>
    class ServiceDeleter
    {
    public:
        SKL_FORCEINLINE void operator()( TService* InService ) const noexcept
        {
            DeleteService( InService );
        }
    };

    template<typename TService>
    using TServicePtr = std::unique_ptr<TService, ServiceDeleter<TService>>;
}