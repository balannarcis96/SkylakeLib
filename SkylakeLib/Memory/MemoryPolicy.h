//!
//! \file MemoryPolicy.h
//! 
//! \brief Global allocation policies
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL::MemoryPolicy
{
    struct ArrayHeader
    {
        SKL_FORCEINLINE ArrayHeader( uint32_t ItemSize, uint32_t ItemCount ) noexcept : ItemSize{ ItemSize }, ItemCount{ ItemCount } {}
        ~ArrayHeader() noexcept = default;

        SKL_FORCEINLINE uint32_t GetTotalArraySizeInBytes() const noexcept { return ItemSize * ItemCount; }

        uint32_t ItemSize;
        uint32_t ItemCount;
    };

    struct ControlBlock
    {
        ControlBlock( uint32_t ReferenceCount, uint32_t BlockSize ) noexcept 
            : ReferenceCount{ ReferenceCount },
              BlockSize{ BlockSize } 
        {}

        /**
         * \brief Adds 1 to the reference count of this instance.
         * \remarks Only call this function while holding a valid reference to this instance.
         */
        SKL_FORCEINLINE void AddReference() noexcept
        {
            uint32_t RefCountValue{ ReferenceCount.load( std::memory_order_relaxed ) };
            while( false == std::atomic_compare_exchange_strong_explicit( &ReferenceCount
                                                                        , &RefCountValue
                                                                        , RefCountValue + 1
                                                                        , std::memory_order_release
                                                                        , std::memory_order_relaxed ) ) { }
        }

        SKL_FORCEINLINE void ReleaseReferenceChecked() noexcept
        {
            ( void )--ReferenceCount;
        }

        SKL_FORCEINLINE bool ReleaseReference() noexcept
        {
            return 0 == --ReferenceCount;
        }

        std::atomic<uint32_t>  ReferenceCount { 0 };
        const uint32_t         BlockSize      { 0 };
    };

    struct UniqueMemoryPolicy final
    {
        static constexpr size_t CArrayHeaderSize = sizeof( ArrayHeader );

        //! Get unique array header
        SKL_FORCEINLINE static ArrayHeader& GetArrayHeader( void* InPtr ) noexcept
        {
            return *reinterpret_cast<ArrayHeader*>( reinterpret_cast<uint8_t*>( InPtr ) - CArrayHeaderSize );
        }

        //! Get memory block pointer and memory block size for unique object or array
        static std::pair<void*, size_t> GetArrayMemoryBlockAndBlockSize( void* InPtr ) noexcept
        {
            auto& Header { GetArrayHeader( InPtr ) };
            return { reinterpret_cast<void*>( &Header ), static_cast<size_t>( Header.GetTotalArraySizeInBytes() + CArrayHeaderSize ) };
        }
    
        //! Get the total memory block size
        SKL_FORCEINLINE static size_t GetMemoryBlockSizeForArray( void* InPtr ) noexcept
        {
            return GetArrayHeader( InPtr ).GetTotalArraySizeInBytes() + CArrayHeaderSize;
        }

        //! Get the total memory block size
        template<typename TObject>
        SKL_FORCEINLINE constexpr static size_t GetMemoryBlockSizeForObject( void* InPtr ) noexcept
        {
            return sizeof( TObject );
        }

        //! Get the total memory block size
        template<typename TArrayItem>
        SKL_FORCEINLINE constexpr static size_t CalculateNeededSizeForArray( uint32_t ItemCount ) noexcept
        {
            return ( sizeof( TArrayItem ) * ItemCount ) + CArrayHeaderSize;
        }

        //! Get the total memory block size
        template<typename TObject>
        SKL_FORCEINLINE constexpr static size_t CalculateNeededSizeForObject() noexcept
        {
            return sizeof( TObject );
        }

        SKL_FORCEINLINE static bool IsValidIndexInArray( void *InPtr, uint32_t Index ) noexcept 
        {
            const auto& Header{ GetArrayHeader( InPtr ) };
            return Header.ItemCount > Index;
        }

        //! Apply memory policy for array on memory block and, if possible/wanted, default construct each array item
        template<typename TArrayItem, bool bConstruct = true, bool bAcceptNoThrowConstructor = false>
        static TArrayItem* ConstructArray( void* InMemoryBlockPointer, uint32_t ItemCount ) noexcept
        {
            SKL_ASSERT( nullptr != InMemoryBlockPointer );
            SKL_ASSERT( 0 != ItemCount );

            constexpr uint32_t ItemSize     { sizeof( TArrayItem ) };
            constexpr bool     bCanConstruct{ true  == bConstruct && true == std::is_default_constructible_v<TArrayItem> };
            
            static_assert( false == bCanConstruct 
                      ||   false == bAcceptNoThrowConstructor ? 
                              true == std::is_nothrow_default_constructible_v<TArrayItem> : 
                              true == std::is_default_constructible_v<TArrayItem>   
                       , "TArrayItem must be [bAcceptNoThrowConstructor ? throw : nothrow] default constructible" );

            // construct array header
            GConstructNothrow<ArrayHeader>( InMemoryBlockPointer, ItemSize, ItemCount );
    
            // construct array pointer
            TArrayItem* Result{ reinterpret_cast<TArrayItem*>( reinterpret_cast<uint8_t*>( InMemoryBlockPointer ) + CArrayHeaderSize ) };
            
            // construct array items if possible
            if constexpr( true == bCanConstruct )
            {
                for( uint32_t i = 0; i < ItemCount; ++i )
                {
                    if constexpr( false == bAcceptNoThrowConstructor || std::is_nothrow_default_constructible_v<TArrayItem> )
                    {
                        GConstructNothrow<TArrayItem>( &Result[ i ] );  
                    }
                    else
                    {
                        GConstruct<TArrayItem>( &Result[ i ] );  
                    }                
                }
            }

            return Result;
        }

        //! Apply memory policy for object policy on memory block and, if possible/wanted, construct the object
        template<typename TObject, bool bConstruct, bool bAcceptNoThrowConstructor, typename ...TArgs>
        static TObject* ConstructObject( void* InMemoryBlockPointer, TArgs... Args ) noexcept
        {
            SKL_ASSERT( nullptr != InMemoryBlockPointer );

            constexpr bool bCanConstruct{ true == bConstruct && true == std::is_constructible_v<TObject, TArgs...> };

            static_assert( false == bConstruct || 
                           false == bAcceptNoThrowConstructor ? 
                              true == std::is_nothrow_constructible_v<TObject, TArgs...> : 
                              true == std::is_constructible_v<TObject, TArgs...>   
                       , "TObject must be [bAcceptNoThrowConstructor ? throw : nothrow] constructible" );

            TObject* Result{ reinterpret_cast<TObject*>( InMemoryBlockPointer ) };
            
            // construct if possible
            if constexpr( true == bCanConstruct )
            {
                if constexpr( false == bAcceptNoThrowConstructor || std::is_nothrow_constructible_v<TObject, TArgs...> )
                {
                    GConstructNothrow<TObject>( Result, std::forward<TArgs>( Args )... );  
                }
                else
                {
                    GConstruct<TObject>( Result, std::forward<TArgs>( Args )... );  
                }
            }

            return Result;
        }

        //! Deconstruct objects in the array if possible/wanted and return a pointer to the memory block and the memory block size
        template<typename TArrayItem, bool bDeconstruct = true, bool bAcceptNoThrowDestructor = false>
        static std::pair<void*, size_t> DestroyForArray( TArrayItem* InArrayPtr ) noexcept
        {
            SKL_ASSERT( nullptr != InArrayPtr );

            constexpr bool bCanDestruct{ true == bDeconstruct && true == std::is_destructible_v<TArrayItem> };
            
            static_assert( false == bCanDestruct 
                        || false == bAcceptNoThrowDestructor ? true == std::is_nothrow_destructible_v<TArrayItem> : true == std::is_destructible_v<TArrayItem>   
                         , "TArrayItem must be [bAcceptNoThrowConstructor ? throw : nothrow] destructible" );

            auto& Header{ GetArrayHeader( InArrayPtr ) };

            // destruct array items if possible/wanted
            if constexpr( true == bCanDestruct )
            {
                for( uint32_t i = 0; i < Header.ItemCount; ++i )
                {
                    if constexpr( false == bAcceptNoThrowDestructor || std::is_nothrow_destructible_v<TArrayItem> )
                    {
                        GDestructNothrow<TArrayItem>( &InArrayPtr[ i ] );  
                    }
                    else
                    {
                        GDestruct<TArrayItem>( &InArrayPtr[ i ] );  
                    }                
                }
            }

            return { reinterpret_cast<void*>( &Header ), static_cast<size_t>( Header.GetTotalArraySizeInBytes() + CArrayHeaderSize ) };
        }
        
        //! Destroy the policy and deconstruct the object if possible and wanted
        template<typename TObject, bool bDeconstruct = true, bool bAcceptNoThrowDestructor = false>
        static void* DestroyForObject( TObject* InObjectPtr ) noexcept
        {
            SKL_ASSERT( nullptr != InObjectPtr );

            constexpr bool bCanDestruct{ true == bDeconstruct && true == std::is_destructible_v<TObject> };
            
            static_assert( false == bCanDestruct 
                        || false == bAcceptNoThrowDestructor ? true == std::is_nothrow_destructible_v<TObject> : true == std::is_destructible_v<TObject>   
                         , "TObject must be [bAcceptNoThrowConstructor ? throw : nothrow] destructible" );

            // destruct object if possible/wanted
            if constexpr( true == bCanDestruct )
            {
                if constexpr( false == bAcceptNoThrowDestructor || std::is_nothrow_destructible_v<TObject> )
                {
                    GDestructNothrow<TObject>( InObjectPtr );  
                }
                else
                {
                    GDestruct<TObject>( InObjectPtr );  
                }  
            }

            return InObjectPtr;
        }
    };

    struct SharedMemoryPolicy final
    {
        static constexpr size_t CArrayHeaderSize        = sizeof( ArrayHeader );
        static constexpr size_t CControlBlockSize       = sizeof( ControlBlock );
        static constexpr size_t CSharedObjectHeaderSize = CControlBlockSize;
        static constexpr size_t CSharedArrayHeaderSize  = CControlBlockSize + CArrayHeaderSize;
        
        //! Get control block header pointer for the shared object
        SKL_FORCEINLINE static ControlBlock& GetControlBlockForObject( void* InPtr ) noexcept
        {
            return *reinterpret_cast<ControlBlock*>( reinterpret_cast<uint8_t*>( InPtr ) - CSharedObjectHeaderSize );
        }

        //! Get control block header pointer for the shared array
        SKL_FORCEINLINE static ControlBlock& GetControlBlockForArray( void* InPtr ) noexcept
        {
            return *reinterpret_cast<ControlBlock*>( reinterpret_cast<uint8_t*>( InPtr ) - CSharedArrayHeaderSize );
        }

        SKL_FORCEINLINE static void IncrementReferenceForObject( void* InPtr ) noexcept
        {
            auto& CBlock{ GetControlBlockForObject( InPtr ) };
            CBlock.AddReference();
        }

        SKL_FORCEINLINE static void IncrementReferenceForArray( void* InPtr ) noexcept
        {
            auto& CBlock{ GetControlBlockForArray( InPtr ) };
            CBlock.AddReference();
        }

        SKL_FORCEINLINE static uint32_t GetReferenceCountForObject( void* InPtr ) noexcept
        {
            return GetControlBlockForObject( InPtr ).ReferenceCount.load( std::memory_order_relaxed );
        }

        SKL_FORCEINLINE static uint32_t GetReferenceCountForArray( void* InPtr ) noexcept
        {
            return GetControlBlockForArray( InPtr ).ReferenceCount.load( std::memory_order_relaxed );
        }

        //! Get array header pointer for the shared array
        SKL_FORCEINLINE static ArrayHeader& GetArrayHeader( void* InPtr ) noexcept
        {
            return *reinterpret_cast<ArrayHeader*>( reinterpret_cast<uint8_t*>( InPtr ) - CArrayHeaderSize );
        }

        //! Get the total memory block size
        SKL_FORCEINLINE static size_t GetMemoryBlockSizeForArray( void* InPtr ) noexcept
        {
            return GetArrayHeader( InPtr ).GetTotalArraySizeInBytes() + CSharedArrayHeaderSize;
        }

        //! Get the total memory block size
        template<typename TObject>
        SKL_FORCEINLINE constexpr static size_t GetMemoryBlockSizeForObject( void* InPtr ) noexcept
        {
            return sizeof( TObject ) + CControlBlockSize;
        }

        //! Get memory block pointer and memory block size for shared object
        static std::pair<void*, size_t> GetObjectMemoryBlockAndBlockSize( void* InPtr ) noexcept
        {
            auto& ControlBlock { GetControlBlockForObject( InPtr ) };
            return { reinterpret_cast<void*>( &ControlBlock ), static_cast<size_t>( ControlBlock.BlockSize ) };
        }

        //! Get memory block pointer and memory block size for shared array
        static std::pair<void*, size_t> GetArrayMemoryBlockAndBlockSize( void* InPtr ) noexcept
        {
            auto& ControlBlock { GetControlBlockForArray( InPtr ) };
            return { reinterpret_cast<void*>( &ControlBlock ), static_cast<size_t>( ControlBlock.BlockSize ) };
        }

        //! Get the total memory block size
        template<typename TArrayItem>
        SKL_FORCEINLINE constexpr static size_t CalculateNeededSizeForArray( uint32_t ItemCount ) noexcept
        {
            return ( sizeof( TArrayItem ) * ItemCount ) + CSharedArrayHeaderSize;
        }

        //! Get the total memory block size
        template<typename TObject>
        SKL_FORCEINLINE constexpr static size_t CalculateNeededSizeForObject() noexcept
        {
            return sizeof( TObject ) + CSharedObjectHeaderSize;
        }

        SKL_FORCEINLINE static bool IsValidIndexInArray( void *InPtr, uint32_t Index ) noexcept 
        {
            const auto& Header{ GetArrayHeader( InPtr ) };
            return Header.ItemCount > Index;
        }

        //! Apply memory policy for array on memory block and, if possible/wanted, default construct each array item
        template<typename TArrayItem, bool bConstruct = true, bool bAcceptNoThrowConstructor = false>
        static TArrayItem* ConstructArray( void* InMemoryBlockPointer, uint32_t ItemCount ) noexcept
        {
            SKL_ASSERT( nullptr != InMemoryBlockPointer );
            SKL_ASSERT( 0 != ItemCount );

            constexpr uint32_t ItemSize     { sizeof( TArrayItem ) };
            constexpr bool     bCanConstruct{ true == bConstruct && true == std::is_default_constructible_v<TArrayItem> };
            
            static_assert( false == bCanConstruct 
                      ||   false == bAcceptNoThrowConstructor ? 
                              true == std::is_nothrow_default_constructible_v<TArrayItem> : 
                              true == std::is_default_constructible_v<TArrayItem>   
                       , "TArrayItem must be [bAcceptNoThrowConstructor ? throw : nothrow] default constructible" );

            // construct the control block
            GConstructNothrow<ControlBlock>( InMemoryBlockPointer, 1, ( ItemSize * ItemCount ) + static_cast<uint32_t>( CSharedArrayHeaderSize ) );
    
            // construct array header
            GConstructNothrow<ArrayHeader>( reinterpret_cast<uint8_t*>( InMemoryBlockPointer ) + CControlBlockSize, ItemSize, ItemCount );
    
            // construct array pointer
            TArrayItem* Result{ reinterpret_cast<TArrayItem*>( reinterpret_cast<uint8_t*>( InMemoryBlockPointer ) + CSharedArrayHeaderSize ) };
            
            // construct array items if possible
            if constexpr( true == bCanConstruct )
            {
                for( uint32_t i = 0; i < ItemCount; ++i )
                {
                    if constexpr( false == bAcceptNoThrowConstructor || std::is_nothrow_default_constructible_v<TArrayItem> )
                    {
                        GConstructNothrow<TArrayItem>( &Result[ i ] );  
                    }
                    else
                    {
                        GConstruct<TArrayItem>( &Result[ i ] );  
                    }                
                }
            }

            return Result;
        }

        //! Apply memory policy for object policy on memory block and, if possible/wanted, construct the object
        template<typename TObject, bool bConstruct, bool bAcceptNoThrowConstructor, typename ...TArgs>
        static TObject* ConstructObject( void* InMemoryBlockPointer, TArgs... Args ) noexcept
        {
            SKL_ASSERT( nullptr != InMemoryBlockPointer );

            constexpr bool     bCanConstruct{ true == bConstruct && true == std::is_constructible_v<TObject, TArgs...> };
            constexpr uint32_t AllocSize{ static_cast<uint32_t>( CalculateNeededSizeForObject<TObject>() ) };

            static_assert( false == bConstruct || 
                           false == bAcceptNoThrowConstructor ? 
                              true == std::is_nothrow_constructible_v<TObject, TArgs...> : 
                              true == std::is_constructible_v<TObject, TArgs...>   
                       , "TObject must be [bAcceptNoThrowConstructor ? throw : nothrow] constructible" );

            // construct the control block
            GConstructNothrow<ControlBlock>( InMemoryBlockPointer, 1, AllocSize );
    
            // calcuate the object pointer
            TObject* Result{ reinterpret_cast<TObject*>( reinterpret_cast<uint8_t*>( InMemoryBlockPointer ) + CSharedObjectHeaderSize ) };

            // construct if possible
            if constexpr( true == bCanConstruct )
            {
                if constexpr( false == bAcceptNoThrowConstructor || std::is_nothrow_constructible_v<TObject, TArgs...> )
                {
                    GConstructNothrow<TObject>( Result, std::forward<TArgs>( Args )... );  
                }
                else
                {
                    GConstruct<TObject>( Result, std::forward<TArgs>( Args )... );  
                }
            }

            return Result;
        }

        //! Try to destroy the policy (release reference) and deconstruct all objects in the array if possible 
        //! \returns a pointer to the memory block and the memory block size
        template<typename TArrayItem, bool bDeconstruct = true, bool bAcceptNoThrowDestructor = false>
        static std::pair<void*, size_t> DestroyForArray( TArrayItem* InArrayPtr ) noexcept
        {
            SKL_ASSERT( nullptr != InArrayPtr );

            constexpr bool bCanDestruct{ true == bDeconstruct && true == std::is_destructible_v<TArrayItem> };
            
            static_assert( false == bCanDestruct 
                        || false == bAcceptNoThrowDestructor ? 
                              true == std::is_nothrow_destructible_v<TArrayItem> : 
                              true == std::is_destructible_v<TArrayItem>   
                         , "TArrayItem must be [bAcceptNoThrowConstructor ? throw : nothrow] destructible" );

            auto& ControlBlock{ GetControlBlockForArray( InArrayPtr ) };

            // release reference 
            if ( true == ControlBlock.ReleaseReference() )
            {
                const auto& Header{ GetArrayHeader( InArrayPtr ) };

                // destruct array items if possible/wanted
                if constexpr( true == bCanDestruct )
                {
                    for( uint32_t i = 0; i < Header.ItemCount; ++i )
                    {
                        if constexpr( false == bAcceptNoThrowDestructor || std::is_nothrow_destructible_v<TArrayItem> )
                        {
                            GDestructNothrow<TArrayItem>( &InArrayPtr[ i ] );  
                        }
                        else
                        {
                            GDestruct<TArrayItem>( &InArrayPtr[ i ] );  
                        }                
                    }
                }
        
                return { reinterpret_cast<void*>( &ControlBlock ), static_cast<size_t>( ControlBlock.BlockSize ) };
            }

            return { nullptr , 0 };
        }
        
        //! Try to destroy the policy (release reference) and deconstruct the object if possible and wanted
        template<typename TObject, bool bDeconstruct = true, bool bAcceptNoThrowDestructor = false>
        static void* DestroyForObject( TObject* InObjectPtr ) noexcept
        {
            SKL_ASSERT( nullptr != InObjectPtr );

            constexpr bool bCanDestruct{ true == bDeconstruct && true == std::is_destructible_v<TObject> };
            
            static_assert( false == bCanDestruct 
                        || false == bAcceptNoThrowDestructor ? 
                              true == std::is_nothrow_destructible_v<TObject> : 
                              true == std::is_destructible_v<TObject>   
                         , "TObject must be [bAcceptNoThrowConstructor ? throw : nothrow] destructible" );

            auto& ControlBlock { GetControlBlockForObject( InObjectPtr ) };
            
            // release reference 
            if ( true == ControlBlock.ReleaseReference() )
            {
                // destruct object if possible/wanted
                if constexpr( true == bCanDestruct )
                {
                    if constexpr( false == bAcceptNoThrowDestructor || std::is_nothrow_destructible_v<TObject> )
                    {
                        GDestructNothrow<TObject>( InObjectPtr );  
                    }
                    else
                    {
                        GDestruct<TObject>( InObjectPtr );  
                    }  
                }

                return &ControlBlock;
            }

            return nullptr;
        }
    };

    template<typename TPolicy> requires( std::is_same_v<TPolicy, UniqueMemoryPolicy> || std::is_same_v<TPolicy, SharedMemoryPolicy> )
    struct MemoryPolicyApplier 
    {
        //! Get the total memory block size
        SKL_FORCEINLINE static size_t GetMemoryBlockSizeForArray( void* InPtr ) noexcept
        {
            return TPolicy::GetMemoryBlockSizeForArray( InPtr );
        }

        //! Get the total memory block size
        template<typename TObject>
        SKL_FORCEINLINE static size_t GetMemoryBlockSizeForObject( void* InPtr ) noexcept
        {
            return TPolicy::template GetMemoryBlockSizeForObject<TObject>( InPtr );
        }

        //! Apply memory policy for array on memory block and, if possible/wanted, default construct each array item
        template<typename TArrayItem, bool bConstruct = true, bool bAcceptNoThrowConstructor = false>
        SKL_FORCEINLINE static TArrayItem* ApplyPolicyAndConstructArray( void* InMemoryBlockPointer, uint32_t ItemsCount ) noexcept
        {
            return TPolicy::template ConstructArray<TArrayItem, bConstruct, bAcceptNoThrowConstructor>( InMemoryBlockPointer, ItemsCount );
        }

        //! Apply memory policy for object policy on memory block and, if possible/wanted, construct the object
        template<typename TObject, bool bConstruct = true, bool bAcceptNoThrowConstructor = false, typename ...TArgs>
        SKL_FORCEINLINE static TObject* ApplyPolicyAndConstructObject( void* InMemoryBlockPointer, TArgs... Args ) noexcept
        {
            return TPolicy::template ConstructObject<TObject, bConstruct, bAcceptNoThrowConstructor, TArgs...>( InMemoryBlockPointer, std::forward<TArgs>( Args )... );
        }

        //! \brief Try to destroy the policy (release reference) and deconstruct all objects in the array if possible and wanted
        //! \returns a pointer to the memory block and the memory block size
        template<typename TArrayItem, bool bDeconstruct = true, bool bAcceptNoThrowDestructor = false>
        SKL_FORCEINLINE static std::pair<void*, size_t> TryDestroyPolicyForArray( TArrayItem* IArrayPtr ) noexcept
        {
            return TPolicy::template DestroyForArray<TArrayItem, bDeconstruct, bAcceptNoThrowDestructor>( IArrayPtr );
        }

        //! Try to destroy the policy and deconstruct the object if possible and wanted
        template<typename TObject, bool bDeconstruct = true, bool bAcceptNoThrowDestructor = false>
        SKL_FORCEINLINE static void* TryDestroyPolicyForObject( TObject* InObjectPtr ) noexcept
        {
            return TPolicy::template DestroyForObject<TObject, bDeconstruct, bAcceptNoThrowDestructor>( InObjectPtr );
        }
    };
}

