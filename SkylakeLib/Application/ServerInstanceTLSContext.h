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

        SKL_FORCEINLINE const char *GetName( ) const noexcept { return SourceServerInstance ? NameBuffer : "[UNINITIALIZED ServerInstanceTLSData]"; }

        void Reset() noexcept;
        void Clear() noexcept;

        SKL_FORCEINLINE ServerInstance* GetServerInstance() const noexcept { return SourceServerInstance; }
        SKL_FORCEINLINE ServerInstanceFlags GetServerInstanceFlags() const noexcept { return ServerFlags; }
        SKL_FORCEINLINE WorkerGroupTag GetCurrentWorkerGroupTag() const noexcept { return ParentWorkerGroup; }
        SKL_FORCEINLINE const std::vector<WorkerGroup*>& GetDeferredTasksHandlingGroups() const noexcept { return DeferredTasksHandlingGroups; }

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