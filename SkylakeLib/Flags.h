//!
//! \file Flags.h
//! 
//! \brief Compilation flags
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

// Enable dequeuing multiple async IO requests using a single system call
// as opposed of dequeuing one async IO request per system call
#ifndef SKLL_ASYNCWORKER_DEQUEUEMULTIPLEASYNCWORKPERSYSTEMCALL
#define SKLL_ASYNCWORKER_DEQUEUEMULTIPLEASYNCWORKPERSYSTEMCALL true
#endif

// Assume that the worker groups count is a power of two
#ifndef SKLL_TASKSCHEDULING_ASSUMETHATTASKHANDLINGWORKERGROUPCOUNTISPOWEROFTWO
#define SKLL_TASKSCHEDULING_ASSUMETHATTASKHANDLINGWORKERGROUPCOUNTISPOWEROFTWO false
#endif

// Assume that the workers count from the worker groups that handle timer tasks or AOD is a power of two
#ifndef SKLL_TASKSCHEDULING_ASSUMETHATWORKERSCOUNTISPOWEROFTWO
#define SKLL_TASKSCHEDULING_ASSUMETHATWORKERSCOUNTISPOWEROFTWO false
#endif

// Assume that all worker groups handle timer tasks
#ifndef SKLL_TASKSCHEDULING_ASSUMEALLWORKERGROUPSHANDLETIMERTASKS
#define SKLL_TASKSCHEDULING_ASSUMEALLWORKERGROUPSHANDLETIMERTASKS false
#endif

// Assume that all worker groups handle AOD(Advanced Object-Bound Dispatcher)
#ifndef SKLL_TASKSCHEDULING_ASSUMEALLWORKERGROUPSHANDLEAOD
#define SKLL_TASKSCHEDULING_ASSUMEALLWORKERGROUPSHANDLEAOD false
#endif

// Assume that all worker groups have TLS memory management enabled
#ifndef SKLL_TASKSCHEDULING_ASSUMEALLWORKERGROUPSHAVETLSMEMORYMANAGEMENT
#define SKLL_TASKSCHEDULING_ASSUMEALLWORKERGROUPSHAVETLSMEMORYMANAGEMENT false
#endif

// Should the tasks scheduling use `if` instead of `modulo` in a critical part of the logic
#ifndef SKLL_TASKSCHEDULING_USEIFINSTEADOFMODULO
#define SKLL_TASKSCHEDULING_USEIFINSTEADOFMODULO false
#endif

// Enable tasks execution throttling, worker will execute a max no of tasks per tick 
// as opposed all tasks that are available are executed per tick
#ifndef SKLL_TASK_DOTHROTTLEGENERALTASKEXECUTION
#define SKLL_TASK_DOTHROTTLEGENERALTASKEXECUTION true
#endif

namespace SKL
{
    /*------------------------------------------------------------
        Feature flags
      ------------------------------------------------------------*/
    constexpr bool CAsyncWorker_DequeueMultipleAsyncWorkPerSystemCall                 = SKLL_ASYNCWORKER_DEQUEUEMULTIPLEASYNCWORKPERSYSTEMCALL;
    constexpr bool CTaskScheduling_AssumeThatTaskHandlingWorkerGroupCountIsPowerOfTwo = SKLL_TASKSCHEDULING_ASSUMETHATTASKHANDLINGWORKERGROUPCOUNTISPOWEROFTWO;
    constexpr bool CTaskScheduling_AssumeThatWorkersCountIsPowerOfTwo                 = SKLL_TASKSCHEDULING_ASSUMETHATWORKERSCOUNTISPOWEROFTWO;
    constexpr bool CTaskScheduling_AssumeAllWorkerGroupsHandleTimerTasks              = SKLL_TASKSCHEDULING_ASSUMEALLWORKERGROUPSHANDLETIMERTASKS;
    constexpr bool CTaskScheduling_AssumeAllWorkerGroupsHandleAOD                     = SKLL_TASKSCHEDULING_ASSUMEALLWORKERGROUPSHANDLEAOD;
    constexpr bool CTaskScheduling_AssumeAllWorkerGroupsHaveTLSMemoryManagement       = SKLL_TASKSCHEDULING_ASSUMEALLWORKERGROUPSHAVETLSMEMORYMANAGEMENT;
    constexpr bool CTaskScheduling_UseIfInsteadOfModulo                               = SKLL_TASKSCHEDULING_USEIFINSTEADOFMODULO;
    constexpr bool CTask_DoThrottleGeneralTaskExecution                               = SKLL_TASK_DOTHROTTLEGENERALTASKEXECUTION;
}
