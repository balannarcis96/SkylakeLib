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
                SKL_ERR_FMT( "MakeUniqueArray<T>() Cannot alloc more than %llu. Attempted %llu!", CMemoryManager_MaxAllocSize, AllocSize );
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
                SKL_ERR_FMT( "MakeUniqueArray<T>() Cannot alloc more than %llu. Attempted %llu!", CMemoryManager_MaxAllocSize, AllocSize );
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
                SKL_ERR_FMT( "MakeSharedArrayRaw<T>() Cannot alloc more than %llu. Attempted %llu!", CMemoryManager_MaxAllocSize, AllocSize );
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

namespace SKL
{
    template<typename T>
    class STLAllocator
    {
    public:
        static_assert( false == std::is_const_v<T>, "The C++ Standard forbids containers of const elements "
                                                       "because allocator<const T> is ill-formed." );

        using value_type      = std::conditional_t<std::is_array_v<T>, std::remove_all_extents_t<T>, T>;
        using size_type       = uint32_t;
        using difference_type = ptrdiff_t;
        using pointer         = T*;
        using const_pointer   = const T*;
        using reference       = T&;
        using const_reference = const T&;

        using propagate_on_container_move_assignment           = std::true_type;
        using is_always_equal _CXX20_DEPRECATE_IS_ALWAYS_EQUAL = std::true_type;

        constexpr STLAllocator() noexcept {}
        constexpr STLAllocator( const STLAllocator& ) noexcept = default;
        template <class _Other>
        constexpr STLAllocator( const STLAllocator<_Other>& ) noexcept {}

        constexpr ~STLAllocator() = default;
        constexpr STLAllocator& operator=( const STLAllocator& ) = default;

        constexpr void deallocate( T* const InPtr, const size_t InCount ) noexcept
        {
            SKL_ASSERT_MSG( InPtr != nullptr || InCount == 0, "null pointer cannot point to a block of non-zero size");
            constexpr uint32_t TSize{ static_cast<uint32_t>( sizeof( T ) ) };
            // no overflow check on the following multiply; we assume _Allocate did that check
            const uint32_t AllocateSize{ TSize * static_cast<uint32_t>( InCount ) };
            GlobalMemoryManager::Deallocate( InPtr, AllocateSize );
        }

        SKL_NODISCARD constexpr SKL_ALLOCATOR_FUNCTION T* allocate( const size_t InCount ) noexcept 
        {
            static_assert( sizeof(value_type) > 0, "value_type must be complete before calling allocate." );

            constexpr uint32_t TSize{ static_cast<uint32_t>( sizeof( T ) ) };
#if !defined(SKL_BUILD_SHIPPING)
            constexpr bool bOverflowIsPossible{ TSize > 1 };
            if constexpr( true == bOverflowIsPossible ) 
            {
                constexpr uint32_t MaxPossible{ static_cast<uint32_t>( -1 ) / TSize };
                SKL_ASSERT_ALLWAYS_MSG( static_cast<uint32_t>( InCount ) <= MaxPossible, "STLAllocator<T>::allocate() multiply overflow" );
            }
#endif
            const uint32_t AllocateSize{ TSize * static_cast<uint32_t>( InCount ) };
            auto AllocResult{ GlobalMemoryManager::Allocate( AllocateSize ) };
            if( false == AllocResult.IsValid() ) SKL_UNLIKELY
            {
                SKL_WRN_FMT( "STLAllocator<T>::Allocate() Failed to allocate %u bytes (%llu items)", AllocateSize, InCount );
                return nullptr;
            }
            
            return reinterpret_cast<T*>( AllocResult.MemoryBlock );
        }
    };

    template<typename T>
    class STLTLSAllocator
    {
    public:
        static_assert( false == std::is_const_v<T>, "The C++ Standard forbids containers of const elements "
                                                       "because allocator<const T> is ill-formed." );

