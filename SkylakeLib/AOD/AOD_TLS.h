//!
//! \file AOD_TLS.h
//! 
//! \brief Async Object bound Dispatcher thread local state 
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    struct AODTLSData final : ITLSSingleton<AODTLSData>
    {
        AODTLSData() noexcept = default;
        ~AODTLSData() noexcept = default;

        RStatus Initialize( ServerInstance* InServerInstance ) noexcept;

        const char *GetName( ) const noexcept { return SourceServerInstance ? NameBuffer : "[UNINITIALIZED AODTLSData]"; }

        void Reset() noexcept;

        ServerInstance*                           SourceServerInstance       { nullptr };
        ServerInstanceFlags                       ServerFlags                {};
        std::vector<std::shared_ptr<WorkerGroup>> DeferredTasksHandlingGroups{};
        char                                      NameBuffer[512]            { 0 };
    };
}