//!
//! \file SkylakeLib.h
//! 
//! \brief Interface for the SkylakeLib
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

//! The header only part
#include <SkylakeLibHeaderOnly.h>

//! Std
#include "Std/Std.h"

//! Tuning
#include "Tuning/Tuning.h"

//! Logging
#include "Diagnostics/Log.h"

extern void _mi_assert_fail(const char* assertion, const char* fname, unsigned line, const char* func );
#define mi_assert(expr)     ((expr) ? (void)0 : _mi_assert_fail(#expr,__FILE__,__LINE__,__func__))

//! Forward declarations
namespace SKL
{
    struct ITask;
    struct IAsyncIOTask;
    struct IAODTask;
    struct AODObject;
    class WorkerGroup;
    class Worker;
    class ServerInstance;

    union ServerInstanceFlags
    {
        struct
        {
            uint8_t bAllGroupsAreActive: 1;
            uint8_t bSupportsDelayedTasks: 1;
        };   
        uint32_t Flags{ 0 };
    };

    struct WorkerGroupTag final
    {
        uint32_t       TickRate                        { 0 };       //!< Tick rate of worker [ bIsActive == true ]
        uint32_t       SyncTLSTickRate                 { 0 };       //!< Tick rate of tls sync [ bSupportsTLSSync == true ]
        uint16_t       Id                              { 0 };       //!< UID of the tag [max 65536 workers] recommended to treat id as index starting from 1 (0 = invalid id)
        uint16_t       WorkersCount                    { 0 };       //!< Number of workers in the group
        bool           bIsActive                       { false };   //!< Is this an pro-active worker [ it has an active ticks/second loop ]
        bool           bHandlesTasks                   { false };   //!< Does this group handle tasks and async IO tasks [ an AsyncIO instance will be created for the group if true ]
        bool           bSupportsAOD                    { false };   //!< true -> Workers in this group can use AOD (Async Object Dispatcher) delayed tasks directly (handled by the same thread)
        bool           bHandlesTimerTasks              { false };   //!< true -> This group handles global and AOD(if bSupportsAOD=true) delayed tasks -> requires [bIsActive=true]
        bool           bSupportsTLSSync                { false };   //!< Supports TLSSync [ TLSSync it is its own feature, please get documented before use ]
        bool           bHasThreadLocalMemoryManager    { false };   //!< true -> If any of the workers in this group need to use ThreadLocalMemoryManager or associated allocation strategies
        bool           bPreallocateAllThreadLocalPools { false };   //!< true -> Preallocate all pools in ThreadLocalMemoryManager
        bool           bSupportesTCPAsyncAcceptors     { false };   //!< Does this group supports and handles tcp async acceptors
        bool           bCallTickHandler                { false };   //!< true -> the workers in the group will call the thick handler
        const wchar_t *Name                            { nullptr }; //!< Name of the worker group
        mutable bool   bIsValid                        { false };   //!< Initialize this member to false if you want your server to run correctly ;)

        //! [*]
        //!  Case 1.If all worker groups in the server instance are active [bIsActive=true], delayed tasks produced on any thread will be proceseed by the thread that produced it
        //!         Why to consider Case 1:
        //!           -  All tasks will be allocated through the thread local allocator -> very fast allocation/deallocation
        //!           -  No contention between threads (no load balancing)
        //!           -  Better time precision on delayed tasks
        //!           -> Requires bHasThreadLocalMemoryManager == true
        //!
        //!  Case 2.If not all worker groups in the server are active and you need the possibility to delay tasks in non-active worker groups then 
        //!    all worker groups marked with [bHandlesTimerTasks=true] will be used to check and dispatch delayed tasks (using rr-load balancing between worker groups and between workers)
        //!          Why to consider Case 2:
        //!              - The server can have inactive worker groups that can delay tasks
        //!
        //! Important: If no active worker group is present, the delayed tasks feature must not be used. Add at least one active worker group if you need to delay tasks.
        //!
        //! Note: Delayed tasks include free delayed tasks and AOD delayed tasks
        //!
        //! Note: [bSupportesTCPAsyncAcceptors=true] Accepted sockets will not be associated to any async IO api.
        //!

