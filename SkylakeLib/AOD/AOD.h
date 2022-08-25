//!
//! \file AOD.h
//! 
//! \brief Async Object bound Dispatcher abstraction for SkylakeLib
//! 
//! \reference https://github.com/balannarcis96/Dispatcher (G.O.D: Grand Object-bound Dispatcher)
//!
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

#include "AOD_Object.h"

namespace SKL
{
    struct alignas( SKL_ALIGNMENT ) EmbededAODObject
    {
        //! Execute the functor thread safe relative to the object [void(AODObject&)noexcept]
        template<typename TFunctor>
        SKL_FORCEINLINE RStatus DoAsync(TFunctor&& InFunctor) noexcept
        {
            return AODObjectInterface.DoAsync(std::forward<TFunctor>(InFunctor));
        }

        //! Execute the functor after [AfterMilliseconds], thread safe relative to the object [void(AODObject&)noexcept]
        template<typename TFunctor>
        SKL_FORCEINLINE RStatus DoAsyncAfter(TDuration AfterMilliseconds, TFunctor&& InFunctor) noexcept
        {
            return AODObjectInterface.DoAsync(AfterMilliseconds, std::forward<TFunctor>(InFunctor));
        }

    protected:
        MemoryPolicy::ControlBlock CB { 1, 0 };
        AODObject AODObjectInterface {};
    };
}
