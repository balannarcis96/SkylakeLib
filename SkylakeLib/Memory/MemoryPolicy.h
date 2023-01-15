//!
//! \file MemoryPolicy.h
//! 
//! \brief Global allocation policies
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    template<typename TObject>
    using TVirtualDeleter = ASD::FnPtr<void( SKL_CDECL *)( TObject* ) noexcept>;
}

namespace SKL::MemoryPolicy
{
    //[SemVer] Any changes must bump at least one of these components
    constexpr int32_t CVersionMajor = 1;
    constexpr int32_t CVersionMinor = 1;
    constexpr int32_t CVersionPatch = 1;

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

        //! Adds 1 to the reference count of this instance
        //! \remarks Only call this function while holding a valid reference to this instance
        SKL_FORCEINLINE void AddReference() noexcept
        {
#if defined(_MSC_VER) && (defined(_M_X64) || defined(_M_IX86))
		    // We do a regular SC increment here because it maps to an _InterlockedIncrement (lock inc).
		    // The codegen for a relaxed fetch_add is actually much worse under MSVC (lock xadd).
		    ++ReferenceCount;
#else
		    ( void )ReferenceCount.fetch_add( 1, std::memory_order_relaxed );
#endif
        }

        //! Removes 1 from the reference count of this instance
        //! \remarks Only call this function when you know that removing 1 reference will not 0 reference count
        SKL_FORCEINLINE void ReleaseReferenceChecked() noexcept
        {
            ( void )ReferenceCount.fetch_sub( 1, std::memory_order_acq_rel );
        }

        //! Removes 1 from the reference count of this instance
        //! \returns true if reached 0 ref count
        SKL_FORCEINLINE bool ReleaseReference() noexcept
        {
            return 1U == ReferenceCount.fetch_sub( 1, std::memory_order_acq_rel );
        }

