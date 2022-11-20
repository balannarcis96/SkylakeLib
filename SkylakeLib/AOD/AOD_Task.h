//!
//! \file AOD_Task.h
//! 
//! \brief Async Object bound Dispatcher Task abstractions for SkylakeLib
//! 
//! \reference https://github.com/balannarcis96/Dispatcher (G.O.D: Grand Object-bound Dispatcher)
//!
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

//IAODTaskBase
namespace SKL
{
    constexpr size_t CAODTaskMinimumSize = 1;
    struct IAODTaskBase
    { 
        IAODTaskBase* volatile Next{ nullptr }; //!< Intrusive singly-linked list next pointer
    };
}

//AODSharedObjectTask
namespace SKL
{
    //! 
    //! \brief Single level dispatched task
    //! 
    //! \important Do not temper! Any modifications that will affect sizeof(ITask) will break the task abstraction.
    //! 
    struct IAODSharedObjectTask : IAODTaskBase
    {   
        using TDispatchFunctionPtr = void( SKL_CDECL* )( AOD::SharedObject& ) noexcept;
        using TDispatchProto       = ASD::UniqueFunctorWrapper<CAODTaskMinimumSize, TDispatchFunctionPtr>;

        IAODSharedObjectTask() = default;
        ~IAODSharedObjectTask() noexcept 
        {
            Clear();
        }

        //! \brief Dispatch this task
        SKL_FORCEINLINE void Dispatch() noexcept
        {
            SKL_ASSERT( false == IsNull() );
            SKL_ASSERT( nullptr != Parent.get() );

            CastSelfToProto().Dispatch( *Parent );
        }
        
        //! Is this task valid
        SKL_FORCEINLINE SKL_NODISCARD bool IsNull() const noexcept
        {
            return CastSelfToProto().IsNull();
        }

        //! Clear the underlying functor
        SKL_FORCEINLINE void Clear() noexcept
        {
            CastSelfToProto().Destroy();
        }

        //! Set parent AOD Object
        void SetParent( AOD::SharedObject* InObject )noexcept;

        //! Get parent AOD Object ptr
        SKL_FORCEINLINE SKL_NODISCARD AOD::SharedObject* GetParent() const noexcept { return Parent.get(); }

        //! Set due time
        SKL_FORCEINLINE void SetDue( TDuration AfterMilliseconds ) noexcept
        {
            Due = GetSystemUpTickCount() + static_cast<TEpochTimePoint>( AfterMilliseconds );
        }

        //! Is this task due
        SKL_FORCEINLINE SKL_NODISCARD bool IsDue( TEpochTimePoint InNow ) const noexcept
        {
            return InNow >= Due;
        }

        //! A > B
        SKL_FORCEINLINE SKL_NODISCARD bool operator>( const IAODSharedObjectTask& Other ) const noexcept    
        {
            return Due > Other.Due;
        }

    protected:
        SKL_FORCEINLINE const TDispatchProto& CastSelfToProto() const noexcept
        {
            return *reinterpret_cast<const TDispatchProto*>( 
                reinterpret_cast<const uint8_t*>( this ) + sizeof( IAODSharedObjectTask )
            );
        }

        SKL_FORCEINLINE TDispatchProto& CastSelfToProto() noexcept
        {
            return *reinterpret_cast<TDispatchProto*>( 
                reinterpret_cast<uint8_t*>( this ) + sizeof( IAODSharedObjectTask )
            );
        }

        TSharedPtr<AOD::SharedObject> Parent{ nullptr }; //!< Parent object ref, the AOD object, this task will be dispatched on
        TEpochTimePoint               Due   { 0 };       //!< Used for when this task is delayed

        friend struct AODTaskQueue;
    };

    template<size_t TaskSize>
    struct AODSharedObjectTask: IAODSharedObjectTask
    {
        using TDispatch = ASD::UniqueFunctorWrapper<TaskSize, typename IAODSharedObjectTask::TDispatchFunctionPtr>;

        AODSharedObjectTask() noexcept = default;
        ~AODSharedObjectTask() noexcept = default;
        
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

//AODStaticObjectTask
namespace SKL
{
    //! 
    //! \brief Single level dispatched task
    //! 
    //! \important Do not temper! Any modifications that will affect sizeof(ITask) will break the task abstraction.
    //! 
    struct IAODStaticObjectTask : IAODTaskBase
    {   
        using TDispatchFunctionPtr = void( SKL_CDECL* )() noexcept;
        using TDispatchProto       = ASD::UniqueFunctorWrapper<CAODTaskMinimumSize, TDispatchFunctionPtr>;

