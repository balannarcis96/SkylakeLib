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
        using PriorityTasksQueue = TLSManagedPriorityQueue<ITask*, ITaskComparer>;

        ServerInstanceTLSContext( ServerInstance* InServerInstance, WorkerGroupTag InWorkerGroupTag ) noexcept;
        ~ServerInstanceTLSContext() noexcept;

        RStatus Initialize() noexcept;

        SKL_FORCEINLINE SKL_NODISCARD const char *GetName( ) const noexcept { return SourceServerInstance ? NameBuffer : "[UNINITIALIZED ServerInstanceTLSData]"; }

        void Reset() noexcept;
        void Clear() noexcept;

        SKL_FORCEINLINE SKL_NODISCARD ServerInstance* GetServerInstance() const noexcept { return SourceServerInstance; }
        SKL_FORCEINLINE SKL_NODISCARD ServerInstanceFlags GetServerInstanceFlags() const noexcept { return ServerFlags; }
        SKL_FORCEINLINE SKL_NODISCARD WorkerGroupTag GetCurrentWorkerGroupTag() const noexcept { return ParentWorkerGroup; }
        SKL_FORCEINLINE SKL_NODISCARD const std::vector<WorkerGroup*>& GetDeferredTasksHandlingGroups() const noexcept { return DeferredTasksHandlingGroups; }
        SKL_FORCEINLINE SKL_NODISCARD const size_t GetPendingDelayedTasksCount() const noexcept { return PendingDelayedTasks.size(); }
        SKL_FORCEINLINE SKL_NODISCARD const size_t GetDelayedTasksCount() const noexcept { return DelayedTasks.size(); }

    private:
        TLSManagedQueue<ITask*>   PendingDelayedTasks        {};
        PriorityTasksQueue        DelayedTasks               {};
        WorkerGroupTag            ParentWorkerGroup          {};
        ServerInstanceFlags       ServerFlags                {};
        std::vector<WorkerGroup*> DeferredTasksHandlingGroups{};
        uint16_t                  RRLastIndex                { 0 };
        uint16_t                  RRLastIndex2               { 0 };
        uint32_t                  Padding                    { 0 };
        ServerInstance*           SourceServerInstance       { nullptr };
        char                      NameBuffer[512]            { 0 };

        friend void DeferTask( ITask* InTask ) noexcept;    
        friend void DeferTaskAgain( ITask* InTask ) noexcept;
        friend void DeferTaskAgain( TDuration AfterMilliseconds, ITask* InTask ) noexcept;
        friend void ScheduleTask( ServerInstanceTLSContext& TLSContext, ITask* InTask ) noexcept;

        friend class WorkerGroup;
    };
}