        std::atomic<uint32_t>  ReferenceCount{ 0 }; //!< ref count
        const uint32_t         BlockSize     { 0 }; //!< total size of the shared memory block
    };

    struct UniqueMemoryPolicy final
    {
        static constexpr size_t CArrayHeaderSize   = sizeof( ArrayHeader );
        static constexpr bool   CHasVirtualDeleter = false;

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

        SKL_FORCEINLINE static void* GetBlockPointerForArray( void* InPtr ) noexcept
        {
            auto& Header{ GetArrayHeader( InPtr ) };
            return reinterpret_cast<void*>( &Header );
        }

        SKL_FORCEINLINE static void* GetBlockPointerForObject( void* InPtr ) noexcept
        {
            return InPtr;
        }

        SKL_FORCEINLINE static std::pair<void*, size_t> GetBlockPointerAndMetaBlockSizeForArray( void* InPtr ) noexcept
        {
            auto& Header { GetArrayHeader( InPtr ) };
            return { reinterpret_cast<void*>( &Header ), CArrayHeaderSize };
        }

        SKL_FORCEINLINE static std::pair<void*, size_t> GetBlockPointerAndMetaBlockSizeForObject( void* InPtr ) noexcept
        {
            return { InPtr, 0 };
        }

        SKL_FORCEINLINE static consteval size_t GetMetaBlockSizeForArray() noexcept
        {
            return CArrayHeaderSize;
        }

        SKL_FORCEINLINE static consteval size_t GetMetaBlockSizeForObject() noexcept
        {
            return 0;
        }

        //! Apply memory policy for array on memory block and, if possible/wanted, default construct each array item
        template<typename TArrayItem, bool bConstruct = true, bool bAcceptThrowConstructor = false>
        static TArrayItem* ConstructArray( void* InMemoryBlockPointer, uint32_t ItemCount ) noexcept
        {
            SKL_ASSERT( nullptr != InMemoryBlockPointer );
            SKL_ASSERT( 0 != ItemCount );

            constexpr uint32_t ItemSize     { sizeof( TArrayItem ) };
            constexpr bool     bCanConstruct{ true  == bConstruct && true == std::is_default_constructible_v<TArrayItem> };
            
            static_assert( false == bCanConstruct 
                      ||   false == bAcceptThrowConstructor ? 
                              true == std::is_nothrow_default_constructible_v<TArrayItem> : 
                              true == std::is_default_constructible_v<TArrayItem>   
                       , "TArrayItem must be [bAcceptThrowConstructor ? throw : nothrow] default constructible" );

            // construct array header
            GConstructNothrow<ArrayHeader>( InMemoryBlockPointer, ItemSize, ItemCount );
    
            // construct array pointer
            TArrayItem* Result{ reinterpret_cast<TArrayItem*>( reinterpret_cast<uint8_t*>( InMemoryBlockPointer ) + CArrayHeaderSize ) };
            
            // construct array items if possible
            if constexpr( true == bCanConstruct )
            {
                for( uint32_t i = 0; i < ItemCount; ++i )
                {
                    if constexpr( false == bAcceptThrowConstructor || std::is_nothrow_default_constructible_v<TArrayItem> )
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
        template<typename TObject, bool bConstruct, bool bAcceptThrowConstructor, typename ...TArgs>
        static TObject* ConstructObject( void* InMemoryBlockPointer, TArgs... Args ) noexcept
        {
            SKL_ASSERT( nullptr != InMemoryBlockPointer );

            constexpr bool bCanConstruct{ true == bConstruct && true == std::is_constructible_v<TObject, TArgs...> };

            static_assert( false == bConstruct || 
                           false == bAcceptThrowConstructor ? 
                              true == std::is_nothrow_constructible_v<TObject, TArgs...> : 
                              true == std::is_constructible_v<TObject, TArgs...>   
                       , "TObject must be [bAcceptThrowConstructor ? nothrow : throw] constructible" );

            TObject* Result{ reinterpret_cast<TObject*>( InMemoryBlockPointer ) };
            
            // construct if possible
            if constexpr( true == bCanConstruct )
            {
                if constexpr( false == bAcceptThrowConstructor || std::is_nothrow_constructible_v<TObject, TArgs...> )
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
        template<typename TArrayItem, bool bDeconstruct = true, bool bAcceptThrowConstructor = false>
        static std::pair<void*, size_t> DestroyForArray( TArrayItem* InArrayPtr ) noexcept
        {
            SKL_ASSERT( nullptr != InArrayPtr );

            constexpr bool bCanDestruct{ true == bDeconstruct && true == std::is_destructible_v<TArrayItem> };
            
            static_assert( false == bCanDestruct 
                        || false == bAcceptThrowConstructor ? true == std::is_nothrow_destructible_v<TArrayItem> : true == std::is_destructible_v<TArrayItem>   
                         , "TArrayItem must be [bAcceptThrowConstructor ? throw : nothrow] destructible" );

            auto& Header{ GetArrayHeader( InArrayPtr ) };

            // destruct array items if possible/wanted
            if constexpr( true == bCanDestruct )
            {
                for( uint32_t i = 0; i < Header.ItemCount; ++i )
                {
                    if constexpr( false == bAcceptThrowConstructor || std::is_nothrow_destructible_v<TArrayItem> )
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
        template<typename TObject, bool bDeconstruct = true, bool bAcceptThrowConstructor = false>
        static void* DestroyForObject( TObject* InObjectPtr ) noexcept
        {
            SKL_ASSERT( nullptr != InObjectPtr );

            constexpr bool bCanDestruct{ true == bDeconstruct && true == std::is_destructible_v<TObject> };
            
            static_assert( false == bCanDestruct 
                        || false == bAcceptThrowConstructor ? true == std::is_nothrow_destructible_v<TObject> : true == std::is_destructible_v<TObject>   
                         , "TObject must be [bAcceptThrowConstructor ? throw : nothrow] destructible" );

            // destruct object if possible/wanted
            if constexpr( true == bCanDestruct )
            {
                if constexpr( false == bAcceptThrowConstructor || std::is_nothrow_destructible_v<TObject> )
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

    template<bool bVirtualDeleter = false>
    struct SharedMemoryPolicy final
    {
        static constexpr bool   CHasVirtualDeleter      = bVirtualDeleter;
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
        
        SKL_FORCEINLINE static void DecrementReferenceForObject( void* InPtr ) noexcept
        {
            auto& CBlock{ GetControlBlockForObject( InPtr ) };
            CBlock.ReleaseReferenceChecked();
        }

        SKL_FORCEINLINE static void DecrementReferenceForArray( void* InPtr ) noexcept
        {
            auto& CBlock{ GetControlBlockForArray( InPtr ) };
            CBlock.ReleaseReferenceChecked();
        }

        SKL_FORCEINLINE static void SetReferenceCountForObject( void* InPtr, uint32_t InRefCount ) noexcept
        {
            auto& CBlock{ GetControlBlockForObject( InPtr ) };
            CBlock.ReferenceCount.store( InRefCount, std::memory_order_relaxed );
        }

        SKL_FORCEINLINE static void SetReferenceCountForArray( void* InPtr, uint32_t InRefCount ) noexcept
        {
            auto& CBlock{ GetControlBlockForArray( InPtr ) };
            CBlock.ReferenceCount.store( InRefCount, std::memory_order_relaxed );
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
        SKL_FORCEINLINE static std::pair<void*, size_t> GetObjectMemoryBlockAndBlockSize( void* InPtr ) noexcept
        {
            auto& ControlBlock { GetControlBlockForObject( InPtr ) };
            return { reinterpret_cast<void*>( &ControlBlock ), static_cast<size_t>( ControlBlock.BlockSize ) };
        }

        //! Get memory block pointer and memory block size for shared array
        SKL_FORCEINLINE static std::pair<void*, size_t> GetArrayMemoryBlockAndBlockSize( void* InPtr ) noexcept
        {
            auto& ControlBlock { GetControlBlockForArray( InPtr ) };
            return { reinterpret_cast<void*>( &ControlBlock ), static_cast<size_t>( ControlBlock.BlockSize ) };
        }

        SKL_FORCEINLINE static void* GetBlockPointerForArray( void* InPtr ) noexcept
        {
            auto& ControlBlock { GetControlBlockForArray( InPtr ) };
            return reinterpret_cast<void*>( &ControlBlock );
        }

        SKL_FORCEINLINE static void* GetBlockPointerForObject( void* InPtr ) noexcept
        {
            auto& ControlBlock { GetControlBlockForObject( InPtr ) };
            return reinterpret_cast<void*>( &ControlBlock );
        }

        SKL_FORCEINLINE static std::pair<void*, size_t> GetBlockPointerAndMetaBlockSizeForArray( void* InPtr ) noexcept
        {
            auto& ControlBlock { GetControlBlockForArray( InPtr ) };
            return { reinterpret_cast<void*>( &ControlBlock ), CSharedArrayHeaderSize };
        }

        SKL_FORCEINLINE static std::pair<void*, size_t> GetBlockPointerAndMetaBlockSizeForObject( void* InPtr ) noexcept
        {
            auto& ControlBlock { GetControlBlockForObject( InPtr ) };
            return { reinterpret_cast<void*>( &ControlBlock ), CSharedObjectHeaderSize };
        }

        SKL_FORCEINLINE static consteval size_t GetMetaBlockSizeForArray() noexcept
        {
            return CSharedArrayHeaderSize;
        }

        SKL_FORCEINLINE static consteval size_t GetMetaBlockSizeForObject() noexcept
        {
            return CSharedObjectHeaderSize;
        }
        
        template<typename TObject>
        SKL_FORCEINLINE static TVirtualDeleter<TObject>& GetVirtualDeleterForObject( void* InPtr ) noexcept
        {
            static_assert( false == std::is_array_v<TObject> );

            auto[ Ptr, Size ] = GetObjectMemoryBlockAndBlockSize( InPtr );
            SKL_ASSERT( sizeof( TVirtualDeleter<TObject> ) < static_cast<size_t>( Size ) );

            const size_t OffsetFromBaseWhereTheDeleterResides{ Size - sizeof( TVirtualDeleter<TObject> ) };
            uint8_t*     CursorPtr                           { reinterpret_cast<uint8_t*>( Ptr ) + OffsetFromBaseWhereTheDeleterResides };

            return *reinterpret_cast<TVirtualDeleter<TObject>*>( CursorPtr );
        }

        template<typename TObject>
        SKL_FORCEINLINE static void SetVirtualDeleterForObject( void* InPtr, TVirtualDeleter<TObject>&& InCustomDeleter ) noexcept
        {
            static_assert( false == std::is_array_v<TObject> );

            TVirtualDeleter<TObject>& Target{ GetVirtualDeleterForObject<TObject>( InPtr ) };
            Target = std::move( InCustomDeleter );
        }
        
        template<typename TArrayItem>
        SKL_FORCEINLINE static TVirtualDeleter<TArrayItem>& GetVirtualDeleterForArray( void* InPtr ) noexcept
        {
            static_assert( false, "Virtual deleter for array not yet supported" );
            return {};
        }

        template<typename TArrayItem>
        SKL_FORCEINLINE static void SetVirtualDeleterForArray( void* InPtr, TVirtualDeleter<TArrayItem>&& InCustomDeleter ) noexcept
        {
            static_assert( false, "Virtual deleter for array not yet supported" );
        }

        //! Get the total memory block size
        template<typename TArrayItem>
        SKL_FORCEINLINE constexpr static size_t CalculateNeededSizeForArray( uint32_t ItemCount ) noexcept
        {
            static_assert( false == CHasVirtualDeleter, "Virtual deleter for arrays is not yet supported!" );

            return ( sizeof( TArrayItem ) * ItemCount ) + CSharedArrayHeaderSize;
        }

        //! Get the total memory block size
        template<typename TObject>
        SKL_FORCEINLINE constexpr static size_t CalculateNeededSizeForObject() noexcept
        {
            size_t Result{ sizeof( TObject ) + CSharedObjectHeaderSize };

            if constexpr( CHasVirtualDeleter )
            {
                Result += sizeof( TVirtualDeleter<TObject> );
            }

            return Result;
        }

        SKL_FORCEINLINE static bool IsValidIndexInArray( void *InPtr, uint32_t Index ) noexcept 
        {
            const auto& Header{ GetArrayHeader( InPtr ) };
            return Header.ItemCount > Index;
        }

        //! Apply memory policy for array on memory block and, if possible/wanted, default construct each array item
        template<typename TArrayItem, bool bConstruct = true, bool bAcceptThrowConstructor = false>
        static TArrayItem* ConstructArray( void* InMemoryBlockPointer, uint32_t ItemCount ) noexcept
        {
            static_assert( false == CHasVirtualDeleter, "Virtual deleter for arrays is not yet supported!" );

            SKL_ASSERT( nullptr != InMemoryBlockPointer );
            SKL_ASSERT( 0 != ItemCount );

            constexpr uint32_t ItemSize     { sizeof( TArrayItem ) };
            constexpr bool     bCanConstruct{ true == bConstruct && true == std::is_default_constructible_v<TArrayItem> };
            
            static_assert( false == bCanConstruct 
                      ||   false == bAcceptThrowConstructor ? 
                              true == std::is_nothrow_default_constructible_v<TArrayItem> : 
                              true == std::is_default_constructible_v<TArrayItem>   
                       , "TArrayItem must be [bAcceptThrowConstructor ? throw : nothrow] default constructible" );

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
                    if constexpr( false == bAcceptThrowConstructor || std::is_nothrow_default_constructible_v<TArrayItem> )
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
        template<typename TObject, bool bConstruct, bool bAcceptThrowConstructor, typename ...TArgs>
        static TObject* ConstructObject( void* InMemoryBlockPointer, TArgs... Args ) noexcept
        {
            SKL_ASSERT( nullptr != InMemoryBlockPointer );

            constexpr bool     bCanConstruct{ true == bConstruct && true == std::is_constructible_v<TObject, TArgs...> };
            constexpr uint32_t AllocSize{ static_cast<uint32_t>( CalculateNeededSizeForObject<TObject>() ) };

            static_assert( false == bConstruct || 
                           false == bAcceptThrowConstructor ? 
                              true == std::is_nothrow_constructible_v<TObject, TArgs...> : 
                              true == std::is_constructible_v<TObject, TArgs...>   
                       , "TObject must be [bAcceptThrowConstructor ? throw : nothrow] constructible" );

            // construct the control block
            GConstructNothrow<ControlBlock>( InMemoryBlockPointer, 1, AllocSize );
    
            // calculate the object pointer
            TObject* Result{ reinterpret_cast<TObject*>( reinterpret_cast<uint8_t*>( InMemoryBlockPointer ) + CSharedObjectHeaderSize ) };

            // construct if possible
            if constexpr( true == bCanConstruct )
            {
                if constexpr( false == bAcceptThrowConstructor || std::is_nothrow_constructible_v<TObject, TArgs...> )
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
        template<typename TArrayItem, bool bDeconstruct = true, bool bAcceptThrowConstructor = false>
        static std::pair<void*, size_t> DestroyForArray( TArrayItem* InArrayPtr ) noexcept
        {
            static_assert( false == CHasVirtualDeleter, "Virtual deleter for arrays is not yet supported!" );

            SKL_ASSERT( nullptr != InArrayPtr );

            constexpr bool bCanDestruct{ true == bDeconstruct && true == std::is_destructible_v<TArrayItem> };
            
            static_assert( false == bCanDestruct 
                        || false == bAcceptThrowConstructor ? 
                              true == std::is_nothrow_destructible_v<TArrayItem> : 
                              true == std::is_destructible_v<TArrayItem>   
                         , "TArrayItem must be [bAcceptThrowConstructor ? throw : nothrow] destructible" );

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
                        if constexpr( false == bAcceptThrowConstructor || std::is_nothrow_destructible_v<TArrayItem> )
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
        template<typename TObject, bool bDeconstruct = true, bool bAcceptThrowConstructor = false>
        static void* DestroyForObject( TObject* InObjectPtr ) noexcept
        {
            SKL_ASSERT( nullptr != InObjectPtr );

            constexpr bool bCanDestruct{ true == bDeconstruct && true == std::is_destructible_v<TObject> && false == CHasVirtualDeleter };
            
            static_assert( false == bCanDestruct 
                        || false == bAcceptThrowConstructor ? 
                              true == std::is_nothrow_destructible_v<TObject> : 
                              true == std::is_destructible_v<TObject>   
                         , "TObject must be [bAcceptThrowConstructor ? throw : nothrow] destructible" );

            auto& ControlBlock { GetControlBlockForObject( InObjectPtr ) };
            
            // release reference 
            if ( true == ControlBlock.ReleaseReference() )
            {
                // destruct object if possible/wanted
                if constexpr( true == bCanDestruct )
                {
                    if constexpr( false == bAcceptThrowConstructor || std::is_nothrow_destructible_v<TObject> )
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

    template<typename TPolicy> requires( std::is_same_v<TPolicy, UniqueMemoryPolicy> || std::is_same_v<TPolicy, SharedMemoryPolicy<true>> || std::is_same_v<TPolicy, SharedMemoryPolicy<false>> )
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
        template<typename TArrayItem, bool bConstruct = true, bool bAcceptThrowConstructor = false>
        SKL_FORCEINLINE static TArrayItem* ApplyPolicyAndConstructArray( void* InMemoryBlockPointer, uint32_t ItemsCount ) noexcept
        {
            return TPolicy::template ConstructArray<TArrayItem, bConstruct, bAcceptThrowConstructor>( InMemoryBlockPointer, ItemsCount );
        }

        //! Apply memory policy for object policy on memory block and, if possible/wanted, construct the object
        template<typename TObject, bool bConstruct = true, bool bAcceptThrowConstructor = false, typename ...TArgs>
        SKL_FORCEINLINE static TObject* ApplyPolicyAndConstructObject( void* InMemoryBlockPointer, TArgs... Args ) noexcept
        {
            return TPolicy::template ConstructObject<TObject, bConstruct, bAcceptThrowConstructor, TArgs...>( InMemoryBlockPointer, std::forward<TArgs>( Args )... );
        }

        //! \brief Try to destroy the policy (release reference) and deconstruct all objects in the array if possible and wanted
        //! \returns a pointer to the memory block and the memory block size
        template<typename TArrayItem, bool bDeconstruct = true, bool bAcceptThrowConstructor = false>
        SKL_FORCEINLINE static std::pair<void*, size_t> TryDestroyPolicyForArray( TArrayItem* IArrayPtr ) noexcept
        {
            return TPolicy::template DestroyForArray<TArrayItem, bDeconstruct, bAcceptThrowConstructor>( IArrayPtr );
        }

        //! Try to destroy the policy and deconstruct the object if possible and wanted
        template<typename TObject, bool bDeconstruct = true, bool bAcceptThrowConstructor = false>
        SKL_FORCEINLINE static void* TryDestroyPolicyForObject( TObject* InObjectPtr ) noexcept
        {
            return TPolicy::template DestroyForObject<TObject, bDeconstruct, bAcceptThrowConstructor>( InObjectPtr );
        }

        template<typename TObject>
        SKL_FORCEINLINE static TVirtualDeleter<TObject>& GetVirtualDeleterForObject( TObject* InObjectPtr ) noexcept
        {
            return TPolicy::template GetVirtualDeleterForObject<TObject>( InObjectPtr );
        }

        template<typename TObject>
        SKL_FORCEINLINE static void SetVirtualDeleterForObject( TObject* InObjectPtr, TVirtualDeleter<TObject>&& InDeleter ) noexcept
        {
            TPolicy::template SetVirtualDeleterForObject<TObject>( InObjectPtr, std::move( InDeleter ) );
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

    template<typename TObject, typename TMyMemoryPolicy, bool bDestruct = true> requires( std::is_same_v<TMyMemoryPolicy, MemoryPolicy::SharedMemoryPolicy<true>> || std::is_same_v<TMyMemoryPolicy, MemoryPolicy::SharedMemoryPolicy<false>> )
    struct SharedMemoryDeallocator final
    {
        using TDecayObject          = std::remove_all_extents_t<TObject>;
        using MyMemoryPolicy        = TMyMemoryPolicy;
        using MyMemoryPolicyApplier = SKL::MemoryPolicy::MemoryPolicyApplier<MyMemoryPolicy>;

        static constexpr bool CHasVirtualDeleter = MyMemoryPolicy::CHasVirtualDeleter;

        // std api
        void operator()( TDecayObject* InPtr ) const noexcept
        {
            Deallocate( InPtr );
        }

        static void Deallocate( TDecayObject* InPtr ) noexcept
        {
            static_assert( false == std::is_array_v<TObject> || false == CHasVirtualDeleter, "Virtual deleter for array is not yet supported!" );

            if constexpr( true == std::is_array_v<TObject> )
            {
                auto Result{ MyMemoryPolicyApplier::template TryDestroyPolicyForArray<TDecayObject, bDestruct, false>( InPtr ) };
                if ( nullptr != Result.first )
                {
                    GlobalMemoryManager::Deallocate( Result.first, Result.second );
                }
            }
            else
            {
                auto Result{ MyMemoryPolicyApplier::template TryDestroyPolicyForObject<TObject, bDestruct, false>( InPtr ) };
                if ( nullptr != Result )
                {
                    if constexpr( CHasVirtualDeleter )
                    {
                        TVirtualDeleter<TDecayObject>& VirtualDeleter{ MyMemoryPolicyApplier::template GetVirtualDeleterForObject<TDecayObject>( InPtr ) };
                        VirtualDeleter( InPtr );
                    }
                    else
                    {
                        const typename MemoryPolicy::ControlBlock* CB{ reinterpret_cast<typename MemoryPolicy::ControlBlock*>( Result ) };
                        GlobalMemoryManager::Deallocate( Result, static_cast<size_t>( CB->BlockSize ) );
                    }
                }
            }
        }
    };
}

namespace SKL
{
    template<typename TObject>
    void GlobalAllocatedDeleter( TObject* InObj ) noexcept
    {
        if constexpr ( std::is_nothrow_destructible_v<TObject> )
        {
            GDestructNothrow<TObject>( InObj );
        }
        else
        {
            GDestruct<TObject>( InObj );
        }

        SKL_ASSERT( nullptr != InObj );
        auto Data{ SKL::TVirtualDeletedSharedPtr<TObject>::Static_GetBlockPtrAndMetaBlockSize( InObj ) };
        SKL::GlobalMemoryManager::Deallocate( Data.first, Data.second );
    }
}

namespace SKL::MemoryAllocation
{
    template<typename TObject, typename TMyMemoryPolicy> requires( std::is_same_v<TMyMemoryPolicy, MemoryPolicy::UniqueMemoryPolicy> || std::is_same_v<TMyMemoryPolicy, MemoryPolicy::SharedMemoryPolicy<true>> || std::is_same_v<TMyMemoryPolicy, MemoryPolicy::SharedMemoryPolicy<false>> )
    struct MemoryAllocator final
    {
        using TDecayObject          = std::remove_all_extents_t<TObject>;
        using MyMemoryPolicy        = TMyMemoryPolicy;
        using MyMemoryPolicyApplier = SKL::MemoryPolicy::MemoryPolicyApplier<MyMemoryPolicy>;
        using VirtualDeleter        = TVirtualDeleter<TObject>;

        static constexpr bool CHasVirtualDeleter = MyMemoryPolicy::CHasVirtualDeleter;

        //! Calculate needed size for an array of ItemsCount by policy
        template<typename TArrayItemType>
        SKL_FORCEINLINE static constexpr size_t CalculateNeededSizeForArray( uint32_t ItemCount ) noexcept
        {
            static_assert( false == CHasVirtualDeleter, "Virtual deleter for arrays is not yet supported!" );
            return MyMemoryPolicy::template CalculateNeededSizeForArray<TArrayItemType>( ItemCount );
        }

        //! Calculate needed size for an TObject by policy
        template<typename TObjectType>
        SKL_FORCEINLINE static constexpr size_t CalculateNeededSizeForObject() noexcept
        {
            return MyMemoryPolicy::template CalculateNeededSizeForObject<TObjectType>();
        }

        template<bool bConstruct = true, bool bAcceptThrowConstructor = false, typename ...TArgs>
        static TDecayObject* AllocateObject( TArgs... Args ) noexcept
        {
            static_assert( false == CHasVirtualDeleter, "Use the overload which accepts the virtual deleter" );

            constexpr uint32_t AllocSize{ static_cast<uint32_t>( MyMemoryPolicy::template CalculateNeededSizeForObject<TDecayObject>() ) };

            TDecayObject* Result;

            // allocate block
            auto AllocResult{ GlobalMemoryManager::Allocate<AllocSize>() };
            if( AllocResult.IsValid() ) SKL_LIKELY
            {
                // apply the object policy on the block
                Result = MyMemoryPolicyApplier::template ApplyPolicyAndConstructObject<TDecayObject, bConstruct, bAcceptThrowConstructor, TArgs...>( AllocResult.MemoryBlock, std::forward<TArgs>( Args )... );
            }
            else
            {
                SKLL_ERR_FMT( "MemoryAllocator<>::AllocateObject(size:%u) Failed to allocate from GlobalMemoryManager!", AllocSize );
                Result = nullptr;
            }
            
            return Result;
        }
        
        template<bool bConstruct = true, bool bAcceptThrowConstructor = false, typename ...TArgs>
        static TDecayObject* AllocateObject( VirtualDeleter&& Deleter, TArgs... Args ) noexcept
        {
            static_assert( true == CHasVirtualDeleter, "Use the overload which does not accept the virtual deleter" );

            constexpr uint32_t AllocSize{ static_cast<uint32_t>( MyMemoryPolicy::template CalculateNeededSizeForObject<TDecayObject>() ) };

            TDecayObject* Result;

            // allocate block
            auto AllocResult{ GlobalMemoryManager::Allocate<AllocSize>() };
            if( AllocResult.IsValid() ) SKL_LIKELY
            {
                // apply the object policy on the block
                Result = MyMemoryPolicyApplier::template ApplyPolicyAndConstructObject<TDecayObject, bConstruct, bAcceptThrowConstructor, TArgs...>( AllocResult.MemoryBlock, std::forward<TArgs>( Args )... );

                // set virtual deleter
                MyMemoryPolicyApplier::template SetVirtualDeleterForObject<TDecayObject>( Result, std::move( Deleter ) );
            }
            else
            {
                SKLL_ERR_FMT( "MemoryAllocator<>::AllocateObject(size:%u) Failed to allocate from GlobalMemoryManager!", AllocSize );
                Result = nullptr;
            }
            
            return Result;
        }

        template<bool bConstruct = true, bool bAcceptThrowConstructor = false>
        static TDecayObject* AllocateArray( uint32_t ItemCount ) noexcept
        {
            static_assert( false == CHasVirtualDeleter, "Virtual deleter for arrays is not yet supported!" );

            const uint32_t AllocSize{ static_cast<uint32_t>( MyMemoryPolicy::template CalculateNeededSizeForArray<TDecayObject>( ItemCount ) ) };

            TDecayObject* Result;

            // allocate block
            auto AllocResult{ GlobalMemoryManager::Allocate( AllocSize ) };
            if( AllocResult.IsValid() ) SKL_LIKELY
            {
                // apply the array policy on the block
                Result = MyMemoryPolicyApplier::template ApplyPolicyAndConstructArray<TDecayObject, bConstruct, bAcceptThrowConstructor>( AllocResult.MemoryBlock, ItemCount );
            }
            else
            {
                SKLL_ERR_FMT( "MemoryAllocator<>::AllocateObject(size:%u) Failed to allocate from GlobalMemoryManager!", AllocSize );
                Result = nullptr;
            }
            
            return Result;        
        }
    };
}

namespace SKL::MemoryStrategy
{
    template<typename TObject, bool bVirtualDeleter = false>
    struct UniqueMemoryStrategy
    {
        static_assert( false == bVirtualDeleter, "Virtual deleter for unique objects not yet supported" );

        using MemoryPolicy        = SKL::MemoryPolicy::UniqueMemoryPolicy;
        using MemoryPolicyApplier = SKL::MemoryPolicy::MemoryPolicyApplier<MemoryPolicy>;
        using Deallocator         = SKL::MemoryDeallocation::UniqueMemoryDeallocator<TObject, MemoryPolicy, false>;
        using DestructDeallocator = SKL::MemoryDeallocation::UniqueMemoryDeallocator<TObject, MemoryPolicy>;
        using Allocator           = SKL::MemoryAllocation::MemoryAllocator<TObject, MemoryPolicy>;
    };

    template<typename TObject, bool bVirtualDeleter = false>
    struct SharedMemoryStrategy
    {
        using MemoryPolicy        = SKL::MemoryPolicy::SharedMemoryPolicy<bVirtualDeleter>;
        using MemoryPolicyApplier = SKL::MemoryPolicy::MemoryPolicyApplier<MemoryPolicy>;
        using Deallocator         = SKL::MemoryDeallocation::SharedMemoryDeallocator<TObject, MemoryPolicy, false>;
        using DestructDeallocator = SKL::MemoryDeallocation::SharedMemoryDeallocator<TObject, MemoryPolicy>;
        using Allocator           = SKL::MemoryAllocation::MemoryAllocator<TObject, MemoryPolicy>;
    };
}

namespace SKL
{
    consteval bool CExpectMemoryPolicyVersion( int32_t Major )
    {
        return MemoryPolicy::CVersionMajor == Major;
    }
}