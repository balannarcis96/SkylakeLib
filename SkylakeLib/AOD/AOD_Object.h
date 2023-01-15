//!
//! \file AOD_Object.h
//! 
//! \brief Async Object bound Dispatcher abstractions for SkylakeLib
//! 
//! \reference https://github.com/balannarcis96/Dispatcher (G.O.D: Grand Object-bound Dispatcher)
//!
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

//Object
namespace SKL::AOD
{
    struct Object
    {
        Object() noexcept = default;
        ~Object() noexcept = default;

    protected:
        //First cache line
        std::relaxed_value<uint64_t> RemainingTasksCount; //!< Remaining tasks to execute on this object
        AODTaskQueue                 TaskQueue;           //!< Task queue
    };  
}

//SharedObject
namespace SKL::AOD
{
    struct SharedObject : public Object
    {
        SharedObject( void* TargetSharedPointer ) noexcept : TargetSharedPointer { TargetSharedPointer ? TargetSharedPointer : this } {}
        ~SharedObject() noexcept = default;

        //! Execute the functor thread safe relative to the object [void( AOD::SharedObject& ) noexcept]
        //! \returns RAllocationFailed if allocating the task object failed
        //! \returns RExecutedSync if the functor was dispatched sync (in this call)
        //! \returns RSuccess if the functor will be dispatched async
        template<typename TFunctor>
        SKL_FORCEINLINE SKL_NODISCARD RStatus DoAsync( TFunctor&& InFunctor ) noexcept
        {
            using TaskType = AODSharedObjectTask<sizeof(TFunctor)>;
            
            TaskType* NewTask{ MakeSharedRaw<TaskType>() };
            if( nullptr == NewTask ) SKL_UNLIKELY
            {   
                SKLL_ERR( "SharedObject::DoAsync() Failed to allocate task!" );
                return RAllocationFailed;
            }

            NewTask->SetParent( this );
            NewTask->SetDispatch( std::forward<TFunctor>( InFunctor ) );

            if( Dispatch( NewTask ) )
            {
                return RExecutedSync;
            }
        
            return RSuccess;
        }

        //! Execute the functor after [AfterMilliseconds], thread safe relative to the object [void( AOD::SharedObject& ) noexcept]
        //! \returns RAllocationFailed if allocating the task object failed
        //! \returns RFail if scheduling the task failed
        //! \returns RSuccess if the functor will be dispatched async
        template<typename TFunctor>
        SKL_FORCEINLINE SKL_NODISCARD RStatus DoAsyncAfter( TDuration AfterMilliseconds, TFunctor&& InFunctor ) noexcept
        {
            using TaskType = AODSharedObjectTask<sizeof(TFunctor)>;
            
            TaskType* NewTask{ MakeSharedRaw<TaskType>() };
            if( nullptr == NewTask ) SKL_UNLIKELY
            {   
                SKLL_ERR( "SharedObject::DoAsyncAfter() Failed to allocate task!" );
                return RAllocationFailed;
            }

            NewTask->SetParent( this );
            NewTask->SetDue( AfterMilliseconds );
            NewTask->SetDispatch( std::forward<TFunctor>( InFunctor ) );

            if ( false == DelayTask( NewTask ) ) SKL_UNLIKELY
            {
                return RFail;
            }

            return RSuccess;
        }

        //! Get the cached pointer to the parent instance
        void* GetParentObjectPointer() const noexcept { return TargetSharedPointer; }

        //! Get the cached pointer to the parent instance
        template<typename T>
        SKL_FORCEINLINE SKL_NODISCARD T& GetParentObject() const noexcept { return *reinterpret_cast<T*>( TargetSharedPointer ); }

    private:
        void Flush() noexcept;
        bool Dispatch( IAODSharedObjectTask* InTask ) noexcept;
        bool DelayTask( IAODSharedObjectTask* InTask ) noexcept;

        void* TargetSharedPointer{ nullptr }; //!< Cached pointer to base the shared memory policy off of

        friend IAODSharedObjectTask;
        friend class WorkerGroup;
    };
}

//StaticObject
namespace SKL::AOD
{
    struct StaticObject : public Object
    {
        StaticObject() noexcept = default;
        ~StaticObject() noexcept = default;

        //! Execute the functor thread safe relative to the object [void() noexcept]
        //! \returns RAllocationFailed if allocating the task object failed
        //! \returns RExecutedSync if the functor was dispatched sync (in this call)
        //! \returns RSuccess if the functor will be dispatched async
        template<typename TFunctor>
        SKL_FORCEINLINE SKL_NODISCARD RStatus DoAsync( TFunctor&& InFunctor ) noexcept
        {
            using TaskType = AODStaticObjectTask<sizeof(TFunctor)>;
            
            TaskType* NewTask{ MakeSharedRaw<TaskType>() };
            if( nullptr == NewTask ) SKL_UNLIKELY
            {   
                SKLL_ERR( "StaticObject::DoAsync() Failed to allocate task!" );
                return RAllocationFailed;
            }

            NewTask->SetDispatch( std::forward<TFunctor>( InFunctor ) );

            if( Dispatch( NewTask ) )
            {
                return RExecutedSync;
            }
        
            return RSuccess;
        }

