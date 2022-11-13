//!
//! \file TLSSyncSystem.h
//! 
//! \brief TLS sync system abstraction
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    class ServerInstance;

    struct TLSSyncSystem
    {
         TLSSyncSystem() noexcept  = default;
        ~TLSSyncSystem() noexcept
        {
            Queue.Clear();
        }

        TLSSyncSystem( const TLSSyncSystem & ) = delete;
        TLSSyncSystem &operator=( const TLSSyncSystem & ) = delete;
        TLSSyncSystem( TLSSyncSystem && )= delete;
        TLSSyncSystem &operator=( TLSSyncSystem && ) = delete;

        //! Called by each worker that supports TLSSync 
        void TLSInitialize( Worker& /*InWorker*/, WorkerGroup& /*InGroup*/ ) noexcept
        {
            Queue.TLSInitialize();
        }

        //! Called by each worker that supports TLSSync 
        void TLSShutdown() noexcept
        {
        }

        //! Push new TLSSync task 
        SKL_FORCEINLINE void PushTask( ITLSSyncTask* InTask ) noexcept
        {
            Queue.Push( InTask );
        }

        //! Get the number of workers that will execute the TLSSync tasks
        SKL_FORCEINLINE SKL_NODISCARD uint64_t GetNoOfWorkersThatSupportTLSSync() const noexcept
        {
            SKL_ASSERT( 0 < NoOfWorkersThatSupportTLSSync );
            return NoOfWorkersThatSupportTLSSync;
        }
        
        //! Called by each worker that supports TLSSync 
        void TLSTick( Worker& InWorker, WorkerGroup& InGroup ) noexcept;

    private:
        TLSSyncQueue Queue                        {};    //!< Queue of all tasks
        uint64_t     NoOfWorkersThatSupportTLSSync{ 0 }; //!< No of workers that support TLS sync

        friend ServerInstance;
    };
}