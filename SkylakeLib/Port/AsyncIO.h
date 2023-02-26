//!
//! \file AsyncIO.h
//! 
//! \brief Async IO platform abstraction layer for SkylakeLib
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    //! Platform specific async IO API
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
        
        //! Get the OS specific handle to the API
        THandle GetOsHandle() const noexcept { return QueueHandle.load_relaxed(); }

        //! Get the max number of threads that can access this api instance at once
        int32_t GetNumberOfThreads() const noexcept { return ThreadsCount.load_relaxed(); }

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
        
        //! \brief Attempt to retrieve completed async IO request from the OS 
        //! \remarks Will block
        //! \param OutCompletedRequestOpaqueTypeInstancePtr ptr to ptr that will contain the instance of the opaque type passed to the OS when making the async IO request
        //! \param OutNumberOfBytesTransferred number of bytes transferred for the async IO request (eg. when recv/send/red/write async request is completed)
        //! \param OutCompletionKey key used to identify the completed async IO request
        //! \returns RSuccess when a valid async IO request or custom work item is retrieved
        //! \returns RSuccessAsyncIORequestCancelled when a async IO request was canceled due to the socket being closed
        //! \returns RSystemTerminated when a valid, terminate sentinel is retrieved, signaling the termination of the this async IO system instance
        //! \returns RSystemFailure when a system failure occurs

        //! \brief Attempt to retrieve at most OutputBufferCount completed async IO requests from the OS 
        //! \param OutputBuffer buffer of async IO requests receiver entries
        //! \param OutputBufferCount no of items in OutputBuffer
        //! \param OutCount the no of entries populated in OutputBuffer
        //! \returns RSuccess when a valid async IO request or custom work item is retrieved
        //! \returns RSuccessAsyncIORequestCancelled when a async IO request was canceled due to the socket being closed
        //! \returns RSystemTerminated when a valid, terminate sentinel is retrieved, signaling the termination of the this async IO system instance
        //! \returns RSystemFailure when a system failure occurs
        //! \remarks [WIN32] This function has 100ns time resolution
        RStatus GetMultipleCompletedAsyncRequest( AsyncIOOpaqueEntryType* OutputBuffer, uint32_t OutputBufferCount, uint32_t& OutCount ) noexcept;
        
        //! \brief Attempt to retrieve at most OutputBufferCount completed async IO requests from the OS 
        //! \param OutputBuffer buffer of async IO requests receiver entries
        //! \param OutputBufferCount no of items in OutputBuffer
        //! \param OutCount the no of entries populated in OutputBuffer
        //! \param InTimeout Time in milliseconds to wait for any async IO request to be completed
        //! \returns RSuccess when a valid async IO request or custom work item is retrieved
        //! \returns RSuccessAsyncIORequestCancelled when a async IO request was canceled due to the socket being closed
        //! \returns RSystemTerminated when a valid, terminate sentinel is retrieved, signaling the termination of the this async IO system instance
        //! \returns RSystemFailure when a system failure occurs
        //! \remarks [WIN32] This function has 100ns time resolution
        RStatus TryGetMultipleCompletedAsyncRequest( AsyncIOOpaqueEntryType* OutputBuffer, uint32_t OutputBufferCount, uint32_t& OutCount, uint32_t InTimeout ) noexcept;

        //! \brief Attempt to enqueue custom async work
        //! \param InCompletionKey completion key representing the work (eg pointer to object)
        //! \return RSuccess on success
        //! \return RFail on failure
        RStatus QueueAsyncWork( TCompletionKey InCompletionKey ) noexcept;

        //! Associate the socket to this async IO API
        RStatus AssociateToTheAPI( TSocket InSocket ) const noexcept;

        //! \brief Start an async receive request on InSocket
        //! \param InSocket target stream socket to receive from
        //! \param InBuffer buffer to receive into 
        //! \param InOpaqueObject opaque object instance
        //! \return RSuccess on success
        //! \return RFail on failure
        static RStatus ReceiveAsync( TSocket InSocket, IBuffer* InBuffer, TSharedPtr<AsyncIOOpaqueType> InOpaqueObject ) noexcept;

        //! \brief Start an async send request on InSocket
        //! \param InSocket target stream socket to send to
        //! \param InBuffer buffer to receive into 
        //! \param InOpaqueObject opaque object instance
        //! \return RSuccess on success
        //! \return RFail on failure
        static RStatus SendAsync( TSocket InSocket, IBuffer* InBuffer, TSharedPtr<AsyncIOOpaqueType> InOpaqueObject ) noexcept;

        //! \brief Start an async send request on InSocket
        //! \param InSocket target stream socket to receive from
        //! \param InAsyncIOTask the send async IO task
        //! \return RSuccess on success
        //! \return RFail on failure
        static RStatus SendAsync( TSocket InSocket, TSharedPtr<IAsyncIOTask> InAsyncIOTask ) noexcept;

        //! \brief Start an async receive request on InSocket
        //! \param InSocket target stream socket to receive from
        //! \param InAsyncIOTask the send async IO task
        //! \return RSuccess on success
        //! \return RFail on failure
        static RStatus ReceiveAsync(TSocket InSocket, TSharedPtr<IAsyncIOTask> InAsyncIOTask ) noexcept;

    private:
        std::relaxed_value<THandle> QueueHandle  { 0 };
        std::relaxed_value<int32_t> ThreadsCount { 0 };
    };
}