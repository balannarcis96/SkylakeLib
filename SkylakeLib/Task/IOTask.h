//!
//! \file IOTask.h
//! 
//! \brief IO Task abstractions based on ASD functor wrapper types
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
    //! \important Do not temper! Any modifications that will affect sizeof(ITask) will break the task abstraction.
    //! 
    struct IAsyncIOTask
    {   
        using TDispatchFunctionPtr = void( ASD_CDECL* )() noexcept;
        using TDispatchProto       = ASD::UniqueFunctorWrapper<CTaskMinimumSize, TDispatchFunctionPtr>;

        IAsyncIOTask() noexcept
        {
            memset( &OsOpaqueType, 0, sizeof( AsyncIOOpaqueType ) );
        }
        ~IAsyncIOTask() noexcept 
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

    protected:
        const TDispatchProto& CastSelfToProto() const noexcept
        {
            return *reinterpret_cast<const TDispatchProto*>( 
                reinterpret_cast<const uint8_t*>( this ) - sizeof( AsyncIOOpaqueType )
            );
        }

        TDispatchProto& CastSelfToProto() noexcept
        {
            return *reinterpret_cast<TDispatchProto*>( 
                reinterpret_cast<uint8_t*>( this ) - sizeof( AsyncIOOpaqueType )
            );
        }

        AsyncIOOpaqueType OsOpaqueType;
    };

    template<size_t TaskSize>
    struct AsyncIOTask : IAsyncIOTask
    {
        static_assert( TaskSize >= 1, "The task size must be at least 1 byte" );

        using TDispatch = ASD::UniqueFunctorWrapper<TaskSize, typename IAsyncIOTask::TDispatchFunctionPtr>;

        AsyncIOTask() noexcept = default;
        ~AsyncIOTask() noexcept = default;
        
        //! 
        //! Set the functor for this task
        //! 
        template<typename TFunctor>
        ASD_FORCEINLINE void operator+=( TFunctor&& InFunctor ) noexcept
        {
            // set the dispatch functor
            OnDispatch += std::forward<TFunctor>( InFunctor );
        }

        //! 
        //! Set the functor for this task
        //! 
        template<typename TFunctor>
        ASD_FORCEINLINE void SetDispatch( TFunctor&& InFunctor ) noexcept
        {
            // set the dispatch functor
            OnDispatch += std::forward<TFunctor>( InFunctor );
        }

    private:
        TDispatch OnDispatch; //!< The functor to dispatch for this task
    };
}
