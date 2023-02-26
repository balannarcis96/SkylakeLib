#include <gtest/gtest.h>

#include <SkylakeLib.h>

namespace AsyncIOTestsSuite
{
    TEST( AsyncIOTestsSuite, InitializeSystem_And_ShutdownSystem )
    {
        auto Result = SKL::AsyncIO::InitializeSystem();
        ASSERT_TRUE( SKL::RSuccess == Result );
        
        Result = SKL::AsyncIO::ShutdownSystem();
        ASSERT_TRUE( SKL::RSuccess == Result );
    }

    TEST( AsyncIOTestsSuite, Start_Stop_Instance )
    {
        auto Result = SKL::AsyncIO::InitializeSystem();
        ASSERT_TRUE( SKL::RSuccess == Result );
        
        SKL::AsyncIO Instance;

        Result = Instance.Start( 1 );
        ASSERT_TRUE( SKL::RSuccess == Result );

        Result = Instance.Stop();
        ASSERT_TRUE( SKL::RSuccess == Result );

        Result = SKL::AsyncIO::ShutdownSystem();
        ASSERT_TRUE( SKL::RSuccess == Result );
    }

    TEST( AsyncIOTestsSuite, Timeout_TryGetCompletedAsyncRequest_Instance )
    {
        auto Result = SKL::SetOsTimeResolution( 1 );
        ASSERT_TRUE( SKL::RSuccess == Result );

        Result = SKL::AsyncIO::InitializeSystem();
        ASSERT_TRUE( SKL::RSuccess == Result );
        
        SKL::AsyncIO Instance;

        Result = Instance.Start( 1 );
        ASSERT_TRUE( SKL::RSuccess == Result );

        SKL::AsyncIOOpaqueType* OpaqueInstance             {};
        uint32_t                NumberOfBytesTransferred { 0 };
        SKL::TCompletionKey     CompletionKey             { nullptr };

        const auto Before = SKL::GetSystemUpTickCount();

        Result = Instance.TryGetCompletedAsyncRequest( &OpaqueInstance, &NumberOfBytesTransferred, &CompletionKey, 10 );
        ASSERT_TRUE( SKL::RTimeout == Result );

        const auto After = SKL::GetSystemUpTickCount();
        const auto Diff = After - Before;
        ASSERT_TRUE( Diff <= 10 + 18 );

        Result = Instance.Stop();
        ASSERT_TRUE( SKL::RSuccess == Result );

        Result = SKL::AsyncIO::ShutdownSystem();
        ASSERT_TRUE( SKL::RSuccess == Result );
    }

    TEST( AsyncIOTestsSuite, Block_GetCompletedAsyncRequest_Instance )
    {
        auto Result = SKL::SetOsTimeResolution( 1 );
        ASSERT_TRUE( SKL::RSuccess == Result );

        Result = SKL::AsyncIO::InitializeSystem();
        ASSERT_TRUE( SKL::RSuccess == Result );
        
        SKL::AsyncIO Instance;

        Result = Instance.Start( 1 );
        ASSERT_TRUE( SKL::RSuccess == Result );

        SKL::AsyncIOOpaqueType* OpaqueInstance             {};
        uint32_t                NumberOfBytesTransferred { 0 };
        SKL::TCompletionKey     CompletionKey             { nullptr };

        std::jthread stopThread([&Instance](){
            std::this_thread::sleep_for( TCLOCK_MILLIS( 10 ) );
            const auto Result = Instance.Stop();
            ASSERT_TRUE( SKL::RSuccess == Result );
        });    

        Result = Instance.GetCompletedAsyncRequest( &OpaqueInstance, &NumberOfBytesTransferred, &CompletionKey );
        ASSERT_TRUE( SKL::RSystemFailure == Result );

        Result = Instance.Stop();
        ASSERT_TRUE( SKL::RAlreadyPerformed == Result );

        Result = SKL::AsyncIO::ShutdownSystem();
        ASSERT_TRUE( SKL::RSuccess == Result );
    }

    TEST( AsyncIOTestsSuite, Block_GetCompletedAsyncRequest_ValidWork )
    {
        struct CustomWorkType
        {
            int a ;
        };

        auto Result = SKL::SetOsTimeResolution( 1 );
        ASSERT_TRUE( SKL::RSuccess == Result );

        Result = SKL::AsyncIO::InitializeSystem();
        ASSERT_TRUE( SKL::RSuccess == Result );
        
        SKL::AsyncIO Instance;

        Result = Instance.Start( 2 );
        ASSERT_TRUE( SKL::RSuccess == Result );

        SKL::AsyncIOOpaqueType* OpaqueInstance           {};
        uint32_t                NumberOfBytesTransferred { 0 };
        SKL::TCompletionKey     CompletionKey            { nullptr };

        std::jthread stopThread( [&Instance](){
            std::this_thread::sleep_for( TCLOCK_MILLIS( 10 ) );
        
            auto* ptr = new CustomWorkType { 10 };
            ASSERT_TRUE( nullptr != ptr );

            const auto Result = Instance.QueueAsyncWork( reinterpret_cast<SKL::TCompletionKey>( ptr ) );
            ASSERT_TRUE( SKL::RSuccess == Result );
        } );    

        Result = Instance.GetCompletedAsyncRequest( &OpaqueInstance, &NumberOfBytesTransferred, &CompletionKey );
        ASSERT_TRUE( SKL::RSuccess == Result );
        ASSERT_TRUE( nullptr != CompletionKey );

        auto* ptr = reinterpret_cast<CustomWorkType*>( CompletionKey );
        ASSERT_TRUE( 10 == ptr->a );
        delete ptr;

        Result = Instance.Stop();
        ASSERT_TRUE( SKL::RSuccess == Result );

        Result = SKL::AsyncIO::ShutdownSystem();
        ASSERT_TRUE( SKL::RSuccess == Result );
    }
}

int main( int argc, char** argv )
{
    testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS( );
}
