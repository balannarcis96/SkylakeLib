//!
//! \file SharedPointer.h
//! 
//! \brief Shared pointer abstraction for SkylakeLib
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    template<typename TObject, typename TDeallocator = typename SKL::MemoryStrategy::SharedMemoryStrategy<TObject>::DestructDeallocator>
    struct TSharedPtr
    {
        using TObjectDecay          = std::remove_all_extents_t<TObject>;
        using element_type          = TObjectDecay;
        using MemoryPolicy          = typename TDeallocator::MyMemoryPolicy;
        using MyMemoryPolicyApplier = typename TDeallocator::MyMemoryPolicyApplier;
        
        TSharedPtr() noexcept : Pointer { nullptr } {}
        TSharedPtr( TObjectDecay* InPointer ) noexcept : Pointer{ InPointer } {}
        TSharedPtr( const TSharedPtr& Other ) noexcept : Pointer{ Other.Pointer } { if( nullptr != Pointer ){ Static_IncrementReference( Pointer ); } }
        TSharedPtr( TSharedPtr&& Other ) noexcept : Pointer{ Other.Pointer } { Other.Pointer = nullptr; }
        TSharedPtr& operator=( const TSharedPtr& Other ) noexcept
        {
            SKL_ASSERT( this != &Other );
            
            // Release the potentialy held ref
            reset();
    
            // Copy the object pointer and increment ref if possible
            Pointer = Other.NewRefRaw();

            return *this;
        }
        TSharedPtr& operator=( TSharedPtr&& Other ) noexcept
        {
            SKL_ASSERT( this != &Other );
            
            // Release the potentialy held ref
            reset();
    
            // Copy the object pointer
            Pointer = Other.Pointer;
            
            // Remove the pointer from Other
            Other.Pointer = nullptr;
        
            return *this;
        }
        ~TSharedPtr() noexcept 
        {
            reset();
        }

        // stl compatible API
        SKL_FORCEINLINE          TObjectDecay* get()                              noexcept { return Pointer; }
        SKL_FORCEINLINE const    TObjectDecay* get()                        const noexcept { return Pointer; }
        SKL_FORCEINLINE          TObjectDecay* operator->()                       noexcept { SKL_ASSERT( Pointer != nullptr ); return Pointer; }
        SKL_FORCEINLINE const    TObjectDecay* operator->()                 const noexcept { SKL_ASSERT( Pointer != nullptr ); return Pointer; }
        SKL_FORCEINLINE          TObjectDecay& operator*()                        noexcept { SKL_ASSERT( Pointer != nullptr ); return *Pointer; }
        SKL_FORCEINLINE const    TObjectDecay& operator*()                  const noexcept { SKL_ASSERT( Pointer != nullptr ); return *Pointer; }
        SKL_FORCEINLINE          size_t        use_count()                  const noexcept { SKL_ASSERT( Pointer != nullptr ); return static_cast<size_t>( Static_GetReferenceCount( Pointer ) ); }
        SKL_FORCEINLINE explicit               operator bool()              const noexcept { return nullptr != Pointer; }
        SKL_FORCEINLINE          void          reset()                            noexcept { if( nullptr != Pointer ) SKL_LIKELY { Static_Reset( Pointer ); Pointer = nullptr;  } }
        SKL_FORCEINLINE          TObjectDecay& operator[]( uint32_t Index )       noexcept { SKL_ASSERT( Pointer != nullptr ); SKL_ASSERT( true == MemoryPolicy::template IsValidIndexInArray( Pointer, Index ) ); return Pointer[ Index ]; }
        SKL_FORCEINLINE const    TObjectDecay& operator[]( uint32_t Index ) const noexcept { SKL_ASSERT( Pointer != nullptr ); SKL_ASSERT( true == MemoryPolicy::template IsValidIndexInArray( Pointer, Index ) ); return Pointer[ Index ]; }
        
        template<typename TNewObject, typename TNewDeallocator = TDeallocator>
        SKL_FORCEINLINE TSharedPtr<TNewObject, TNewDeallocator> CastTo() noexcept
        {
            if constexpr( std::is_class_v<TNewObject> )
            {
                return { static_cast<TNewObject*>( NewRefRaw() ) };
            }
            else
            {
                return { reinterpret_cast<TNewObject*>( NewRefRaw() ) };
            }
        }    
        template<typename TNewObject, typename TNewDeallocator = TDeallocator>
        SKL_FORCEINLINE TSharedPtr<TNewObject, TNewDeallocator> CastMoveTo() noexcept
        {
            TNewObject* Result;

            if constexpr( std::is_class_v<TNewObject> )
            {
                Result = static_cast<TNewObject*>( NewRefRaw() );
            }
            else
            {
                Result = reinterpret_cast<TNewObject*>( NewRefRaw() );
            }

            Pointer = nullptr;

            return { Result };
        }    
    
        //! 
        //! Reset the shared reference for InPtr
        //! 
        //! \invariant InPtr must be a valid pointer allocated using the same MemoryPolicy as this call
        //! 
        SKL_FORCEINLINE static void Static_Reset( TObjectDecay* InPtr ) noexcept
        {
            SKL_ASSERT( nullptr != InPtr );

            TDeallocator::Deallocate( InPtr );
        }
        
        //! 
        //! Get the reference count for InPtr
        //! 
        //! \invariant InPtr must be a valid pointer allocated using the same MemoryPolicy as this call
        //! 
        SKL_FORCEINLINE static uint32_t Static_GetReferenceCount( TObjectDecay* InPtr ) noexcept
        {
            SKL_ASSERT( nullptr != InPtr );

            if constexpr( std::is_array_v<TObject> )
            {
                return MemoryPolicy::GetReferenceCountForArray( InPtr );
            }
            else
            {
                return MemoryPolicy::GetReferenceCountForObject( InPtr );
            }
        }

        //! 
        //! Get the pointer to the actual memory block of the managed pointer
        //! 
        //! \invariant InPtr must be a valid pointer allocated using the same MemoryPolicy as this call
        //! 
        SKL_FORCEINLINE static void* Static_GetBlockPtr( TObjectDecay* InPtr ) noexcept
        {
            SKL_ASSERT( nullptr != InPtr );

            if constexpr( std::is_array_v<TObject> )
            {
                return MemoryPolicy::GetBlockPointerForArray( InPtr );
            }
            else
            {
                return MemoryPolicy::GetBlockPointerForObject( InPtr );
            }
        }

        //! 
        //! Get the pointer to the actual memory block of the managed pointer and the size of the metadata block
        //! 
        //! \invariant InPtr must be a valid pointer allocated using the same MemoryPolicy as this call
        //! 
        SKL_FORCEINLINE static std::pair<void*, size_t> Static_GetBlockPtrAndMetaBlockSize( TObjectDecay* InPtr ) noexcept
        {
            SKL_ASSERT( nullptr != InPtr );

            if constexpr( std::is_array_v<TObject> )
            {
                return MemoryPolicy::GetBlockPointerAndMetaBlockSizeForArray( InPtr );
            }
            else
            {
                return MemoryPolicy::GetBlockPointerAndMetaBlockSizeForObject( InPtr );
            }
        }

        //! 
        //! Get the size of the metadata block (compile time value)
        //! 
        //! \invariant InPtr must be a valid pointer allocated using the same MemoryPolicy as this call
        //! 
        SKL_FORCEINLINE static consteval size_t Static_GetMetaBlockSize() noexcept
        {
            if constexpr( std::is_array_v<TObject> )
            {
                return MemoryPolicy::GetMetaBlockSizeForArray();
            }
            else
            {
                return MemoryPolicy::GetMetaBlockSizeForObject();
            }
        }

        //! 
        //! Increment the reference count for InPtr
        //! 
        //! \invariant InPtr must be a valid pointer allocated using the same MemoryPolicy as this call
        //! 
        SKL_FORCEINLINE static void Static_IncrementReference( TObjectDecay* InPtr ) noexcept
        {
            SKL_ASSERT( nullptr != InPtr );

            if constexpr( std::is_array_v<TObject> )
            {
                MemoryPolicy::IncrementReferenceForArray( InPtr );
            }
            else
            {
                MemoryPolicy::IncrementReferenceForObject( InPtr );
            }
        }

        TObjectDecay* NewRefRaw() noexcept
        {
            if( Pointer != nullptr )
            {
                Static_IncrementReference( Pointer );
            }

            return Pointer;  
        }

    private:
        TObjectDecay* Pointer { nullptr };

        friend struct IAODSharedObjectTask;
    };

    template<typename TObject>
    using TSharedPtrNoDestruct = TSharedPtr<TObject, typename SKL::MemoryStrategy::SharedMemoryStrategy<TObject>::Deallocator>;
}