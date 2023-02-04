//!
//! \file TLSService.h
//! 
//! \brief TLS Active and AOD interfaced service
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    class WorkerService : public AODService
    {
    public:
        WorkerService( uint32_t UID ) noexcept : AODService{ UID } {}

    protected: 
        //! [Callback] Each time a worker started
        virtual RStatus OnWorkerStarted( Worker& InWorker, WorkerGroup& InWorkerGroup ) noexcept = 0;

        //! [Callback] Each time a worker stopped
        virtual void OnWorkerStopped( Worker& InWorker, WorkerGroup& InWorkerGroup ) noexcept = 0;

        //! [Callback] Tick for each active worker
        virtual void OnTickWorker( Worker& InWorker, WorkerGroup& InWorkerGroup ) noexcept = 0;

        friend ServerInstance;

        template<bool bIsActive, bool bHandlesTasks, bool bSupportsAOD, bool bHandlesTimerTasks, bool bSupportsTLSSync, bool bHasTickHandler, bool bAllWorkerGroupsAreActive, bool bTickWorkerServices>
        friend struct WorkerGroupRunVariant;
    };
}    