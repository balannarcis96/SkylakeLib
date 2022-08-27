//!
//! \file UIDStore.h
//! 
//! \brief Thread safe UID store
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    template<typename TUIDType, TUIDType IdentityValue, TUIDType MaxUIDValue>
    struct UIDStore
    {
        using OnAllFreedTask = ASD::UniqueFunctorWrapper<32, void(SKL_CDECL*)()noexcept>;

        UIDStore() noexcept = default;
        ~UIDStore() noexcept = default;

        void Activate() noexcept
        {
            for( TUIDType i = MaxUIDValue; i > IdentityValue; --i )
            {
                SKL_ASSERT( i != IdentityValue );
                FreeIndices.push( i );            
            }

            bIsActive.exchange( TRUE );
        }
        void Deactivate() noexcept
        {
            bIsActive.exchange( FALSE );
        }

        TUIDType Allocate() noexcept
        {
            TUIDType Result{ IdentityValue };

            if( TRUE == bIsActive )
            {
                SpinLockScopeGuard Guard{ IndicesLock };
                if( FALSE == FreeIndices.empty() )
                {
                    Result = FreeIndices.top();
                    FreeIndices.pop();
                }
            }

            return Result;
        }
        void Deallocate( TUIDType InUID ) noexcept
        {
            bool bDeallocatedAll{ false };

            {
                SpinLockScopeGuard Guard{ IndicesLock };
                FreeIndices.push( InUID );

                bDeallocatedAll = FreeIndices.size() == MaxUIDValue;
            }

            if( FALSE == bIsActive.load_relaxed() && true == bDeallocatedAll && false == OnAllFreed.IsNull() )
            {
                OnAllFreed.Dispatch();
            }
        }

        std::span<TUIDType> GetView() const noexcept
        {
            SKL_ASSERT( FALSE == bIsActive.load_relaxed() );
            return { FreeIndices };
        }

        template<typename TFunctor>
        void SetOnAllFreed( TFunctor&& InFunctor ) noexcept
        {
            OnAllFreed += std::forward<TFunctor>( InFunctor );
        }

    private:
        std::relaxed_value<int32_t>                 bIsActive{ FALSE };
        SpinLock                                    IndicesLock{};
        std::stack<TUIDType, std::vector<TUIDType>> FreeIndices{};
        OnAllFreedTask                              OnAllFreed;
    };
}