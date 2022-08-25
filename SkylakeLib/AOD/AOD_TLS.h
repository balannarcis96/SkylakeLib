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
                uint8_t bIsAnyStaticDispatchInProgress : 1;
                uint8_t bIsAnySharedDispatchInProgress : 1;
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
        TLSManagedPriorityQueue<IAODSharedObjectTask*> DelayedSharedObjectTasks      {};          //!< Priority queue of AOD Shared Object delayed tasks
        TLSManagedPriorityQueue<IAODStaticObjectTask*> DelayedStaticObjectTasks      {};          //!< Priority queue of AOD Static Object delayed tasks
        TLSManagedQueue<AOD::SharedObject*>            PendingAOD_SharedObjects      {};          //!< Queue of pending AOD Shared Objects to be dispatched by the consumer
        TLSManagedQueue<AOD::StaticObject*>            PendingAOD_StaticObjects      {};          //!< Queue of pending AOD Static Objects to be dispatched by the consumer
        uint16_t                                       bScheduleAODDelayedTasks      { FALSE };   //!< Should attempt to scheduler tasks to other workers 
        ThreadFlags                                    Flags                         { 0 };       //!< Flags
        uint16_t                                       RRLastIndex                   { 0 };       //!< Round-Robin index   [for scheduling tasks to other workers]
        uint16_t                                       RRLastIndex2                  { 0 };       //!< Round-Robin index 2 [for scheduling tasks to other workers]
        ServerInstance*                                SourceServerInstance          { nullptr }; //!< ServerInstance cached pointer
        ServerInstanceFlags                            ServerFlags                   {};          //!< ServerInstanceFlags cached
        WorkerGroupTag                                 ParentWorkerGroup             {};          //!< Cached tag of this thread's parent worker group
        std::vector<std::shared_ptr<WorkerGroup>>      DeferredAODTasksHandlingGroups{};          //!< Cached list of working groups that can handle deferred AOD tasks
        char                                           NameBuffer[512]               { 0 };       //!< Name buffer
    };
}