namespace SKL::MemoryDeallocation
{
    template<typename TObject, typename TMyMemoryPolicy, bool bDestruct = true> requires( std::is_same_v<TMyMemoryPolicy, MemoryPolicy::UniqueMemoryPolicy> )
    struct UniqueMemoryDeallocator final
    {
        using TDecayObject          = std::remove_all_extents_t<TObject>;
        using MyMemoryPolicy        = TMyMemoryPolicy;
        using MyMemoryPolicyApplier = SKL::MemoryPolicy::MemoryPolicyApplier<MyMemoryPolicy>;

        // std api
        void operator()( TDecayObject* InPtr ) const noexcept
        {
            Deallocate( InPtr );
        }

        static void Deallocate( TDecayObject* InPtr ) noexcept
        {
            if constexpr( true == std::is_array_v<TObject> )
            {
                auto Result{ MyMemoryPolicyApplier::template TryDestroyPolicyForArray<TDecayObject, bDestruct, false>( InPtr ) };
                GlobalMemoryManager::Deallocate( Result.first, Result.second );
            }
            else
            {
                auto Result{ MyMemoryPolicyApplier::template TryDestroyPolicyForObject<TObject, bDestruct, false>( InPtr ) };
                GlobalMemoryManager::Deallocate<sizeof( TObject )>( Result );
            }
        }
    };

