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

        //! activate the store
        void Activate() noexcept
        {
            // clear the stack
            while( false == FreeIds.empty() ){ FreeIds.pop(); }

            // populate the stack with all possible values
            for( TUIDType i = MaxUIDValue; i > IdentityValue; --i )
            {
                SKL_ASSERT( i != IdentityValue );
                FreeIds.push( i );            
            }

            // set active
            bIsActive.exchange( TRUE );
            
            // clear flag
            bHasCalledOnAllFreed.exchange( FALSE );
        }
        
        //! deactivate the store, when all ids are freed the callback is called
        void Deactivate() noexcept
        {
            bIsActive.exchange( FALSE );

            bool bAllAreDeallocated{ false };
            
            {
                SpinLockScopeGuard Guard{ IdsLock };
                bAllAreDeallocated = FreeIds.size() == MaxUIDValue;           
            }
            
            if( true == bAllAreDeallocated )
            {
                if( FALSE == bHasCalledOnAllFreed.exchange( TRUE ) )
                {
                    if( false == OnAllFreed.IsNull() )
                    {
                       OnAllFreed.Dispatch();
                    }
                }
            }
        }

        //! allocate new id
        SKL_NODISCARD TUIDType Allocate() noexcept
        {
            TUIDType Result{ IdentityValue };

            if( TRUE == bIsActive ) SKL_LIKELY
            {
                SpinLockScopeGuard Guard{ IdsLock };
                if( FALSE == FreeIds.empty() )
                {
                    Result = FreeIds.top();
                    FreeIds.pop();
                }
            }

            if( IdentityValue != Result ) SKL_LIKELY
            {
                AllocationsCount.increment();
            }

            return Result;
        }
        
        //! deallocate id
        void Deallocate( TUIDType InUID ) noexcept
        {
            bool bDeallocatedAll{ false };

            {
                SpinLockScopeGuard Guard{ IdsLock };
                FreeIds.push( InUID );

                bDeallocatedAll = FreeIds.size() == MaxUIDValue;
            }

            AllocationsCount.decrement();

            if( ( FALSE == bIsActive.load_relaxed() ) && ( true == bDeallocatedAll ) )
            {
                if( FALSE == bHasCalledOnAllFreed.exchange( TRUE ) )
                {
                    if( false == OnAllFreed.IsNull() )
                    {
                       OnAllFreed.Dispatch();
                    }
                }
            }
        }

        //! set the functor to be dispatched when all ids are deallocated and the store is not active
        template<typename TFunctor>
        SKL_FORCEINLINE void SetOnAllFreed( TFunctor&& InFunctor ) noexcept
        {
            OnAllFreed += std::forward<TFunctor>( InFunctor );
        }

        //! is the store active
        SKL_FORCEINLINE SKL_NODISCARD bool IsActive() const noexcept{ return TRUE == bIsActive.load_relaxed(); }
        
        //! is ready to be destroyed
        SKL_FORCEINLINE SKL_NODISCARD bool IsShutdownAndReadyToDestroy() const noexcept{ return ( FALSE == bIsActive.load_relaxed() ) && ( TRUE == bHasCalledOnAllFreed.load_relaxed() ); }

        //! get the count of allocated ids
        SKL_FORCEINLINE SKL_NODISCARD size_t GetAllocatedIdsCount() const noexcept { return AllocationsCount.load_relaxed(); }

    private:
        std::relaxed_value<int32_t>                 bIsActive            { FALSE }; //!< is the store active
        std::relaxed_value<int32_t>                 bHasCalledOnAllFreed { FALSE }; //!< is the store active
        std::relaxed_value<size_t>                  AllocationsCount     { 0 };     //!< is the store active
        SpinLock                                    IdsLock              {};        //!< spin lock to guard the FreeIds stack
        std::stack<TUIDType, std::vector<TUIDType>> FreeIds              {};        //!< stack of free ids
        OnAllFreedTask                              OnAllFreed           {};        //!< functor to dispatch when all ids are freed and the store is not active
    };
}