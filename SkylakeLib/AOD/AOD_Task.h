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

    struct IAODTaskBase
    { 
        IAODTask* volatile Next{ nullptr }; //!< Intrusive singly-linked list next pointer
    };

    //! 
    //! \brief Single level dispatched task
    //! 
    //! \important Do not temper! Any modifications that will affect sizeof(ITask) will break the task abstraction.
    //! 
    struct IAODTask : IAODTaskBase
    {   
        using TDispatchFunctionPtr = void( SKL_CDECL* )( AODObject& ) noexcept;
        using TDispatchProto       = ASD::UniqueFunctorWrapper<CAODTaskMinimumSize, TDispatchFunctionPtr>;

        IAODTask() = default;
        ~IAODTask() noexcept 
        {
            Clear();
        }

        //! \brief Dispatch this task
        SKL_FORCEINLINE void Dispatch() noexcept
        {
            SKL_ASSERT( false == IsNull() );
            SKL_ASSERT( nullptr == Parent.get() );

            CastSelfToProto().Dispatch( *Parent );
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

        SKL_FORCEINLINE void SetParent( AODObject* InObject )noexcept
        {
            TSharedPtr<AODObject>::Static_IncrementReference( InObject );
            Parent.Pointer = InObject;
        }

        SKL_FORCEINLINE void SetDue( TDuration AfterMilliseconds ) noexcept
        {
            Due = GetSystemUpTickCount() + AfterMilliseconds;
        }

        SKL_FORCEINLINE bool operator>( const IAODTask& Other ) noexcept    
        {
            return Due > Other.Due;
        }

    protected:
        const TDispatchProto& CastSelfToProto() const noexcept
        {
            return *reinterpret_cast<const TDispatchProto*>( 
                reinterpret_cast<const uint8_t*>( this ) + sizeof( IAODTask )
            );
        }

        TDispatchProto& CastSelfToProto() noexcept
        {
            return *reinterpret_cast<TDispatchProto*>( 
                reinterpret_cast<uint8_t*>( this ) + sizeof( IAODTask )
            );
        }

        TSharedPtr<AODObject> Parent{ nullptr};  //!< Parent object ref, the AOD object, this task will be dispatched on
        TEpochTimePoint       Due   { 0 };       //!< Used for when this task is delayed

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
