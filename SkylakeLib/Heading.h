//!
//! \file Heading.h
//! 
//! \brief Forwarded types and types
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

#if defined(SKL_USE_MIMALLOC)
    #include <mimalloc.h>

    #define SKL_MALLOC( InSize ) mi_malloc( InSize )
    #define SKL_MALLOC_ALIGNED( InSize, InAlignment ) mi_malloc_aligned( InSize, InAlignment )

    #define SKL_FREE( InPtr ) mi_free( InPtr )
    #define SKL_FREE_ALIGNED( InPtr, InAlignment ) mi_free_aligned( InPtr, InAlignment )
    #define SKL_FREE_SIZE_ALIGNED( InPtr, InSize, InAlignment ) mi_free_size_aligned( InPtr, InSize, InAlignment )
#else
    extern void* GAllocAligned( size_t Size, size_t Alignment ) noexcept;
    extern void  GFreeAligned( void* Ptr ) noexcept;

    #define SKL_MALLOC( InSize ) malloc( InSize )
    #define SKL_MALLOC_ALIGNED( InSize, InAlignment ) GAllocAligned( InSize, InAlignment )
    
    #define SKL_FREE( InPtr ) free( InPtr )
    #define SKL_FREE_ALIGNED( InPtr, InAlignment ) GFreeAligned( InPtr )
    #define SKL_FREE_SIZE_ALIGNED( InPtr, InSize, InAlignment ) GFreeAligned( InPtr )
#endif

//! Forward declarations
namespace SKL
{
    struct ITask;
    struct IAsyncIOTask;
    struct IAODStaticObjectTask;
    struct IAODSharedObjectTask;
    class WorkerGroup;
    class Worker;
    class ServerInstance;

    namespace AOD
    {
        struct Object;
        struct StaticObject;
        struct SharedObject;
    }
}

namespace SKL
{
    union ServerInstanceFlags
    {
        struct
        {
            uint8_t bAllGroupsAreActive: 1;
            uint8_t bSupportsDelayedTasks: 1;
        };   
        uint32_t Flags{ 0 };
    };

    struct WorkerGroupTagFlags
    {
        bool bIsActive                     { false };   //!< Is this an pro-active worker [ it has an active ticks/second loop ]
        bool bEnableAsyncIO                { false };   //!< Does this group handle tasks and async IO tasks [ an AsyncIO instance will be created for the group if true ]
        bool bSupportsAOD                  { false };   //!< true -> Workers in this group can use AOD (Async Object Dispatcher) delayed tasks directly (handled by the same thread)
        bool bHandlesTimerTasks            { false };   //!< true -> This group handles global and AOD(if bSupportsAOD=true) delayed tasks -> requires [bIsActive=true]
        bool bSupportsTLSSync              { false };   //!< Supports TLSSync [ TLSSync it is its own feature, please get documented before use ]
        bool bCallTickHandler              { false };   //!< true -> the workers in the group will call the thick handler
        bool bTickWorkerServices           { false };   //!< true -> each worker in the group will call the thick handler of all all registered worker services -> requires [bIsActive=true]
        bool bHasWorkerGroupSpecificTLSSync{ false };   //!< true -> SyncTLS can be used for specific worker group
        bool bEnableTaskQueue              { false };   //!< true -> each worker in group will use a SCMP queue for tasks -> requires[bEnableAsyncIO=false]
    };

    struct WorkerGroupTag final : public WorkerGroupTagFlags
    {
        uint32_t       TickRate                       { 0 };       //!< Tick rate of worker [ bIsActive == true ]
        uint32_t       SyncTLSTickRate                { 0 };       //!< Tick rate of tls sync [ bSupportsTLSSync == true ]
        uint16_t       Id                             { 0 };       //!< UID of the tag [max 65536 workers] recommended to treat id as index starting from 1 (0 = invalid id)
        uint16_t       WorkersCount                   { 0 };       //!< Number of workers in the group
        bool           bPreallocateAllThreadLocalPools{ false };   //!< true -> Preallocate all pools in ThreadLocalMemoryManager
        bool           bSupportesTCPAsyncAcceptors    { false };   //!< Does this group supports and handles TCP async acceptors
        const wchar_t *Name                           { nullptr }; //!< Name of the worker group
        mutable bool   bIsValid                       { false };   //!< Initialize this member to false if you want your server to run correctly ;)