        SKL_FORCEINLINE bool Validate() const noexcept
        {
            SKL_ASSERT_ALLWAYS( nullptr != Name );

            if ( 0 == Id )
            {
                SKL_ERR_FMT( "WorkerGroupTag[%ws] Invalid Id %u!", Name, Id );
                return false;
            }

            // tick rate 0 means no delay
            //if ( true == bIsActive && 0 == TickRate )
            //{
            //    SKL_ERR_FMT( "WorkerGroupTag[%ws] Invalid TickRate %u!", Name, TickRate );
            //    return false;
            //}

            if ( false == bIsActive && false == bHandlesTasks )
            {
                SKL_ERR_FMT( "WorkerGroupTag[%ws] All inactive worker groups must marked [bIsActive=false;bHandlesTasks=true]!", Name );
                return false;
            }

            if ( true == bIsActive && true == bHandlesTasks && false == bCallTickHandler )
            {
                SKL_WRN_FMT( "WorkerGroupTag[%ws] For [bIsActive=true;bHandlesTasks=true;bCallTickHandler=false] Recommended to use a reactive worker group instead!", Name );
            }

            // tick rate 0 means no delay or relay on TickRate
            //if ( true == bSupportsTLSSync && 0 == SyncTLSTickRate )
            //{
            //    SKL_ERR_FMT( "WorkerGroupTag[%ws] Invalid SyncTLSTickRate %u!", Name, SyncTLSTickRate );
            //    return false;
            //}

            if( true == bPreallocateAllThreadLocalPools && false == bHasThreadLocalMemoryManager )
            {
                SKL_ERR_FMT( "WorkerGroupTag[%ws] [bPreallocateAllThreadLocalPools == true] requires -> bHasThreadLocalMemoryManager = true!", Name );
                return false;
            }

            if( true == bSupportsAOD && ( false == bHasThreadLocalMemoryManager ) )
            {
                SKL_ERR_FMT( "WorkerGroupTag[%ws] [bSupportsAOD == true] requires -> bHasThreadLocalMemoryManager = true!", Name );
                return false;
            }

            if( true == bSupportesTCPAsyncAcceptors && false == bHandlesTasks )
            {
                SKL_ERR_FMT( "WorkerGroupTag[%ws] [bSupportesTCPAsyncAcceptors == true] requires -> bHandlesTasks = true!", Name );
                return false;
            }

            if( true == bHandlesTimerTasks && false == bIsActive )
            {
                SKL_ERR_FMT( "WorkerGroupTag[%ws] [bHandlesTimerTasks == true] requires -> bIsActive = true!", Name );
                return false;
            }

            bIsValid = true;
            return true;
        }        
        SKL_FORCEINLINE bool IsValid() const noexcept { return bIsValid; }
    }; 
}

//! Platform
#include "Platform/Platform.h"

//! Thread Local Storage
#include "ThreadLocalStorage/TLS.h"

//! Spin Lock
#include "Utils/SpinLock.h"

//! Memory abstractions
#include "Memory/Memory.h"

//! Task
#include "Task/Task.h"
#include "Task/TaskQueue.h"

//! AOD [Task, Queue, TLSContext]
#include "AOD/AOD_Task.h"
#include "AOD/AOD_Queue.h"
#include "AOD/AOD_TLS.h"

//! Threading
#include "Threading/Threading.h"

//! Networking
#include "Networking/Networking.h"

//! AOD
#include "AOD/AOD.h"

//! Service
#include "Service\Service.h"

//! Application
#include "Application/Application.h"

namespace SKL
{
    //! Initialize the SkylakeLibrary
    RStatus Skylake_InitializeLibrary( int32_t Argc, char** Argv, FILE* InLogOutput ) noexcept;

    //! Terminate the SkylakeLibrary
    RStatus Skylake_TerminateLibrary() noexcept;

    //! Initialize the SkylakeLibrary per thread
    RStatus Skylake_InitializeLibrary_Thread() noexcept;

    //! Terminate the SkylakeLibrary per thread
    RStatus Skylake_TerminateLibrary_Thread() noexcept;

    //! Is the skylake library init and ready to use
    bool Skylake_IsTheLibraryInitialize() noexcept;

    //! Is the skylake library init and ready to use for the calling thread
    bool Skylake_IsTheLibraryInitialize_Thread() noexcept;
}
