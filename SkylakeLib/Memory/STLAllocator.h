//!
//! \file STLAllocator.h
//! 
//! \brief STL managed types
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

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
        //using is_always_equal _CXX20_DEPRECATE_IS_ALWAYS_EQUAL = std::true_type;

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
        //using is_always_equal _CXX20_DEPRECATE_IS_ALWAYS_EQUAL = std::true_type;

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

    //! GlobalMemoryManager stl vector
    template<typename T>
    using ManagedVector = std::vector<T, STLAllocator<T>>;

    //! ThreadLocalMemoryManager stl vector
    template<typename T>
    using TLSManagedVector = std::vector<T, STLTLSAllocator<T>>;

    //! GlobalMemoryManager stl vector
    template<typename T>
    using ManagedStack = std::stack<T, ManagedDeque<T>>;

    //! ThreadLocalMemoryManager stl vector
    template<typename T>
    using TLSManagedStack = std::stack<T, TLSManagedDeque<T>>;
}