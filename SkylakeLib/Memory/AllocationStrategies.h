//!
//! \file AllocationStrategies.h
//! 
//! \brief Global allocation strategies through the MemoryManager
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    template<typename T>
    using skl_unique_ptr = std::unique_ptr<T, typename SKL::MemoryStrategy::UniqueMemoryStrategy<T>::DestructDeallocator>;

    template<typename T>
    using skl_unique_nd_ptr = std::unique_ptr<T, typename SKL::MemoryStrategy::UniqueMemoryStrategy<T>::Deallocator>;

    //! 
    //! Allocate new unique object (raw ptr) through the MemoryManager
    //! 
    //! \tparam bConstruct if true the item will be constructed not otherwise
    //! 
    //! \remark if bConstruct is false don't pass any construction arguments
    //! 
    template<typename TObject, typename ...TArgs>
    skl_unique_ptr<TObject> MakeUnique( TArgs... Args ) noexcept 
    {
        static_assert( false == std::is_array_v<TObject>, "Please use MakeArrayUnique()" ); 
        static_assert( 0 == SKL_GUARD_ALLOC_SIZE_ON || sizeof( TObject ) < CMemoryManager_MaxAllocSize, "Cannot alloc this much memory at once!" );
        using Allocator = typename SKL::MemoryStrategy::UniqueMemoryStrategy<TObject>::Allocator;
        return skl_unique_ptr<TObject>( Allocator::template AllocateObject<true, false>( std::forward<TArgs>( Args )... ) );
    }

    //! 
    //! Allocate new unique object through the MemoryManager
    //! 
    //! \tparam bConstruct if true the item will be constructed not otherwise
    //! 
    //! \remark if bConstruct is false don't pass any construction arguments
    //! 
    //! \remark the object will not be destructed
    //! 
    template<typename TObject, bool bConstruct = false, typename ...TArgs>
    skl_unique_nd_ptr<TObject> MakeUniqueNoDeconstruct( TArgs... Args ) noexcept 
    {
        static_assert( false == std::is_array_v<TObject>, "Please use MakeArrayUnique()" ); 
        static_assert( 0 == SKL_GUARD_ALLOC_SIZE_ON || sizeof( TObject ) < CMemoryManager_MaxAllocSize, "Cannot alloc this much memory at once!" );
        using Allocator = typename SKL::MemoryStrategy::UniqueMemoryStrategy<TObject>::Allocator;
        return skl_unique_nd_ptr<TObject>{ Allocator::template AllocateObject<bConstruct, false>( std::forward<TArgs>( Args )... ) };
    }
    
    //! 
    //! Allocate new unique array through the MemoryManager
    //! 
    //! \tparam bConstructAllItems if true and TItemType is class type all items in the array will be default constructed
    //! 
    template<typename TItemType>
    SKL_FORCEINLINE skl_unique_ptr<TItemType[]> MakeUniqueArray( uint32_t ItemCount ) noexcept
    {
        static_assert( false == std::is_array_v<TItemType>, "Can't allocate array of arrays!" );
        using Allocator = typename SKL::MemoryStrategy::UniqueMemoryStrategy<TItemType[]>::Allocator;

        if constexpr( SKL_GUARD_ALLOC_SIZE_ON )
        {
            const size_t AllocSize{ Allocator::template CalculateNeededSizeForArray<TItemType>( ItemCount ) };
            if( AllocSize > CMemoryManager_MaxAllocSize ) SKL_UNLIKELY
            {
                SKLL_ERR_FMT( "MakeUniqueArray<T>() Cannot alloc more than %llu. Attempted %llu!", CMemoryManager_MaxAllocSize, AllocSize );
                return skl_unique_ptr<TItemType[]>();
            }
        }

        return skl_unique_ptr<TItemType[]>{ Allocator::template AllocateArray<true, false>( ItemCount ) };
    }
     
    //! 
    //! Allocate new unique array through the MemoryManager
    //! 
    //! \tparam bConstructAllItems if true and TItemType is class type all items in the array will be default constructed not otherwise
    //! 
    //! \remark if true and TItemType is class type all items will not be destructed
    //! 
    template<typename TItemType, bool bConstructAllItems = false>
    skl_unique_nd_ptr<TItemType[]> MakeUniqueArrayWithNoDestruct( uint32_t ItemCount ) noexcept
    {
        static_assert( false == std::is_array_v<TItemType>, "Can't allocate array of arrays!" );
        using Allocator = typename SKL::MemoryStrategy::UniqueMemoryStrategy<TItemType[]>::Allocator;

        if constexpr( SKL_GUARD_ALLOC_SIZE_ON )
        {
            const size_t AllocSize{ Allocator::template CalculateNeededSizeForArray<TItemType>( ItemCount ) };
            if( AllocSize > CMemoryManager_MaxAllocSize ) SKL_UNLIKELY
            {
                SKLL_ERR_FMT( "MakeUniqueArray<T>() Cannot alloc more than %llu. Attempted %llu!", CMemoryManager_MaxAllocSize, AllocSize );
                return skl_unique_nd_ptr<TItemType[]>();
            }
        }

        return skl_unique_nd_ptr<TItemType[]>{ Allocator::template AllocateArray<bConstructAllItems, false>( ItemCount ) };
    }
}

