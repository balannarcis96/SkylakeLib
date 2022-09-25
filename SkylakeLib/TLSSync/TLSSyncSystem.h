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
    struct TLSSyncSystem
    {
         TLSSyncSystem( ) noexcept  = default;
        ~TLSSyncSystem( ) noexcept = default;

        TLSSyncSystem( const TLSSyncSystem & ) = delete;
        TLSSyncSystem &operator=( const TLSSyncSystem & ) = delete;
        TLSSyncSystem( TLSSyncSystem && )= delete;
        TLSSyncSystem &operator=( TLSSyncSystem && ) = delete;

        void TLSInitialize( Worker& InWorker, WorkerGroup& InGroup ) noexcept
        {
            Queue.TLSInitialize();
        }

        void TLSShutdown() noexcept
        {
            //Queue.TLSShutdown();
        }

        SKL_FORCEINLINE void PushTask( ITLSSyncTask* InTask ) noexcept
        {
            Queue.Push( InTask );
        }

        void TLSTick( Worker& InWorker, WorkerGroup& InGroup ) noexcept
        {
            //Check if a new task was published for this thread
            auto* Task{ Queue.TLSFront() };
            while( nullptr != Task )
            {
                // Dispatch initial
                Task->Dispatch( InWorker, InGroup, false );
            
                //@TODO update the memory policy API to support this 
                auto& ControlBlock{ TSharedPtr<ITLSSyncTask>::MemoryPolicy::GetControlBlockForObject( Task ) };
                if( true == ControlBlock.ReleaseReference() )
                {
                    //Dispatch again to signal that the task was finalized
                    Task->Dispatch( InWorker, InGroup, true );

                    //Pop task
                    Queue.TLSPop();

                    //Destruct
                    GDestructNothrow( Task );

                    //Deallocate
                    GlobalMemoryManager::Deallocate( &ControlBlock, ControlBlock.BlockSize );
                }

                //Advance to next task on this thread
                Task = Queue.TLSNext();
            }
        }

    private:
        TLSSyncQueue Queue{};
    };
}