//!
//! \file AOD_Task.h
//! 
//! \brief Async Object bound Dispatcher Task abstraction for SkylakeLib
//! 
//! \reference https://github.com/balannarcis96/Dispatcher (G.O.D: Grand Object-bound Dispatcher)
//!
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    constexpr size_t CAODTaskMinimumSize = 1;
    
    //! 
    //! \brief Single level dispatched task
    //! 
    //! \important Do not temper! Any modifications that will affect sizeof(ITask) will break the task abstraction.
    //! 
    struct IAODTask
    {   
        using TDispatchFunctionPtr = void( SKL_CDECL* )() noexcept;
        using TDispatchProto       = ASD::UniqueFunctorWrapper<CAODTaskMinimumSize, TDispatchFunctionPtr>;

        IAODTask() = default;
        ~IAODTask() noexcept 
        {
            Clear();
        }

        //! \brief Dispatch this task
        SKL_FORCEINLINE void Dispatch() const noexcept
        {
            //ASD_ASSERT( false == IsNull() );
            CastSelfToProto().Dispatch();
        }
        
        //! Is this task valid
        SKL_FORCEINLINE bool IsNull() const noexcept
        {
            return CastSelfToProto().IsNull();
        }

        //! Clear the underlying functor
        SKL_FORCEINLINE void Clear() noexcept
        {
            CastSelfToProto().Destroy();
        }

    protected:
        const TDispatchProto& CastSelfToProto() const noexcept
        {
            return *reinterpret_cast<const TDispatchProto*>( this );
        }

        TDispatchProto& CastSelfToProto() noexcept
        {
            return *reinterpret_cast<TDispatchProto*>( this );
        }

        IAODTask* volatile Next{ nullptr }; //!< Intrusive singly-linked list next pointer

        friend struct AODTaskQueue;
    };

    template<size_t TaskSize>
    struct AODTask: IAODTask
    {
        using TDispatch = ASD::UniqueFunctorWrapper<TaskSize, typename IAODTask::TDispatchFunctionPtr>;

        AODTask() noexcept = default;
        ~AODTask() noexcept = default;
        
        //! Set the functor for this task
        template<typename TFunctor>
        SKL_FORCEINLINE void operator+=( TFunctor&& InFunctor ) noexcept
        {
            // set the dispatch functor
            OnDispatch += std::forward<TFunctor>( InFunctor );
        }

        //! Set the functor for this task
        template<typename TFunctor>
        SKL_FORCEINLINE void SetDispatch( TFunctor&& InFunctor ) noexcept
        {
            // set the dispatch functor
            OnDispatch += std::forward<TFunctor>( InFunctor );
        }

    private:
        TDispatch OnDispatch; //!< The functor to dispatch for this task
    };
}
