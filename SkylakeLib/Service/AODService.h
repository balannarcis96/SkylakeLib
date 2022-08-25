//!
//! \file AODService.h
//! 
//! \brief AOD intefaced service abstraction
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    class AODService : public IService
    {
    public:
        AODService( uint32_t UID ) noexcept : IService{ UID } {}

        //! Execute the functor thread safe relative to the object [void(AODObject&)noexcept]
        template<typename TFunctor>
        SKL_FORCEINLINE RStatus DoAsync( TFunctor&& InFunctor ) noexcept
        {
            return AODObjectInterface.DoAsync( std::forward<TFunctor>( InFunctor ) );
        }

        //! Execute the functor after [AfterMilliseconds], thread safe relative to the object [void(AODObject&)noexcept]
        template<typename TFunctor>
        SKL_FORCEINLINE RStatus DoAsyncAfter( TDuration AfterMilliseconds, TFunctor&& InFunctor ) noexcept
        {
            return AODObjectInterface.DoAsync( AfterMilliseconds, std::forward<TFunctor>( InFunctor ) );
        }

    protected:
        alignas(SKL_CACHE_LINE_SIZE) AOD::StaticObject AODObjectInterface{};

        friend ServerInstance;
    };
}    