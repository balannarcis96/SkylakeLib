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

        //! [Callback] When server is signaled to shutdown
        virtual void OnServerStopSignaled() noexcept = 0;

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
