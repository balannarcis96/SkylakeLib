//!
//! \file Task.h
//! 
//! \brief Task abstractions based on ASD functor wrapper types
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
 constexpr size_t CTaskMinimumSize = 1;
    
    //! 
    //! \brief Single level dispatched task
    //! 
    //! \important Do not temper! Any modifications that will affect sizeof(IAsyncTask) will break the task abstraction.
    //! 
    struct IAsyncTask
    {   
        using TDispatchFunctionPtr = void( ASD_CDECL* )() noexcept;
        using TDispatchProto       = ASD::UniqueFunctorWrapper<CTaskMinimumSize, TDispatchFunctionPtr>;

        IAsyncTask() = default;
        ~IAsyncTask() noexcept 
        {
            Clear();
        }

        //! 
        //! \brief Dispatch this task
        //! 
        ASD_FORCEINLINE void Dispatch() const noexcept
        {
            //ASD_ASSERT( false == IsNull() );
            CastSelfToProto().Dispatch();
        }
        
        //! 
        //! Is this task valid
        //! 
        ASD_FORCEINLINE bool IsNull() const noexcept
        {
            return CastSelfToProto().IsNull();
        }

        //! 
        //! Clear the underlying functor
        //! 
        ASD_FORCEINLINE void Clear() noexcept
        {
            CastSelfToProto().Destroy();
        }

        //! 
        //! Get the total size of the whole task. eg used for fast, correct deallocation
        //! 
        ASD_FORCEINLINE uint64_t GetTotalSize() const noexcept
        {       
            return TotalSize;
        }

    protected:
        const TDispatchProto& CastSelfToProto() const noexcept
        {
            const uint8_t* SelfPointer = reinterpret_cast<const uint8_t*>( this );
            return *reinterpret_cast<const TDispatchProto*>( SelfPointer + sizeof( void* ) );
        }

        TDispatchProto& CastSelfToProto() noexcept
        {
            uint8_t* SelfPointer = reinterpret_cast<uint8_t*>( this );
            return *reinterpret_cast<TDispatchProto*>( SelfPointer + sizeof( void* ) );
        }

        uint64_t TotalSize; //!< Task total size, used for fast, correct deallocation
    };

    template<size_t TaskSize>
    struct AsyncTask : IAsyncTask
    {
        static_assert( TaskSize >= 1, "The task size must be at least 1 byte" );

        using TDispatch = ASD::UniqueFunctorWrapper<TaskSize, typename IAsyncTask::TDispatchFunctionPtr>;

        AsyncTask() noexcept = default;
        ~AsyncTask() noexcept = default;
        
        //! 
        //! Set the functor for this task
        //! 
        template<typename TFunctor>
        ASD_FORCEINLINE void operator+=( TFunctor&& InFunctor ) noexcept
        {
            // set the dispatch functor
            OnDispatch += std::forward<TFunctor>( InFunctor );
            this->TotalSize = sizeof( *this );
        }

        //! 
        //! Set the functor for this task
        //! 
        template<typename TFunctor>
        ASD_FORCEINLINE void SetDispatch( TFunctor&& InFunctor ) noexcept
        {
            // set the dispatch functor
            OnDispatch += std::forward<TFunctor>( InFunctor );
            this->TotalSize = sizeof( *this );
        }

    private:
        TDispatch OnDispatch; //!< The functor to dispatch for this task
    };
}