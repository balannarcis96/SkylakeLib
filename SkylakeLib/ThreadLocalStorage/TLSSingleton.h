//!
//! \file TLSSingleton.h
//! 
//! \brief TLS singleton abstraction for the SkylakeLib
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    template< typename TUpper >
    struct ITLSSingleton
    {
        using TLSSingletonType = TLSValue< TUpper >;

        ITLSSingleton( ) noexcept          = default;
        virtual ~ITLSSingleton( ) noexcept = default;

        ITLSSingleton( const ITLSSingleton & ) noexcept = default;
        ITLSSingleton &operator=( const ITLSSingleton & ) noexcept = default;
        ITLSSingleton( ITLSSingleton && ) noexcept = default;
        ITLSSingleton &operator=( ITLSSingleton && ) noexcept = default;

        static RStatus Create( ) noexcept
        {
            auto *NewObject = new TUpper( );
            if( !NewObject )
            {
                return RStatus::AllocationFailed;
            }

            if( const auto InitializeResult = NewObject->Initialize( ); InitializeResult != RSuccess )
            {
                return InitializeResult;
            }

            SKL_ASSERT( TLSSingletonType::GetValuePtr( ) == nullptr );
            TLSSingletonType::SetValuePtr( NewObject );

            return RSuccess;
        }

        /**
         * \brief Get thread local instance
         */
        SKL_FORCEINLINE static TUpper *GetInstance( ) noexcept
        {
            return TLSSingletonType::GetValuePtr( );
        }

        static void Destroy( ) noexcept
        {
            if( const auto *Instance = TLSSingletonType::GetValuePtr( ) )
            {
                delete Instance;
            }

            TLSSingletonType::SetValuePtr( nullptr );
        }

        virtual RStatus Initialize( ) noexcept = 0;

        virtual const char *GetName( ) const noexcept = 0;
    };
}