    template<typename TObject, typename TMyMemoryPolicy, bool bDestruct = true> requires( std::is_same_v<TMyMemoryPolicy, MemoryPolicy::SharedMemoryPolicy> )
    struct SharedMemoryDeallocator final
    {
        using TDecayObject          = std::remove_all_extents_t<TObject>;
        using MyMemoryPolicy        = TMyMemoryPolicy;
        using MyMemoryPolicyApplier = SKL::MemoryPolicy::MemoryPolicyApplier<MyMemoryPolicy>;

        // std api
        void operator()( TDecayObject* InPtr ) const noexcept
        {
            Deallocate( InPtr );
        }

        static void Deallocate( TDecayObject* InPtr ) noexcept
        {
            if constexpr( true == std::is_array_v<TObject> )
            {
                auto Result{ MyMemoryPolicyApplier::TryDestroyPolicyForArray<TDecayObject, bDestruct, false>( InPtr ) };
                if ( nullptr != Result.first )
                {
                    GlobalMemoryManager::Deallocate( Result.first, Result.second );
                }
            }
            else
            {
                auto Result{ MyMemoryPolicyApplier::TryDestroyPolicyForObject<TObject, bDestruct, false>( InPtr ) };
                if ( nullptr != Result )
                {
                    GlobalMemoryManager::Deallocate( Result, sizeof( TObject ) );
                }
            }
        }
    };
}

