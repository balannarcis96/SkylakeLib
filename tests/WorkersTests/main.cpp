#include <gtest/gtest.h>

#include <SkylakeLib.h>

namespace WorkersTests
{
    struct MyType { 
        void operator()( SKL::Worker& Worker, SKL::WorkerGroup& Group ) noexcept {};
        
        int a;
    };

    TEST( WorkersTests, MainTest )
    {
        SKL::Skylake_InitializeLibrary( 0, nullptr, nullptr );

        SKL::WorkerGroupManager Manager;

        SKL::ApplicationWorkersConfig Config { L"TEST_APPLICATION" };

        SKL::ApplicationWorkerGroupConfig Group1
        {
            SKL::WorkerGroupTag 
            {
                .TickRate         = 5, 
                .SyncTLSTickRate  = 0,
                .Id               = 1,
                .WorkersCount     = 1,
                .bIsActive        = true,
                .bHandlesTasks    = true,
                .bSupportsTLSSync = false,
                .Name             = L"FRONT_END_GROUP"
            }
        };

        using TT = ASD::CopyFunctorWrapper<16, void(*)(void)> ;        

        Group1.SetWorkerStartHandler( []( SKL::Worker& Worker, SKL::WorkerGroup& Group ) mutable noexcept -> void {
            SKL_INF( "Worker Group1 WORKER STARTED!" );
        } ); 

        Group1.SetWorkerTickHandler( [ &Manager ]( SKL::Worker& Worker, SKL::WorkerGroup& Group ) mutable noexcept -> void
        {
            SKL_INF( "Worker Group1 Tick()" );

            if( Worker.GetAliveTime() < 1000 )
            {
                return;
            }

            Manager.GetWorkerGroupById( 2 )->Deferre( [ &Manager ]() noexcept {
                SKL_INF( "From TASK" );
                Manager.SignalToStop();
            } );
            
            TCLOCK_SLEEP_FOR_MILLIS( 50 );
        } );
        Config.AddNewGroup( std::move( Group1 ) );

        SKL::ApplicationWorkerGroupConfig Group2
        {
            SKL::WorkerGroupTag 
            {
                .TickRate         = 5, 
                .SyncTLSTickRate  = 0,
                .Id               = 2,
                .WorkersCount     = 2,
                .bIsActive        = false,
                .bHandlesTasks    = true,
                .bSupportsTLSSync = false,
                .Name             = L"BACK_END_GROUP"
            }
        };
        Group2.SetWorkerTickHandler( [ &Manager ]( SKL::Worker& Worker, SKL::WorkerGroup& Group ) mutable noexcept -> void
        {
            SKL_INF( "Worker Group2 Tick()" );
        } );
        Config.AddNewGroup( std::move( Group2 ) );

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