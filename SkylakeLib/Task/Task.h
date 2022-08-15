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
    using ITask = ASD::ITask;

    template<size_t TaskSize>
    using Task  = ASD::Task<TaskSize>;
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
        return NewTask.CastMoveTo<ITask>();
    }

    template<typename TFunctor>
    TSharedPtr<ITask> MakeTask( TFunctor&& InFunctor ) noexcept
    {
        return { MakeTaskRaw( std::forward<TFunctor>( InFunctor ) ) };
    }
}