        //! [*]
        //!  Case 1.If all worker groups in the server instance are active [bIsActive=true], delayed tasks produced on any thread will be processed by the thread that produced it
        //!         Why to consider Case 1:
        //!           -  All tasks will be allocated through the thread local allocator -> very fast allocation/deallocation
        //!           -  No contention between threads (no load balancing)
        //!           -  Better time precision on delayed tasks
        //!
        //!  Case 2.If not all worker groups in the server are active and you need the possibility to delay tasks in non-active worker groups then 
        //!    all worker groups marked with [bHandlesTimerTasks=true] will be used to check and dispatch delayed tasks (using RR-load balancing between worker groups and between workers)
        //!          Why to consider Case 2:
        //!              - The server can have inactive worker groups that can delay tasks
        //!
        //! Important: If no active worker group is present, the delayed tasks feature must not be used. Add at least one active worker group if you need to delay tasks.
        //!
        //! Note: Delayed tasks include free delayed tasks and AOD delayed tasks
        //!
        //! Note: [bSupportesTCPAsyncAcceptors=true] Accepted sockets will not be associated to any async IO API.
        //!

        SKL_FORCEINLINE SKL_NODISCARD bool Validate() const noexcept
        {
            SKL_ASSERT( nullptr != Name );

            if ( 0U == Id )
            {
                SKLL_ERR_FMT( "WorkerGroupTag[%ws] Invalid Id %hu!", Name, Id );
                return false;
            }

            if ( false == bIsActive && false == bEnableAsyncIO )
            {
                SKLL_ERR_FMT( "WorkerGroupTag[%ws] All inactive worker groups must be marked [bIsActive=false;bEnableAsyncIO=true]!", Name );
                return false;
            }

            if( false == bIsActive && bEnableTaskQueue )
            {
                SKLL_ERR_FMT( "WorkerGroupTag[%ws] Reactive worker cannot have a task queue [bIsActive=false;bEnableTaskQueue=true]!", Name );
                return false;
            }

            if ( true == bIsActive && true == bEnableAsyncIO && false == bCallTickHandler && false == bHandlesTimerTasks && false == bSupportsAOD )
            {
                SKLL_WRN_FMT( "WorkerGroupTag[%ws] For [bIsActive=true;bEnableAsyncIO=true;bCallTickHandler=false;bHandlesTimerTasks=false;bSupportsAOD=false] Recommended to use a reactive worker group instead!", Name );
            }

            if( true == bSupportesTCPAsyncAcceptors && false == bEnableAsyncIO )
            {
                SKLL_ERR_FMT( "WorkerGroupTag[%ws] [bSupportesTCPAsyncAcceptors == true] requires -> bEnableAsyncIO = true!", Name );
                return false;
            }

            if( true == bHandlesTimerTasks && false == bIsActive )
            {
                SKLL_ERR_FMT( "WorkerGroupTag[%ws] [bHandlesTimerTasks == true] requires -> bIsActive = true!", Name );
                return false;
            }
            
            if( true == bTickWorkerServices && false == bIsActive )
            {
                SKLL_ERR_FMT( "WorkerGroupTag[%ws] [bTickWorkerServices == true] requires -> bIsActive = true!", Name );
                return false;
            }

            bIsValid = true;
            return true;
        }        

        SKL_FORCEINLINE SKL_NODISCARD bool IsValid() const noexcept { return bIsValid; }
    }; 
}
