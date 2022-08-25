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

        //! Execute the functor thread safe relative to the object [void(AODObject&)noexcept]
        template<typename TFunctor>
        SKL_FORCEINLINE RStatus DoAsync( TFunctor&& InFunctor ) noexcept
        {
            using TaskType = AODSharedObjectTask<sizeof(TFunctor)>;
            
            TaskType* NewTask{ MakeSharedRaw<TaskType>() };
            if( nullptr == NewTask ) SKL_UNLIKELY
            {   
                SKL_ERR( "SharedObject::DoAsync() Failed to allocate task!" );
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

        //! Execute the functor after [AfterMilliseconds], thread safe relative to the object [void(AODObject&)noexcept]
        template<typename TFunctor>
        SKL_FORCEINLINE RStatus DoAsyncAfter( TDuration AfterMilliseconds, TFunctor&& InFunctor ) noexcept
        {
            using TaskType = AODSharedObjectTask<sizeof(TFunctor)>;
            
            TaskType* NewTask{ MakeSharedRaw<TaskType>() };
            if( nullptr == NewTask ) SKL_UNLIKELY
            {   
                SKL_ERR( "SharedObject::DoAsyncAfter() Failed to allocate task!" );
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

        //! Get the cached pointe to the parent instance
        void* GetParentObjectPointer() const noexcept { return TargetSharedPointer; }

        //! Get the cached pointe to the parent instance
        template<typename T>
        T& GetParentObject() const noexcept { return *reinterpret_cast<T*>( TargetSharedPointer ); }

    private:
        void Flush() noexcept;
        bool Dispatch( IAODSharedObjectTask* InTask ) noexcept;
        bool DelayTask( IAODSharedObjectTask* InTask ) noexcept;

        void* TargetSharedPointer{ nullptr }; //!< Cached pointer to base the shared memory policy off of

        friend IAODSharedObjectTask;
    };
}

//StaticObject
namespace SKL::AOD
{
    struct StaticObject : public Object
    {
        StaticObject() noexcept = default;
        ~StaticObject() noexcept = default;

        //! Execute the functor thread safe relative to the object [void(AODObject&)noexcept]
        template<typename TFunctor>
        SKL_FORCEINLINE RStatus DoAsync( TFunctor&& InFunctor ) noexcept
        {
            using TaskType = AODStaticObjectTask<sizeof(TFunctor)>;
            
            TaskType* NewTask{ MakeSharedRaw<TaskType>() };
            if( nullptr == NewTask ) SKL_UNLIKELY
            {   
                SKL_ERR( "StaticObject::DoAsync() Failed to allocate task!" );
                return RAllocationFailed;
            }

            NewTask->SetDispatch( std::forward<TFunctor>( InFunctor ) );

            if( Dispatch( NewTask ) )
            {
                return RExecutedSync;
            }
        
            return RSuccess;
        }

        //! Execute the functor after [AfterMilliseconds], thread safe relative to the object [void(AODObject&)noexcept]
        template<typename TFunctor>
        SKL_FORCEINLINE RStatus DoAsyncAfter( TDuration AfterMilliseconds, TFunctor&& InFunctor ) noexcept
        {
            using TaskType = AODStaticObjectTask<sizeof(TFunctor)>;
            
            TaskType* NewTask{ MakeSharedRaw<TaskType>() };
            if( nullptr == NewTask ) SKL_UNLIKELY
            {   
                SKL_ERR( "StaticObject::DoAsyncAfter() Failed to allocate task!" );
                return RAllocationFailed;
            }

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
    };
}