namespace SKL::MemoryAllocation
{
    template<typename TObject, typename TMyMemoryPolicy> requires( std::is_same_v<TMyMemoryPolicy, MemoryPolicy::UniqueMemoryPolicy> || std::is_same_v<TMyMemoryPolicy, MemoryPolicy::SharedMemoryPolicy> )
    struct MemoryAllocator final
    {
        using TDecayObject          = std::remove_all_extents_t<TObject>;
        using MyMemoryPolicy        = TMyMemoryPolicy;
        using MyMemoryPolicyApplier = SKL::MemoryPolicy::MemoryPolicyApplier<MyMemoryPolicy>;

        //! Calculte needed size for an array of ItemsCount by policy
        template<typename TArrayItemType>
        SKL_FORCEINLINE static size_t CalculateNeededSizeForArray( uint32_t ItemCount ) noexcept
        {
            return MyMemoryPolicy::template CalculateNeededSizeForArray<TArrayItemType>( ItemCount );
        }

        //! Calculte needed size for an TObject by policy
        template<typename TObjectType>
        SKL_FORCEINLINE static size_t CalculateNeededSizeForObject( void* InPtr ) noexcept
        {
            return MyMemoryPolicy::template CalculateNeededSizeForObject<TObjectType>();
        }

        template<bool bConstruct = true, bool bAcceptNoThrowConstructor = false, typename ...TArgs>
        static TDecayObject* AllocateObject( TArgs... Args ) noexcept
        {
            constexpr uint32_t AllocSize{ static_cast<uint32_t>( MyMemoryPolicy::template CalculateNeededSizeForObject<TDecayObject>() ) };

            TDecayObject* Result;

            // allocate block
            auto AllocResult{ GlobalMemoryManager::Allocate<AllocSize>() };
            if( AllocResult.IsValid() ) SKL_LIKELY
            {
                // apply the object policy on the block
                Result = MyMemoryPolicyApplier::template ApplyPolicyAndConstructObject<TDecayObject, bConstruct, bAcceptNoThrowConstructor, TArgs...>( AllocResult.MemoryBlock, std::forward<TArgs>( Args )... );
            }
            else
            {
                SKL_ERR_FMT( "MemoryAllocator<>::AllocateObject(size:%u) Failed to allocate from GlobalMemoryManager!", AllocSize );
                Result = nullptr;
            }
            
            return Result;
        }

