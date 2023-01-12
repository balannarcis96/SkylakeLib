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

            // nothing allocated
            AllocationsCount.exchange( 0U );
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
                if( static_cast<TUIDType>( AllocationsCount.increment() + 1 ) <= MaxUIDValue ) SKL_LIKELY
                {
                    SpinLockScopeGuard Guard{ IdsLock };
                    if( FALSE == FreeIds.empty() )
                    {
                        Result = FreeIds.top();
                        FreeIds.pop();
                    }
                }

                if( IdentityValue == Result ) SKL_UNLIKELY
                {
                    ( void )AllocationsCount.decrement();
                }
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
        std::relaxed_value<int32_t>                 bIsActive            { FALSE }; //!< is the cache active
        std::relaxed_value<int32_t>                 bHasCalledOnAllFreed { FALSE }; //!< has the OnAllFreed handler been called already
        std::relaxed_value<size_t>                  AllocationsCount     { 0 };     //!< no of allocated ids
        SpinLock                                    IdsLock              {};        //!< spin lock to guard the FreeIds stack
        std::stack<TUIDType, std::vector<TUIDType>> FreeIds              {};        //!< stack of free ids
        OnAllFreedTask                              OnAllFreed           {};        //!< functor to dispatch when all ids are freed and the store is not active
    };

    template<typename TUIDType>
    struct UIDAllocationCacheToIndexConvert final
    {
        SKL_NODISCARD SKL_FORCEINLINE static size_t ConvertToIndex( TUIDType Id ) noexcept
        {
            // We assume TUIDType its TEntityId based
            return Id.GetIndex();
        }
    };

    template<typename TUIDType, TUIDType IdentityValue, TUIDType MaxUIDValue, typename StaticToIndexConverter = UIDAllocationCacheToIndexConvert<TUIDType>>
    struct UIDAllocationCache
    {
        static constexpr uint64_t CUIDsCount{ MaxUIDValue + 2 }; //+1 (index 0) +1 (max value itself)
        using OnAllFreedTask = ASD::UniqueFunctorWrapper<32, void(SKL_CDECL*)()noexcept>;

        UIDAllocationCache() noexcept = default;
        ~UIDAllocationCache() noexcept = default;

        //! activate the cache
        void Activate() noexcept
        {
            // initialize all slots to "unallocated"
            for( uint64_t i = IdentityValue + 1; i < CUIDsCount; ++i )
            {
                UIDsAllocationStateCache[i].exchange( false );
            }

            // set active
            bIsActive.exchange( TRUE );
            
            // clear flag
            bHasCalledOnAllFreed.exchange( FALSE );

            // nothing allocated
            AllocationsCount.exchange( 0U );
        }
        
        //! deactivate the cache, when all ids are freed the callback is called
        void Deactivate() noexcept
        {
            bIsActive.exchange( FALSE );

            const bool bAllAreDeallocated{ 0 == AllocationsCount.load_acquire() };
            if( bAllAreDeallocated )
            {
                if( FALSE == bHasCalledOnAllFreed.exchange( TRUE ) )
                {
                    if( false == OnAllFreed.IsNull() ) SKL_LIKELY
                    {
                        OnAllFreed.Dispatch();
                    }
                }
            }
        }

        //! allocate id
        SKL_NODISCARD bool Allocate( TUIDType IdToAllocate ) noexcept
        {
            bool bResult{ false };

            if( TRUE == bIsActive ) SKL_LIKELY
            {
                if( static_cast<TUIDType>( AllocationsCount.increment() + 1 ) <= MaxUIDValue ) SKL_LIKELY
                {
                    const size_t Index{ StaticToIndexConverter::ConvertToIndex( IdToAllocate ) };

                    if( false == UIDsAllocationStateCache[Index].exchange( true ) ) SKL_LIKELY
                    {
                        // allocated
                        bResult = true;
                    }
                    else
                    {
                        ( void )AllocationsCount.decrement();
                        // already allocated
                    }
                }
                else
                {
                    ( void )AllocationsCount.decrement();
                }
            }

            return bResult;
        }
        
        //! deallocate id
        bool Deallocate( TUIDType InUID ) noexcept
        {
            const int64_t NewAllocationsCount { AllocationsCount.decrement() };
            if( NewAllocationsCount <= 0 ) SKL_UNLIKELY
            {
                // all slots deallocated already ?!?!
                ( void )AllocationsCount.increment();
                return false;
            }

            bool       bSuccess       { false };
            const bool bDeallocatedAll{ 1 == NewAllocationsCount };

            const size_t Index{  StaticToIndexConverter::ConvertToIndex( InUID ) };
            if( true == UIDsAllocationStateCache[Index].exchange( false ) ) SKL_LIKELY
            {
                // successfully deallocated
                bSuccess = true;
            }
            else
            {
                // id already deallocated ?!
            }

            if( bSuccess && bDeallocatedAll && FALSE == bIsActive.load() )
            {
                if( FALSE == bHasCalledOnAllFreed.exchange( TRUE ) )
                {
                    if( false == OnAllFreed.IsNull() )
                    {
                        OnAllFreed.Dispatch();
                    }
                }
            }

            return bSuccess;
        }

        //! set the functor to be dispatched when all ids are deallocated and the store is not active
        template<typename TFunctor>
        SKL_FORCEINLINE void SetOnAllFreed( TFunctor&& InFunctor ) noexcept
        {
            OnAllFreed += std::forward<TFunctor>( InFunctor );
        }

        //! is the cache active
        SKL_FORCEINLINE SKL_NODISCARD bool IsActive() const noexcept{ return TRUE == bIsActive.load_relaxed(); }
        
        //! is ready to be destroyed
        SKL_FORCEINLINE SKL_NODISCARD bool IsShutdownAndReadyToDestroy() const noexcept{ return ( FALSE == bIsActive.load_relaxed() ) && ( TRUE == bHasCalledOnAllFreed.load_relaxed() ); }

        //! get the count of allocated ids
        SKL_FORCEINLINE SKL_NODISCARD size_t GetAllocatedIdsCount() const noexcept { return AllocationsCount.load_relaxed(); }

    private:
        std::relaxed_value<int32_t> bIsActive                           { FALSE }; //!< is the cache active
        std::relaxed_value<int32_t> bHasCalledOnAllFreed                { FALSE }; //!< has the OnAllFreed handler been called already
        std::relaxed_value<int64_t> AllocationsCount                    { 0 };     //!< no of allocated ids
        std::synced_value<bool>     UIDsAllocationStateCache[CUIDsCount]{};        //!< all allocable ids
        OnAllFreedTask              OnAllFreed                          {};        //!< functor to dispatch when all ids are freed and the store is not active
    };
}