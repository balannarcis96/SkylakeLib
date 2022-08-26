//!
//! \file SkylakeLib.cpp
//! 
//! \brief Implementation for the SkylakeLib
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#include "SkylakeLib.h"

namespace SKL
{
    using SkylakeLibInitPerThread = TLSValue<bool, 55114>;

    std::relaxed_value<FILE*> GLogOutput { stdout }; //!< Defaulted to stdout
    std::relaxed_value<BOOL>  GIsInit    { FALSE };  //!< Is the SkylakeLib init

    void ValidatePlatformRuntime() noexcept
    {
        const auto L1CacheLineSize { GetL1CacheLineSize() };
        if ( SKL_CACHE_LINE_SIZE != L1CacheLineSize )
        {
            SKL_ERR_FMT( "Expected L1 cache line size to be %d but it is %llu, PLATFORM NOT SUPPORTED, REBUILD!", static_cast<int32_t>( SKL_CACHE_LINE_SIZE ), L1CacheLineSize );
        }

        SKL_ASSERT_ALLWAYS_MSG( SKL_CACHE_LINE_SIZE == L1CacheLineSize, "Unsupported l1 cache line size!" );
    }

    RStatus Skylake_InitializeLibrary( int32_t Argc, char** Argv, FILE* InLogOutput ) noexcept 
    {
        ValidatePlatformRuntime();

        if( TRUE == GIsInit.load_relaxed() )
        {
            SKL_WRN( "The SkylakeLib was already initialized" );
            return RSuccess;
        }

        if( nullptr != InLogOutput )
        {
            // set the log output file handle
            GLogOutput.exchange( InLogOutput );
        }

        auto Result = AsyncIO::InitializeSystem();
        if( RSuccess != Result ) SKL_UNLIKELY
        {
            SKL_ERR( "Failed to initialize the async io system!" );
            return Result;
        }

        Result = Skylake_InitializeLibrary_Thread();
        if( RSuccess != Result ) SKL_UNLIKELY
        {
            SKL_ERR( "Failed to initialize the SkylakeLibrary for the main thread!" );
            return Result;
        }

        GIsInit.exchange( true );
        return RSuccess;
    }

    RStatus Skylake_TerminateLibrary() noexcept
    {
        if( FALSE == GIsInit.load_relaxed() )
        {
            SKL_WRN( "The SkylakeLib was already terminated" );
            return RSuccess;
        }

        auto Result = AsyncIO::ShutdownSystem();
        if( RSuccess != Result ) SKL_UNLIKELY
        {
            SKL_ERR( "Failed to shutdown the async io system!" );
            return Result;
        }

        Result = Skylake_TerminateLibrary_Thread();
        if( RSuccess != Result ) SKL_UNLIKELY
        {
            SKL_ERR( "Failed to terminate the SkylakeLibrary for the main thread!" );
            return Result;
        }

        GlobalMemoryManager::FreeAllPools();

        GIsInit.exchange( false );
        return RSuccess;
    }

    RStatus Skylake_InitializeLibrary_Thread() noexcept 
    {
        if( true == SkylakeLibInitPerThread::GetValue() )
        {
            SKL_INF( "[Skylake_InitializeLibrary_Thread()] The SkylakeLib was already init on this thread!" );
            return RSuccess;
        }

        if( nullptr == ThreadLocalMemoryManager::GetInstance() )
        {
            if( RSuccess != ThreadLocalMemoryManager::Create() )
            {
                SKL_INF( "[Skylake_InitializeLibrary_Thread()] The SkylakeLib failed to create the ThreadLocalMemoryManager!" );
                return RSuccess;
            }

            SKL_VER( "Skylake_InitializeLibrary_Thread() Created ThreadLocalMemoryManager." );
        }

        SkylakeLibInitPerThread::SetValue( true );

        return RSuccess;
    }

    RStatus Skylake_TerminateLibrary_Thread() noexcept
    {
        if( false == SkylakeLibInitPerThread::GetValue() )
        {
            SKL_INF( "[Skylake_TerminateLibrary_Thread()] The SkylakeLib was already terminated on this thread!" );
            return RSuccess;
        }

        if( nullptr != SKL::ThreadLocalMemoryManager::GetInstance() )
        {
            SKL::ThreadLocalMemoryManager::FreeAllPools();
            SKL::ThreadLocalMemoryManager::Destroy();
        }

        SkylakeLibInitPerThread::SetValue( false );

        return RSuccess;
    }

    bool Skylake_IsTheLibraryInitialize() noexcept
    {
        return GIsInit.load_relaxed() == TRUE;
    }

    bool Skylake_IsTheLibraryInitialize_Thread() noexcept
    {
        return SkylakeLibInitPerThread::GetValue();
    }
}

//Memory Manager
namespace SKL
{
    SKL_IFMEMORYSTATS( SKL_CACHE_ALIGNED std::atomic<size_t> SkylakeGlobalMemoryManager::CustomSizeAllocations  { 0 } );
    SKL_IFMEMORYSTATS( SKL_CACHE_ALIGNED std::atomic<size_t> SkylakeGlobalMemoryManager::CustomSizeDeallocations{ 0 } );
    SKL_IFMEMORYSTATS( SKL_CACHE_ALIGNED std::atomic<size_t> SkylakeGlobalMemoryManager::TotalAllocations       { 0 } );
    SKL_IFMEMORYSTATS( SKL_CACHE_ALIGNED std::atomic<size_t> SkylakeGlobalMemoryManager::TotalDeallocations     { 0 } );
}