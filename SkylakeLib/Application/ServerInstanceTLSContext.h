//!
//! \file ServerInstanceTLSContext.h
//! 
//! \brief Thread local context for all workers in a server instance
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    struct ServerInstanceTLSData final : ITLSSingleton<ServerInstanceTLSData>
    {
        ServerInstanceTLSData() noexcept = default;
        ~ServerInstanceTLSData() noexcept = default;

        RStatus Initialize( ServerInstance* InServerInstance ) noexcept;

        const char *GetName( ) const noexcept { return SourceServerInstance ? NameBuffer : "[UNINITIALIZED ServerInstanceTLSData]"; }

        void Reset() noexcept;

        ServerInstance*                           SourceServerInstance       { nullptr };
        ServerInstanceFlags                       ServerFlags                {};
        std::vector<std::shared_ptr<WorkerGroup>> DeferredTasksHandlingGroups{};
        char                                      NameBuffer[512]            { 0 };
    };
}