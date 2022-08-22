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
    struct AODTLSContext final : ITLSSingleton<AODTLSContext>
    {
        union ThreadFlags
        {
            struct
            {
                uint8_t bIsInitialized : 1;
                uint8_t bIsAnyDispatchInProgress : 1;
            };
            uint16_t Flags = { 0 };
        };

        AODTLSContext() noexcept = default;
        ~AODTLSContext() noexcept;

        RStatus Initialize( ServerInstance* InServerInstance, WorkerGroupTag InWorkerGroupTag ) noexcept;

        SKL_FORCEINLINE const char *GetName( ) const noexcept { return SourceServerInstance ? NameBuffer : "[UNINITIALIZED AODTLSContext]"; }

        void Reset() noexcept;
        void Clear() noexcept;

        SKL_FORCEINLINE ServerInstance* GetServerInstance() const noexcept { return SourceServerInstance; }
        SKL_FORCEINLINE ServerInstanceFlags GetServerInstanceFlags() const noexcept { return ServerFlags; }
        SKL_FORCEINLINE WorkerGroupTag GetWorkerGroupTag() const noexcept { return ParentWorkerGroup; }
        SKL_FORCEINLINE std::span<std::shared_ptr<WorkerGroup>> GetDeferredAODTasksHandlingGroups() noexcept { return { DeferredAODTasksHandlingGroups }; }

    public:
        TLSManagedPriorityQueue<IAODTask*> DelayedTasks             {};
        TLSManagedQueue<AODObject*>        PendingAODObjects        {};
        TEpochTimePoint                    CurrentLoopBeginTick     { 0 };
        TEpochTimePoint                    TickCount                { 0 };
        TEpochTimePoint                    BeginTick                { 0 };
        uint16_t                           bScheduleAODDelayedTasks { FALSE };
        ThreadFlags                        Flags                    { 0 };
        uint16_t                           RRLastIndex              { 0 };
        uint16_t                           RRLastIndex2             { 0 };

    private:
        ServerInstance*                               SourceServerInstance          { nullptr };
        ServerInstanceFlags                           ServerFlags                   {};
        WorkerGroupTag                                ParentWorkerGroup             {};
        std::vector<std::shared_ptr<WorkerGroup>>     DeferredAODTasksHandlingGroups{};
        char                                          NameBuffer[512]               { 0 };
    };
}