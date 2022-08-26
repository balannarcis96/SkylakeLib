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

					//Destruct and deallocate
					MemoryStrategy::SharedMemoryStrategy<ITLSSyncTask>::DestructDeallocator::Deallocate( Task );

					//Pop task
					Queue.TLSPop();
				}

				//Try pop next task for this thread
				Task = Queue.TLSPopNext();
			}
		}

	private:
		TLSSyncQueue Queue{};
    };
}