        IAODStaticObjectTask() = default;
        ~IAODStaticObjectTask() noexcept 
        {
            Clear();
        }

        //! Dispatch this task
        SKL_FORCEINLINE void Dispatch() noexcept
        {
            SKL_ASSERT( false == IsNull() );

            CastSelfToProto().Dispatch();
        }
        
        //! Is this task valid
        SKL_FORCEINLINE SKL_NODISCARD bool IsNull() const noexcept
        {
            return CastSelfToProto().IsNull();
        }

        //! Clear the underlying functor
        SKL_FORCEINLINE void Clear() noexcept
        {
            CastSelfToProto().Destroy();
        }
        
        //! Set parent AOD Object
        SKL_FORCEINLINE void SetParent( AOD::StaticObject* InObject ) noexcept
        {
            Parent = InObject;
        }
        
        //! Get parent AOD Object ptr
        SKL_FORCEINLINE SKL_NODISCARD AOD::StaticObject* GetParent() const noexcept { return Parent; }

        //! Set due time
        SKL_FORCEINLINE void SetDue( TDuration AfterMilliseconds ) noexcept
        {
            Due = GetSystemUpTickCount() + static_cast<TEpochTimePoint>( AfterMilliseconds );
        }

        //! Is this task due
        SKL_FORCEINLINE SKL_NODISCARD bool IsDue( TEpochTimePoint InNow ) const noexcept
        {
            return InNow >= Due;
        }

        //! A > B
        SKL_FORCEINLINE SKL_NODISCARD bool operator>( const IAODStaticObjectTask& Other ) const noexcept    
        {
            return Due > Other.Due;
        }

    protected:
        SKL_FORCEINLINE const TDispatchProto& CastSelfToProto() const noexcept
        {
            return *reinterpret_cast<const TDispatchProto*>( 
                reinterpret_cast<const uint8_t*>( this ) + sizeof( IAODStaticObjectTask )
            );
        }

        SKL_FORCEINLINE TDispatchProto& CastSelfToProto() noexcept
        {
            return *reinterpret_cast<TDispatchProto*>( 
                reinterpret_cast<uint8_t*>( this ) + sizeof( IAODStaticObjectTask )
            );
        }

        AOD::StaticObject* Parent{ nullptr }; //!< Parent object ptr, the AOD object, this task will be dispatched on
        TEpochTimePoint    Due   { 0 };       //!< Used for when this task is delayed

        friend struct AODTaskQueue;
    };

    template<size_t TaskSize>
    struct AODStaticObjectTask: IAODStaticObjectTask
    {
        using TDispatch = ASD::UniqueFunctorWrapper<TaskSize, typename IAODStaticObjectTask::TDispatchFunctionPtr>;

        AODStaticObjectTask() noexcept = default;
        ~AODStaticObjectTask() noexcept = default;
        
        //! Set the functor for this task [void( SKL_CDECL* )() noexcept]
        template<typename TFunctor>
        SKL_FORCEINLINE void operator+=( TFunctor&& InFunctor ) noexcept
        {
            // set the dispatch functor
            OnDispatch += std::forward<TFunctor>( InFunctor );
        }

        //! Set the functor for this task [void( SKL_CDECL* )() noexcept]
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

//AODCustomObjectTask
namespace SKL
{
    namespace AOD
    {
        struct CustomObject;

        struct CustomObjectDeallocator
        {
            using TDecayObject          = CustomObject;
            using MyMemoryPolicy        = MemoryPolicy::SharedMemoryPolicy;
            using MyMemoryPolicyApplier = SKL::MemoryPolicy::MemoryPolicyApplier<MyMemoryPolicy>;

            SKL_FORCEINLINE void operator()( CustomObject* InPtr ) const noexcept
            {
                Deallocate( InPtr );
            }

            static void Deallocate( CustomObject* InPtr ) noexcept;
        };
    }

    using TCustomObjectSharedPtr = TSharedPtr<AOD::CustomObject, AOD::CustomObjectDeallocator>;

