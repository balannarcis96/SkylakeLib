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
    struct ServerInstanceTLSContext final : ITLSSingleton<ServerInstanceTLSContext>
    {
        ServerInstanceTLSContext() noexcept = default;
        ~ServerInstanceTLSContext() noexcept = default;

        RStatus Initialize( ServerInstance* InServerInstance, WorkerGroupTag InWorkerGroupTag ) noexcept;

        SKL_FORCEINLINE const char *GetName( ) const noexcept { return SourceServerInstance ? NameBuffer : "[UNINITIALIZED ServerInstanceTLSData]"; }

        void Reset() noexcept;

        SKL_FORCEINLINE ServerInstance* GetServerInstance() const noexcept { return SourceServerInstance; }
        SKL_FORCEINLINE ServerInstanceFlags GetServerInstanceFlags() const noexcept { return ServerFlags; }
        SKL_FORCEINLINE WorkerGroupTag GetCurrentWorkerGroupTag() const noexcept { return ParentWorkerGroup; }
        SKL_FORCEINLINE const std::vector<std::shared_ptr<WorkerGroup>>& GetDeferredTasksHandlingGroups() const noexcept { return DeferredTasksHandlingGroups; }

    private:
        TLSManagedPriorityQueue<ITask*>            DelayedTasks               {};
        WorkerGroupTag                             ParentWorkerGroup          {};
        ServerInstanceFlags                        ServerFlags                {};
        std::vector<std::shared_ptr<WorkerGroup>>  DeferredTasksHandlingGroups{};
        uint16_t                                   RRLastIndex                { 0 };
        uint16_t                                   RRLastIndex2               { 0 };
        uint32_t                                   Padding                    { 0 };
        ServerInstance*                            SourceServerInstance       { nullptr };
        char                                       NameBuffer[512]            { 0 };

        friend bool DeferTask( ITask* InTask ) noexcept;    
        friend bool ScheduleTask( ServerInstanceTLSContext& TLSContext, ITask* InTask ) noexcept;
    };
}