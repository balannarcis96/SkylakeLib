//!
//! \file Task.h
//! 
//! \brief Task abstractions based on ASD plus allocation strategies
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    constexpr size_t CITask_TaskMinimumSize = 1;
    
    struct ITaskBase
    {
        ITask* volatile Next{ nullptr }; //!< Intrusive singly-linked list next pointer
    };

    //! 
    //! \brief Single level dispatched task
    //! 
    //! \important Do not temper! Any modifications that will affect sizeof(ITask) will break the task abstraction.
    //! 
    struct ITask : ITaskBase
    {   
        using TDispatchFunctionPtr = void( SKL_CDECL* )( ITask* ) noexcept;
        using TDispatchProto       = ASD::UniqueFunctorWrapper<CITask_TaskMinimumSize, TDispatchFunctionPtr>;

        ITask() = default;
        ~ITask() noexcept 
        {
            Clear();
        }

        //! Dispatch this task
        SKL_FORCEINLINE void Dispatch() noexcept
        {
            SKL_ASSERT( false == IsNull() );
            CastSelfToProto().Dispatch( this );
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

        //! Set due epoch time point to delay this task to
        SKL_FORCEINLINE void SetDue( TDuration AfterMilliseconds ) noexcept
        {
            Due = GetSystemUpTickCount() + AfterMilliseconds;
        }

        //! Is this task due
        SKL_FORCEINLINE bool IsDue( TEpochTimePoint InNow ) const noexcept
        {
            return InNow >= Due;
        }

        SKL_FORCEINLINE bool operator>( const ITask& Other ) noexcept    
        {
            return Due > Other.Due;
        }

    protected:
        const TDispatchProto& CastSelfToProto() const noexcept
        {
            return *reinterpret_cast<const TDispatchProto*>( 
                reinterpret_cast<const uint8_t*>( this ) + sizeof( ITask )
            );
        }

        TDispatchProto& CastSelfToProto() noexcept
        {
            return *reinterpret_cast<TDispatchProto*>( 
                reinterpret_cast<uint8_t*>( this ) + sizeof( ITask )
            );
        }

        TEpochTimePoint Due { 0 }; //!< Used for when this task is delayed

        friend struct TaskQueue;
    };

    template<size_t TaskSize>
    struct Task : ITask
    {
        using TDispatch = ASD::UniqueFunctorWrapper<TaskSize, typename ITask::TDispatchFunctionPtr>;

        Task() noexcept = default;
        ~Task() noexcept = default;
        
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

namespace SKL
{
    template<typename TFunctor>
    ITask* MakeTaskRaw( TFunctor&& InFunctor ) noexcept
    {
        // allocate
        auto* NewTask { MakeSharedRaw<Task<sizeof( TFunctor )>>() };
        
        // set functor
        NewTask->SetDispatch( std::forward<TFunctor>( InFunctor ) );

        // cast to base and return
        return reinterpret_cast<ITask*>( NewTask );
    }

    template<typename TFunctor>
    TSharedPtr<ITask> MakeTask( TFunctor&& InFunctor ) noexcept
    {
        return { MakeTaskRaw( std::forward<TFunctor>( InFunctor ) ) };
    }

    //! Defer an newly allocated task
    bool DeferTask( ITask* InTask ) noexcept;

    //! Defer an already deferred task
    bool DeferTaskAgain( ITask* InTask ) noexcept;

    //! Defer functor execution asap [void( void )noexcept] 
    template<typename TFunctor>
    bool DeferTask( TFunctor&& InFunctor ) noexcept
    {
        // allocate
        auto* NewTask { MakeSharedRaw<Task<sizeof( TFunctor )>>() };
        
        // set functor
        NewTask->SetDispatch( std::forward<TFunctor>( InFunctor ) );

        // set due
        //NewTask->SetDue( AfterMilliseconds ); due = 0 -> ASAP

        // cast to base and defer
        return DeferTask( reinterpret_cast<ITask*>( NewTask ) );
    }

    //! Defer functor execution after AfterMilliseconds [void( void )noexcept] 
    template<typename TFunctor>
    bool DeferTask( TDuration AfterMilliseconds, TFunctor&& InFunctor ) noexcept
    {
        // allocate
        auto* NewTask { MakeSharedRaw<Task<sizeof( TFunctor )>>() };
        
        // set functor
        NewTask->SetDispatch( std::forward<TFunctor>( InFunctor ) );

        // set due
        NewTask->SetDue( AfterMilliseconds );

        // cast to base and defer
        return DeferTask( reinterpret_cast<ITask*>( NewTask ) );
    }
}