    //! 
    //! \brief Single level dispatched task
    //! 
    //! \important Do not temper! Any modifications that will affect sizeof(ITask) will break the task abstraction.
    //! 
    struct IAODCustomObjectTask : IAODTaskBase
    {   
        using TDispatchFunctionPtr = void( SKL_CDECL* )( AOD::CustomObject& ) noexcept;
        using TDispatchProto       = ASD::UniqueFunctorWrapper<CAODTaskMinimumSize, TDispatchFunctionPtr>;

        IAODCustomObjectTask() = default;
        ~IAODCustomObjectTask() noexcept 
        {
            Clear();
        }

        //! \brief Dispatch this task
        SKL_FORCEINLINE void Dispatch() noexcept
        {
            SKL_ASSERT( false == IsNull() );
            SKL_ASSERT( nullptr != Parent.get() );

            CastSelfToProto().Dispatch( *Parent );
        }
        
        //! Is this task valid
        SKL_FORCEINLINE SKL_NODISCARD bool IsNull() const noexcept
        {
            return CastSelfToProto().IsNull();
        }

        //! Clear the underlying functor
        SKL_FORCEINLINE void Clear() noexcept
        {
            CastSelfToProto().Destroy();
        }

        //! Set parent AOD Object
        void SetParent( AOD::CustomObject* InObject )noexcept;
        
        //! Get parent AOD Object ptr
        SKL_FORCEINLINE SKL_NODISCARD AOD::CustomObject* GetParent() const noexcept { return Parent.get(); }

        //! Set due time
        SKL_FORCEINLINE void SetDue( TDuration AfterMilliseconds ) noexcept
        {
            Due = GetSystemUpTickCount() + static_cast<TEpochTimePoint>( AfterMilliseconds );
        }

        //! Is this task due
        SKL_FORCEINLINE SKL_NODISCARD bool IsDue( TEpochTimePoint InNow ) const noexcept
        {
            return InNow >= Due;
        }

        //! A > B
        SKL_FORCEINLINE SKL_NODISCARD bool operator>( const IAODCustomObjectTask& Other ) const noexcept    
        {
            return Due > Other.Due;
        }

    protected:
        SKL_FORCEINLINE const TDispatchProto& CastSelfToProto() const noexcept
        {
            return *reinterpret_cast<const TDispatchProto*>( 
                reinterpret_cast<const uint8_t*>( this ) + sizeof( IAODCustomObjectTask )
            );
        }

        SKL_FORCEINLINE TDispatchProto& CastSelfToProto() noexcept
        {
            return *reinterpret_cast<TDispatchProto*>( 
                reinterpret_cast<uint8_t*>( this ) + sizeof( IAODCustomObjectTask )
            );
        }

        TCustomObjectSharedPtr Parent{ nullptr }; //!< Parent object ref, the AOD object, this task will be dispatched on
        TEpochTimePoint        Due   { 0 };       //!< Used for when this task is delayed

        friend struct AODTaskQueue;
    };

    template<size_t TaskSize>
    struct AODCustomObjectTask: IAODCustomObjectTask
    {
        using TDispatch = ASD::UniqueFunctorWrapper<TaskSize, typename IAODCustomObjectTask::TDispatchFunctionPtr>;

        AODCustomObjectTask() noexcept = default;
        ~AODCustomObjectTask() noexcept = default;
        
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
    struct IAODSharedObjectTaskPtrComparer
    {
        SKL_FORCEINLINE SKL_NODISCARD bool operator()( const IAODSharedObjectTask* Left, const IAODSharedObjectTask* Right ) const noexcept
        {
            SKL_ASSERT( nullptr != Left );
            SKL_ASSERT( nullptr != Right );

            return ( *Left ) > ( *Right );
        }
    };

    struct IAODStaticObjectTaskPtrComparer
    {
        SKL_FORCEINLINE SKL_NODISCARD bool operator()( const IAODStaticObjectTask* Left, const IAODStaticObjectTask* Right ) const noexcept
        {
            SKL_ASSERT( nullptr != Left );
            SKL_ASSERT( nullptr != Right );

            return ( *Left ) > ( *Right );
        }
    };

    struct IAODCustomObjectTaskPtrComparer
    {
        SKL_FORCEINLINE SKL_NODISCARD bool operator()( const IAODCustomObjectTask* Left, const IAODCustomObjectTask* Right ) const noexcept
        {
            SKL_ASSERT( nullptr != Left );
            SKL_ASSERT( nullptr != Right );

            return ( *Left ) > ( *Right );
        }
    };
}