namespace SKL
{
    //! 
    //! Allocate new shared object (raw ptr) through the MemoryManager
    //! 
    //! \tparam bConstruct if true the item will be constructed not otherwise
    //! 
    //! \remark if bConstruct is false don't pass any construction arguments
    //! 
    template<typename TObject, bool bConstruct = true, typename ...TArgs>
    SKL_FORCEINLINE TObject* MakeSharedRaw( TArgs... Args ) noexcept 
    {
        static_assert( false == std::is_array_v<TObject>, "Please use MakeSharedArrayRaw()" ); 
        static_assert( 0 == SKL_GUARD_ALLOC_SIZE_ON || sizeof( TObject ) < CMemoryManager_MaxAllocSize, "Cannot alloc this much memory at once!" );
        using Allocator = typename SKL::MemoryStrategy::SharedMemoryStrategy<TObject>::Allocator;
        return Allocator::template AllocateObject<bConstruct, false>( std::forward<TArgs>( Args )... );
    }

    //! Allocate new shared object through the MemoryManager
    template<typename TObject, typename ...TArgs>
    SKL_FORCEINLINE TSharedPtr<TObject> MakeShared( TArgs... Args ) noexcept 
    {
        static_assert( false == std::is_array_v<TObject>, "Please use MakeSharedArray()" ); 
        return { MakeSharedRaw<TObject, true>( std::forward<TArgs>( Args )... ) };
    }

    //! 
    //! Allocate new shared object through the MemoryManager
    //! 
    //! \tparam bConstruct if true the item will be constructed not otherwise
    //! 
    //! \remark if bConstruct is false don't pass any construction arguments
    //! 
    //! \remark the object will not be deconstructed
    //! 
    template<typename TObject, bool bConstruct = false, typename ...TArgs>
    SKL_FORCEINLINE TSharedPtrNoDestruct<TObject> MakeSharedNoDestruct( TArgs... Args ) noexcept 
    {
        static_assert( false == std::is_array_v<TObject>, "Please use MakeSharedArray()" ); 
        return { MakeSharedRaw<TObject, bConstruct>( std::forward<TArgs>( Args )... ) };
    }

    //! 
    //! Allocate new shared array (raw ptr) through the MemoryManager
    //! 
    //! \tparam bConstructAllItems if true all array items will be default constructed (if TItemType is class type)
    //! 
    template<typename TItemType, bool bConstructAllItems = true>
    SKL_FORCEINLINE TItemType* MakeSharedArrayRaw( uint32_t ItemCount ) noexcept
    {
        static_assert( false == std::is_array_v<TItemType>, "Cannot allocate array of arrays" ); 
        using Allocator = typename SKL::MemoryStrategy::SharedMemoryStrategy<TItemType[]>::Allocator;
        
        if constexpr( SKL_GUARD_ALLOC_SIZE_ON )
        {
            const size_t AllocSize{ Allocator::template CalculateNeededSizeForArray<TItemType>( ItemCount ) };
            if( AllocSize > CMemoryManager_MaxAllocSize ) SKL_UNLIKELY
            {
                SKLL_ERR_FMT( "MakeSharedArrayRaw<T>() Cannot alloc more than %llu. Attempted %llu!", CMemoryManager_MaxAllocSize, AllocSize );
                return nullptr;
            }
        }

        return Allocator::template AllocateArray<bConstructAllItems, false>( ItemCount );
    }

    //! Allocate new shared array through the MemoryManager
    template<typename TItemType>
    SKL_FORCEINLINE TSharedPtr<TItemType[]> MakeSharedArray( uint32_t ItemCount ) noexcept 
    {
        return { MakeSharedArrayRaw<TItemType, true>( ItemCount ) };
    }

    //! 
    //! Allocate new shared array through the MemoryManager
    //! 
    //! \tparam bConstructAllItems if true all array items will be default constructed (if TItemType is class type)
    //! 
    //! \remark if TItemType is class type, all items in the array will not be deconstructed
    //! 
    template<typename TItemType, bool bConstructAllItems = false>
    SKL_FORCEINLINE TSharedPtrNoDestruct<TItemType[]> MakeSharedArrayNoDestruct( uint32_t ItemCount ) noexcept 
    {
        return { MakeSharedArrayRaw<TItemType, bConstructAllItems>( ItemCount ) };
    }
}
