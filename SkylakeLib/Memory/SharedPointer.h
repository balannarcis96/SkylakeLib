//!
//! \file Memory.h
//! 
//! \brief Shared pointer abstraction for SkylakeLib to be used with the MemoryManager
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
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
	    	int32_t RefCountValue = ReferenceCount.load( std::memory_order_relaxed );
	    	while( false == std::atomic_compare_exchange_strong_explicit( &ReferenceCount, &RefCountValue, RefCountValue + 1, std::memory_order_release, std::memory_order_relaxed ) )
	    	{ }
	    }

        SKL_FORCEINLINE void ReleaseReferenceChecked() noexcept
        {
            ( void )--ReferenceCount;
        }

        SKL_FORCEINLINE bool ReleaseReference() noexcept
        {
            return 0 == --ReferenceCount;
        }

        std::atomic<int32_t>  ReferenceCount { 0 };
        const uint32_t        BlockSize      { 0 };
    };

    template<typename TObject, bool bDeconstruct = true>
    struct TSharedPtr
    {
        TSharedPtr() noexcept : Pointer { nullptr } {}
        
        TSharedPtr( const TSharedPtr& Other ) noexcept : Pointer{ Other.Pointer }
        {
            if( nullptr != Other.Pointer ) SKL_LIKELY
            {
                auto& CBlock { GetControlBlock() };
                CBlock.AddReference();
            }
        }

        TSharedPtr( TSharedPtr&& Other ) noexcept : Pointer{ Other.Pointer }
        {
            Other.Pointer = nullptr;
        }

        TSharedPtr& operator=( const TSharedPtr& Other ) noexcept
        {
            SKL_ASSERT( this != &Other );
            
            // Release the potention held ref
            ReleaseResource();
    
            // Copy the object pointer
            Pointer = Other.Pointer;
            
            //Increment reference count if possible
            if( nullptr != Other.Pointer ) SKL_LIKELY
            {
                auto& CBlock { GetControlBlock() };
                CBlock.AddReference();   
            }
        
            return *this;
        }

        TSharedPtr& operator=( TSharedPtr&& Other ) noexcept
        {
            SKL_ASSERT( this != &Other );
            
            // Release the potention held ref
            ReleaseResource();
    
            // Copy the object pointer
            Pointer = Other.Pointer;
            
            // Remove the pointer from Other
            Other.Pointer = nullptr;
        
            return *this;
        }

        ~TSharedPtr() noexcept 
        {
            ReleaseResource();
        }

        ControlBlock& GetControlBlock() noexcept
        {
            static_assert( sizeof( ControlBlock ) % 8 == 0 );

            return *reinterpret_cast<ControlBlock*>(
                reinterpret_cast<uint8_t*>( Pointer ) - sizeof( ControlBlock )
            );
        }

        const ControlBlock& GetControlBlock() const noexcept
        {
            static_assert( sizeof( ControlBlock ) % 8 == 0 );

            return *reinterpret_cast<ControlBlock*>(
                reinterpret_cast<uint8_t*>( Pointer ) - sizeof( ControlBlock )
            );
        }

        TObject* GetPointer() noexcept { return Pointer; }
        const TObject* GetPointer() const noexcept { return Pointer; }

        // std compatible API
        TObject* get() noexcept { return Pointer; }

        TObject* operator->() noexcept { return Pointer; }
        const TObject* operator->() const noexcept { return Pointer; }

        size_t use_count() const noexcept { return static_cast<size_t>( GetControlBlock().ReferenceCount.load( std::memory_order_relaxed ) ); }

        explicit operator bool() const noexcept { return nullptr != Pointer; }

    private:
        TSharedPtr( TObject* InPointer ) noexcept : Pointer{ InPointer } {}

        TObject* NewRefRaw() noexcept
        {
            //Increment reference count
            auto& CBlock { GetControlBlock() };
            CBlock.AddReference();

            return Pointer;  
        }

        void ReleaseResource() noexcept
        {
            if( nullptr != Pointer ) SKL_LIKELY
            {
                //Decrement reference count and deallocate if needed
                auto& CBlock { GetControlBlock() };
                if ( true == CBlock.ReleaseReference() )
                {   
                    if constexpr( true == bDeconstruct )
                    {
                        GDestructNothrow<TObject>( Pointer );                
                    }

                    MemoryManager::Deallocate( &CBlock, CBlock.BlockSize );
                }

                Pointer = nullptr;
            }
        }

        TObject* Pointer { nullptr };
    
        template<typename TObject, typename ...TArgs>
        friend TSharedPtr<TObject> MakeShared( TArgs... Args ) noexcept;
    };
}