        using value_type      = std::conditional_t<std::is_array_v<T>, std::remove_all_extents_t<T>, T>;
        using size_type       = uint32_t;
        using difference_type = ptrdiff_t;
        using pointer         = T*;
        using const_pointer   = const T*;
        using reference       = T&;
        using const_reference = const T&;

        using propagate_on_container_move_assignment           = std::true_type;
        using is_always_equal _CXX20_DEPRECATE_IS_ALWAYS_EQUAL = std::true_type;

        constexpr STLTLSAllocator() noexcept {}
        constexpr STLTLSAllocator( const STLTLSAllocator& ) noexcept = default;
        template <class _Other>
        constexpr STLTLSAllocator( const STLTLSAllocator<_Other>& ) noexcept {}

        constexpr ~STLTLSAllocator() = default;
        constexpr STLTLSAllocator& operator=( const STLTLSAllocator& ) = default;

        constexpr void deallocate( T* const InPtr, const size_t InCount ) noexcept
        {
            SKL_ASSERT_MSG( InPtr != nullptr || InCount == 0, "null pointer cannot point to a block of non-zero size");
            constexpr uint32_t TSize{ static_cast<uint32_t>( sizeof( T ) ) };
            // no overflow check on the following multiply; we assume _Allocate did that check
            const uint32_t AllocateSize{ TSize * static_cast<uint32_t>( InCount ) };
            ThreadLocalMemoryManager::Deallocate( InPtr, AllocateSize );
        }

        SKL_NODISCARD constexpr SKL_ALLOCATOR_FUNCTION T* allocate( const size_t InCount ) noexcept 
        {
            static_assert( sizeof(value_type) > 0, "value_type must be complete before calling allocate." );

            constexpr uint32_t TSize{ static_cast<uint32_t>( sizeof( T ) ) };
#if !defined(SKL_BUILD_SHIPPING)
            constexpr bool bOverflowIsPossible{ TSize > 1 };
            if constexpr( true == bOverflowIsPossible ) 
            {
                constexpr uint32_t MaxPossible{ static_cast<uint32_t>( -1 ) / TSize };
                SKL_ASSERT_ALLWAYS_MSG( static_cast<uint32_t>( InCount ) <= MaxPossible, "STLTLSAllocator<T>::allocate() multiply overflow" );
            }
#endif
            const uint32_t AllocateSize{ TSize * static_cast<uint32_t>( InCount ) };
            auto AllocResult{ ThreadLocalMemoryManager::Allocate( AllocateSize ) };
            if( false == AllocResult.IsValid() ) SKL_UNLIKELY
            {
                SKL_WRN_FMT( "STLTLSAllocator<T>::Allocate() Failed to allocate %u bytes (%llu items)", AllocateSize, InCount );
                return nullptr;
            }
            
            return reinterpret_cast<T*>( AllocResult.MemoryBlock );
        }
    };
}

namespace SKL
{
    //! Vector based, GlobalMemoryManager managed stl priority queue
    template<typename T, typename Comparator = std::greater<T>>
    using ManagedPriorityQueue = std::priority_queue<T, std::vector<T, STLAllocator<T>>, Comparator>;


    //! Vector based, ThreadLocalMemoryManager managed stl priority queue
    template<typename T, typename Comparator = std::greater<T>>
    using TLSManagedPriorityQueue = std::priority_queue<T, std::vector<T, STLTLSAllocator<T>>, Comparator>;

    //! GlobalMemoryManager stl deque
    template<typename T>
    using ManagedDeque = std::deque<T, STLAllocator<T>>;

    //! ThreadLocalMemoryManager stl deque
    template<typename T>
    using TLSManagedDeque = std::deque<T, STLTLSAllocator<T>>;

    //! GlobalMemoryManager stl queue
    template<typename T>
    using ManagedQueue = std::queue<T, std::deque<T, STLAllocator<T>>>;

    //! ThreadLocalMemoryManager stl queue
    template<typename T>
    using TLSManagedQueue = std::queue<T, std::deque<T, STLTLSAllocator<T>>>;
}