        //! Execute the functor after [AfterMilliseconds], thread safe relative to the object [void() noexcept]
        //! \returns RAllocationFailed if allocating the task object failed
        //! \returns RFail if scheduling the task failed
        //! \returns RSuccess if the functor will be dispatched async
        template<typename TFunctor>
        SKL_FORCEINLINE SKL_NODISCARD RStatus DoAsyncAfter( TDuration AfterMilliseconds, TFunctor&& InFunctor ) noexcept
        {
            using TaskType = AODStaticObjectTask<sizeof(TFunctor)>;
            
            TaskType* NewTask{ MakeSharedRaw<TaskType>() };
            if( nullptr == NewTask ) SKL_UNLIKELY
            {   
                SKLL_ERR( "StaticObject::DoAsyncAfter() Failed to allocate task!" );
                return RAllocationFailed;
            }

            NewTask->SetParent( this );
            NewTask->SetDue( AfterMilliseconds );
            NewTask->SetDispatch( std::forward<TFunctor>( InFunctor ) );

            if ( false == DelayTask( NewTask ) ) SKL_UNLIKELY
            {
                return RFail;
            }

            return RSuccess;
        }

    private:
        void Flush() noexcept;
        bool Dispatch( IAODStaticObjectTask* InTask ) noexcept;
        bool DelayTask( IAODStaticObjectTask* InTask ) noexcept;

        friend class WorkerGroup;
    };
}

//CustomObject
namespace SKL::AOD
{
    // Custom AOD object
    // \remarks Expects that the memory right above it is the ControlBlock
    // \remarks Expects that it is part of an shared object with virtual deleter
    struct CustomObject : public Object
    {
        CustomObject() noexcept = default;
        ~CustomObject() noexcept = default;
        
        //! Execute the functor thread safe relative to the object [void( AOD::CustomObject& ) noexcept]
        //! \returns RAllocationFailed if allocating the task object failed
        //! \returns RExecutedSync if the functor was dispatched sync (in this call)
        //! \returns RSuccess if the functor will be dispatched async
        template<typename TFunctor>
        SKL_FORCEINLINE SKL_NODISCARD RStatus DoAsync( TFunctor&& InFunctor ) noexcept
        {
            using TaskType = AODCustomObjectTask<sizeof(TFunctor)>;
            
            TaskType* NewTask{ MakeSharedRaw<TaskType>() };
            if( nullptr == NewTask ) SKL_UNLIKELY
            {   
                SKLL_ERR( "CustomObject::DoAsync() Failed to allocate task!" );
                return RAllocationFailed;
            }

            NewTask->SetParent( this );
            NewTask->SetDispatch( std::forward<TFunctor>( InFunctor ) );

            if( Dispatch( NewTask ) )
            {
                return RExecutedSync;
            }
        
            return RSuccess;
        }

        //! Execute the functor after [AfterMilliseconds], thread safe relative to the object [void( AOD::CustomObject& ) noexcept]
        //! \returns RAllocationFailed if allocating the task object failed
        //! \returns RFail if scheduling the task failed
        //! \returns RSuccess if the functor will be dispatched async
        template<typename TFunctor>
        SKL_FORCEINLINE SKL_NODISCARD RStatus DoAsyncAfter( TDuration AfterMilliseconds, TFunctor&& InFunctor ) noexcept
        {
            SKLL_TRACE();
            using TaskType = AODCustomObjectTask<sizeof(TFunctor)>;
            
            TaskType* NewTask{ MakeSharedRaw<TaskType>() };
            if( nullptr == NewTask ) SKL_UNLIKELY
            {   
                SKLL_ERR( "CustomObject::DoAsyncAfter() Failed to allocate task!" );
                return RAllocationFailed;
            }

            NewTask->SetParent( this );
            NewTask->SetDue( AfterMilliseconds );
            NewTask->SetDispatch( std::forward<TFunctor>( InFunctor ) );

            if ( false == DelayTask( NewTask ) ) SKL_UNLIKELY
            {
                return RFail;
            }

            return RSuccess;
        }

    private:
        void Flush() noexcept;
        bool Dispatch( IAODCustomObjectTask* InTask ) noexcept;
        bool DelayTask( IAODCustomObjectTask* InTask ) noexcept;

        friend IAODCustomObjectTask;
        friend class WorkerGroup;
    };
}
