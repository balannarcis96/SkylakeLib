//!
//! \file TLSSyncTask.h
//! 
//! \brief TLS sync system task abstraction
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    constexpr size_t CITLSSyncTask_TaskMinimumSize = 1;
    
    //! 
    //! \brief Single level dispatched task
    //! 
    //! \important Do not temper! Any modifications that will affect sizeof(ITask) will break the task abstraction.
    //! 
    struct ITLSSyncTask
    {   
        using TDispatchFunctionPtr = void( SKL_CDECL* )( Worker&, WorkerGroup&, bool ) noexcept;
        using TDispatchProto       = ASD::UniqueFunctorWrapper<CITLSSyncTask_TaskMinimumSize, TDispatchFunctionPtr>;

        ITLSSyncTask() = default;
        ~ITLSSyncTask() noexcept 
        {
            Clear();
        }

        //! Dispatch this task
        SKL_FORCEINLINE void Dispatch( Worker& InWorker, WorkerGroup& InGroup, bool bIsLast ) noexcept
        {
            SKL_ASSERT( false == IsNull() );
            CastSelfToProto().Dispatch( InWorker, InGroup, bIsLast );
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
            return *reinterpret_cast<const TDispatchProto*>( 
                reinterpret_cast<const uint8_t*>( this )
            );
        }

        TDispatchProto& CastSelfToProto() noexcept
        {
            return *reinterpret_cast<TDispatchProto*>( 
                reinterpret_cast<uint8_t*>( this ) 
            );
        }
    };

    template<size_t TaskSize>
    struct TLSSyncTask : ITLSSyncTask
    {
        using TDispatch = ASD::UniqueFunctorWrapper<TaskSize, typename ITLSSyncTask::TDispatchFunctionPtr>;

        TLSSyncTask() noexcept = default;
        ~TLSSyncTask() noexcept = default;
        
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
    ITLSSyncTask* MakeTLSSyncTaskRaw( uint16_t WorkersCount, TFunctor&& InFunctor ) noexcept requires( sizeof( TFunctor ) >= CITLSSyncTask_TaskMinimumSize )
    {
        using TaskType = TLSSyncTask<sizeof( TFunctor )>;
        // allocate
        auto* NewTask { MakeSharedRaw<TaskType>() };
        SKL_ASSERT( nullptr != NewTask );
        
        TSharedPtr<TaskType>::Static_SetReferenceCount( NewTask, WorkersCount );

        // set functor
        NewTask->SetDispatch( std::forward<TFunctor>( InFunctor ) );

        // cast to base and return
        return reinterpret_cast<ITLSSyncTask*>( NewTask );
    }
}