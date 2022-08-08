//! Forward ASD main types
namespace SKL
{
    //!
    //! Simple function pointer wrapper
    //!
    //! \details Usage: FnPtr<void(*)( int )> ptr = DoSmth;
    //! \details sizeof( FnPtr< ... > ) == sizeof( void * )
    //!
    template<typename TFnPtrSignature>
    using FnPtr = ASD::FnPtr<TFnPtrSignature>;

    //!
    //! Simple class function pointer(method) wrapper
    //!
    //! \details Usage: MethodPtr<void(TClass::*)( int )> ptr = &TClass::DoSmth; ptr( Instance, ... )
    //! \details sizeof( MethodPtr< ... > ) == sizeof( void * )
    //!
    template<typename TFnPtrSignature>
    using MethodPtr = ASD::MethodPtr<TFnPtrSignature>;

    //!
    //! Base class for all delegate types
    //!
    //! \details See RawDelegate< ... >, UniqueDelegate< ... >, SharedDelegate< ... >
    //! \details sizeof( DelegateBase< ... > ) == sizeof( void * ) * 2
    //!
    template<template<typename ...> typename TPointerWrapper, typename TFnPtrSignature>
    using DelegateBase = ASD::DelegateBase<TPointerWrapper, TFnPtrSignature>;
}