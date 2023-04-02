//!
//! \file ThreadMemoryManagement.h
//! 
//! \brief Thread local memory management
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    struct ThreadLocalMemoryManager final : public ITLSSingleton<ThreadLocalMemoryManager>
    {
        using MemoryManager = LocalMemoryManager<ThreadLocalMemoryManagerConfig>;
        using AllocResult   = typename MemoryManager::AllocResult;        

        RStatus Initialize() noexcept override
        { 
            return RSuccess; 
        }

        const char *GetName() const noexcept override
        { 
            return "[ThreadLocalMemoryManager]"; 
        }

        //! Preallocate all pools
        SKL_FORCEINLINE static RStatus Preallocate() noexcept
        {
            auto* Instance{ ThreadLocalMemoryManager::GetInstance() }; SKL_ASSERT( nullptr != Instance );
            return Instance->Manager.Preallocate();
        }        

        //! Zero memory all pools, this will force the OS to have the all pages ready in memory (hot)
        SKL_FORCEINLINE static void ZeroAllMemory() noexcept
        {
            auto* Instance{ ThreadLocalMemoryManager::GetInstance() }; SKL_ASSERT( nullptr != Instance );
            Instance->Manager.ZeroAllMemory();
        }
        
        //! Zero memory all pools, this will force the OS to have the all pages ready in memory (hot)
        SKL_FORCEINLINE static void FreeAllPools() noexcept
        {
            auto* Instance{ ThreadLocalMemoryManager::GetInstance() }; SKL_ASSERT( nullptr != Instance );
            Instance->Manager.FreeAllPools();
        }
        
        //! Allocate new memory block with the size known at compile time
        template<size_t AllocSize>
        SKL_FORCEINLINE static AllocResult Allocate() noexcept
        {
            auto* Instance{ ThreadLocalMemoryManager::GetInstance() }; SKL_ASSERT( nullptr != Instance );
            return Instance->Manager.Allocate<AllocSize>();
        }
        
        //! Allocate new memory block with the size known at run time
        SKL_FORCEINLINE static AllocResult Allocate( size_t AllocSize ) noexcept
        {
            auto* Instance{ ThreadLocalMemoryManager::GetInstance() }; SKL_ASSERT( nullptr != Instance );
            return Instance->Manager.Allocate( AllocSize );
        }
        
        //! Deallocate memory block with the size known at compile time
        template<size_t AllocSize>
        SKL_FORCEINLINE static void Deallocate( void* InPtr ) noexcept
        {
            auto* Instance{ ThreadLocalMemoryManager::GetInstance() }; SKL_ASSERT( nullptr != Instance );
            Instance->Manager.Deallocate<AllocSize>( InPtr );
        }
        
        //! Deallocate memory block with the size known at run time
        SKL_FORCEINLINE static void Deallocate( void* InPtr, size_t AllocSize ) noexcept
        {
            auto* Instance{ ThreadLocalMemoryManager::GetInstance() }; SKL_ASSERT( nullptr != Instance );
            return Instance->Manager.Deallocate( InPtr, AllocSize );
        }

        //! Deallocate memory block with the size known at run time
        SKL_FORCEINLINE static void Deallocate( AllocResult& InAllocResult ) noexcept
        {
            auto* Instance{ ThreadLocalMemoryManager::GetInstance() }; SKL_ASSERT( nullptr != Instance );
            Instance->Manager.Deallocate( InAllocResult );
        }

        //! Deallocate memory block with the size known at run time
        SKL_FORCEINLINE static void Deallocate( AllocResult* InAllocResult ) noexcept
        {
            auto* Instance{ ThreadLocalMemoryManager::GetInstance() }; SKL_ASSERT( nullptr != Instance );
            Instance->Manager.Deallocate( InAllocResult );
        }

        //! Get profiling data
        SKL_NODISCARD static auto GetProfilingData() noexcept -> MemoryManager::TProfilingData
        {
            auto* Instance{ ThreadLocalMemoryManager::GetInstance() }; SKL_ASSERT( nullptr != Instance );
            return Instance->Manager.ProfilingData;
        }

        SKL_FORCEINLINE SKL_NODISCARD MemoryManager& GetManager() noexcept{ return Manager; }
        SKL_FORCEINLINE SKL_NODISCARD const MemoryManager& GetManager() const noexcept{ return Manager; }

        ThreadLocalMemoryManager() noexcept = default;
        ~ThreadLocalMemoryManager() noexcept = default;

    private:
        MemoryManager Manager {};

        friend ITLSSingleton<ThreadLocalMemoryManager>;
    };  
}