#include <gtest/gtest.h>

#include <SkylakeLib.h>

namespace WorkersTests
{
    struct MyType { int a; };

    TEST( WorkersTests, MainTest )
    {
        SKL::Skylake_InitializeLibrary( 0, nullptr, nullptr );

        SKL::ApplicationWorkersConfig Config { L"TEST_APPLICATION" };

        SKL::ApplicationWorkerGroupConfig Group1
        {
            SKL::WorkerGroupTag 
            {
                .TickRate         = 60, 
                .SyncTLSTickRate  = 0,
                .Id               = 1,
                .WorkersCount     = 2,
                .bIsActive        = true,
                .bHandlesTasks    = true,
                .bSupportsTLSSync = false,
                .Name             = L"FRONT_END_GROUP"
            }
        };
        Group1.SetWorkerTickHandler( []( SKL::Worker& Worker, SKL::WorkerGroup& Group ) noexcept -> void 
        {
            SKL_INF( "Worker Group1 Tick()" );
            Group.SignalToStop();
        } );
        Config.AddNewGroup( std::move( Group1 ) );

        SKL::ApplicationWorkerGroupConfig Group2
        {
            SKL::WorkerGroupTag 
            {
                .TickRate         = 60, 
                .SyncTLSTickRate  = 0,
                .Id               = 2,
                .WorkersCount     = 2,
                .bIsActive        = true,
                .bHandlesTasks    = true,
                .bSupportsTLSSync = false,
                .Name             = L"BACK_END_GROUP"
            }
        };
        Group2.SetWorkerTickHandler( []( SKL::Worker& Worker, SKL::WorkerGroup& Group ) noexcept -> void 
        {
            SKL_INF( "Worker Group2 Tick()" );
            Group.SignalToStop();
        } );
        Config.AddNewGroup( std::move( Group2 ) );

        SKL::WorkerGroupManager Manager;

        auto Result = Manager.Initialize( std::move( Config ) );
        ASSERT_TRUE( RSuccess == Result );

        auto QueryResult = Manager.GetWorkerGroupById( 1 );
        ASSERT_TRUE( nullptr != QueryResult.get() );
        ASSERT_TRUE( 1 == QueryResult->GetTag().Id );

        QueryResult = Manager.GetWorkerGroupById( 2 );
        ASSERT_TRUE( nullptr != QueryResult.get() );
        ASSERT_TRUE( 2 == QueryResult->GetTag().Id );

        Manager.StartRunningWithCallingThreadAsMaster();
    
        SKL::Skylake_TerminateLibrary();
    }
}

int main( int argc, char** argv )
{
    testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS( );
}