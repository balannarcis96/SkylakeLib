//!
//! \file Platform.h
//! 
//! \brief Platform abstraction layer for SkylakeLib
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    //! Platform agnostic socket type 
    using TSocket = uint64_t;

    //! \brief Platform specific timer API
    struct Timer;

    //! \brief Platform specific, opaque type, for the async IO API
    struct AsyncIOOpaqueType;
    
    //! \brief Platform specific buffer type for async IO requests
    struct IBuffer;
    
    //! \brief Type used as key to identify async IO requests
    using TCompletionKey = void *;
    
    //! \brief type that can hold a "handle" on any platform
    using THandle = uint64_t;

    //! \brief type for the TLS slot
    using TLSSlot = uint32_t;
    
    //! \brief Platform specific async IO API
    struct AsyncIO
    {   
        //! \brief Initialize the OS async IO system
        static RStatus InitializeSystem() noexcept;
        
        //! \brief Shutdown the OS async IO system
        static RStatus ShutdownSystem() noexcept;

        //! \brief Start and instance of the OS async IO system
        //! \param InThreadsCount number of threads accessing this instance
        //! \returns RSuccess on success
        //! \returns RFail on failure
        RStatus Start( int32_t InThreadsCount ) noexcept;

        //! \brief Stop this instance of the OS async IO system
        //! \remarks All outstanding async IO requests will be canceled
        //! \returns RSuccess on success
        //! \returns RAlreadyPerformed when Stop() was already called
        //! \returns RFail on failure
        RStatus Stop() noexcept;
        
        //! \brief Attempt to retrieve completed async IO request from the OS 
        //! \remarks Will block
        //! \param OutCompletedRequestOpaqueTypeInstancePtr ptr to ptr that will contain the instance of the opaque type passed to the OS when making the async IO request
        //! \param OutNumberOfBytesTransferred number of bytes transferred for the async IO request (eg. when recv/send/red/write async request is completed)
        //! \param OutCompletionKey key used to identify the completed async IO request
        //! \returns RSuccess when a valid async IO request or custom work item is retrieved
        //! \returns RSuccessAsyncIORequestCancelled when a async IO request was canceled due to the socket being closed
        //! \returns RSystemTerminated when a valid, terminate sentinel is retrieved, signaling the termination of the this async IO system instance
        //! \returns RSystemFailure when a system failure occurs
        RStatus GetCompletedAsyncRequest( AsyncIOOpaqueType** OutCompletedRequestOpaqueTypeInstancePtr, uint32_t* OutNumberOfBytesTransferred, TCompletionKey* OutCompletionKey ) noexcept;

        //! \brief Attempt to retrieve completed async IO request from the OS 
        //! \param OutCompletedRequestOpaqueTypeInstancePtr ptr to ptr that will contain the instance of the opaque type passed to the OS when making the async IO request
        //! \param OutNumberOfBytesTransferred number of bytes transferred for the async IO request (eg. when recv/send/red/write async request is completed)
        //! \param OutCompletionKey key used to identify the completed async IO request
        //! \param InTimeout Time in milliseconds to wait for any async IO request to be completed
        //! \returns RSuccess when a valid async IO request or custom work item is retrieved
        //! \returns RSuccessAsyncIORequestCancelled when a async IO request was canceled due to the socket being closed
        //! \returns RTimeout when reached timeout value with no completed async IO request retrieved
        //! \returns RSystemTerminated when a valid, terminate sentinel is retrieved, signaling the termination of the this async IO system instance
        //! \returns RSystemFailure when a system failure occurs
        RStatus TryGetCompletedAsyncRequest( AsyncIOOpaqueType** OutCompletedRequestOpaqueTypeInstancePtr, uint32_t* OutNumberOfBytesTransferred, TCompletionKey* OutCompletionKey, uint32_t InTimeout ) noexcept;

        //! \brief Attempt to enqueue custom async work
        //! \param InCompletionKey completion key representing the work (eg pointer to object)
        //! \return RSuccess on success
        //! \return RFail on failure
        RStatus QueueAsyncWork( TCompletionKey InCompletionKey ) noexcept;

        //! \brief Start an async receive request on InSocket
        //! \param InSocket target stream socket to receive from
        //! \param InBuffer buffer to receive into 
        //! \param InOpaqueObject opaque object instance
        //! \return RSuccess on success
        //! \return RFail on failure
        RStatus ReceiveAsync( TSocket InSocket, IBuffer* InBuffer, AsyncIOOpaqueType* InOpaqueObject ) noexcept;

        //! \brief Start an async send request on InSocket
        //! \param InSocket target stream socket to send to
        //! \param InBuffer buffer to receive into 
        //! \param InOpaqueObject opaque object instance
        //! \return RSuccess on success
        //! \return RFail on failure
        RStatus SendAsync( TSocket InSocket, IBuffer* InBuffer, AsyncIOOpaqueType* InOpaqueObject ) noexcept;

    private:
        std::relaxed_value<THandle> QueueHandle  { 0 };
        std::relaxed_value<int32_t> ThreadsCount { 0 };
    };

    //! \brief Enable ANSI color support in the main console window
    //! \return RSuccess on success 
    RStatus EnableConsoleANSIColorSupport() noexcept;

    //! \brief Get the number of milliseconds that have elapsed since the system was started
    TEpochTimePoint GetSystemUpTickCount() noexcept;

    //! \brief Set the timer resolution of the OS
    RStatus SetOsTimeResolution( uint32_t InMilliseconds ) noexcept;

	struct PlatformTLS
	{
		static constexpr TLSSlot INVALID_SLOT_ID = 0xFFFFFFFF;

		/**
		 * \brief Return false if InSlotIndex is an invalid TLS slot
		 * \param InSlotIndex the TLS index to check
		 * \return true if InSlotIndex looks like a valid slot
		 */
		SKL_FORCEINLINE static bool IsValidTlsSlot( TLSSlot InSlotIndex ) noexcept
		{
			return InSlotIndex != INVALID_SLOT_ID;
		}
        
        //! \brief Get the calling thread id
        static uint32_t GetCurrentThreadId() noexcept;

        //! \brief Allocate new thread local storage slot for all threads of the process
        static TLSSlot AllocTlsSlot() noexcept;

        //! \brief Set the TLS value at InSlot for the calling thread
        static void SetTlsValue( TLSSlot InSlot, void* InValue ) noexcept;

        //! \brief Get the TLS value at InSlot for the calling thread
        static void* GetTlsValue( TLSSlot InSlot ) noexcept;

        //! \brief Free a previously allocated TLS slot
        static void FreeTlsSlot( TLSSlot InSlot ) noexcept;
    };
}

#if defined(SKL_BUILD_WINDOWS)
    #define SKL_PLATFORM_NAME "Windows"
    #include "Platform_Windows.h"
#elif defined(SKL_BUILD_FREEBSD)
    #define SKL_PLATFORM_NAME "FreeBSD"
    #include "Platform_Unix.h"
    #include "Platform_FreeBSD.h"
#elif defined(SKL_BUILD_UBUNTU)
    #define SKL_PLATFORM_NAME "Ubuntu"
    #include "Platform_Unix.h"
    #include "Platform_Ubuntu.h"
#else
    #error "Unsupported platform!"
#endif