        template<bool bConstruct = true, bool bAcceptNoThrowConstructor = false>
        static TDecayObject* AllocateArray( uint32_t ItemCount ) noexcept
        {
            const uint32_t AllocSize{ static_cast<uint32_t>( MyMemoryPolicy::template CalculateNeededSizeForArray<TDecayObject>( ItemCount ) ) };

            TDecayObject* Result;

            // allocate block
            auto AllocResult{ GlobalMemoryManager::Allocate( AllocSize ) };
            if( AllocResult.IsValid() ) SKL_LIKELY
            {
                // apply the array policy on the block
                Result = MyMemoryPolicyApplier::template ApplyPolicyAndConstructArray<TDecayObject, bConstruct, bAcceptNoThrowConstructor>( AllocResult.MemoryBlock, ItemCount );
            }
            else
            {
                SKL_ERR_FMT( "MemoryAllocator<>::AllocateObject(size:%u) Failed to allocate from GlobalMemoryManager!", AllocSize );
                Result = nullptr;
            }
            
            return Result;        
        }
    };
}

namespace SKL::MemoryStrategy
{
    template<typename TObject>
    struct UniqueMemoryStrategy
    {
        using MemoryPolicy        = SKL::MemoryPolicy::UniqueMemoryPolicy;
        using Deallocator         = SKL::MemoryDeallocation::UniqueMemoryDeallocator<TObject, MemoryPolicy, false>;
        using DestructDeallocator = SKL::MemoryDeallocation::UniqueMemoryDeallocator<TObject, MemoryPolicy>;
        using Allocator           = SKL::MemoryAllocation::MemoryAllocator<TObject, MemoryPolicy>;
    };

    template<typename TObject>
    struct SharedMemoryStrategy
    {
        using MemoryPolicy        = SKL::MemoryPolicy::SharedMemoryPolicy;
        using Deallocator         = SKL::MemoryDeallocation::SharedMemoryDeallocator<TObject, MemoryPolicy, false>;
        using DestructDeallocator = SKL::MemoryDeallocation::SharedMemoryDeallocator<TObject, MemoryPolicy>;
        using Allocator           = SKL::MemoryAllocation::MemoryAllocator<TObject, MemoryPolicy>;
    };
}
