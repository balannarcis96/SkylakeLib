#include <gtest/gtest.h>

#include <SkylakeLib.h>

namespace TCPAcceptorTests
{
    TEST( TCPAcceptorTests, SkylakeLib_Initialize_And_Termiante )
    {
        ASSERT_TRUE( SKL::RSuccess == SKL::Skylake_InitializeLibrary( 0, nullptr, nullptr ) );     
        ASSERT_TRUE( SKL::RSuccess == SKL::Skylake_TerminateLibrary() );
    }

    TEST( TCPAcceptorTests, DISABLED_AcceptAsync_Start_Stop )
    {
        ASSERT_TRUE( SKL::RSuccess == SKL::Skylake_InitializeLibrary( 0, nullptr, nullptr ) );     

        SKL::ServerInstanceConfig::ServerInstanceConfig AppWorkersConfig{ L"AcceptAsync_Start_Stop_App_WorkersGroups" };
        SKL::ServerInstanceConfig::WorkerGroupConfig    WGConfig        {};

        SKL::WorkerGroupTag Tag
        {
            .TickRate        = 5,
            .SyncTLSTickRate = 5,
            .Id              = 1,
            .WorkersCount    = 1,
            .Name            = L"AcceptAsync_Start_Stop_WorkerGroup"
        };

        Tag.bIsActive = true;
        Tag.bEnableAsyncIO = true;

        //Config the workers group
        WGConfig.SetTag( Tag );
        WGConfig.SetWorkerTickHandler( []( SKL::Worker&, SKL::WorkerGroup& ) noexcept -> void { } );

        //Config the tcp async acceptor
        SKL::TCPAcceptorConfig TCPAsyncAcceptorConfig
        {
            .Id         = 1,
            .IpAddress  = SKL::IPv4FromStringA( "127.0.0.1" ),
            .Port       = 11011,
            .Backlog    = 100
        };

        TCPAsyncAcceptorConfig.SetOnAcceptHandler( []( SKL::TSocket InAcceptedSocket ) noexcept
        {
            ASSERT_TRUE( true == SKL::IsValidSocket( InAcceptedSocket ) );
            SKLL_INF( "New tcp socket accepted!" );
            SKL::CloseSocket( InAcceptedSocket );
            SKL::ShutdownSocket( InAcceptedSocket );
        } );

        WGConfig.AddTCPAsyncAcceptor( TCPAsyncAcceptorConfig );
        AppWorkersConfig.AddNewGroup( std::move( WGConfig ) );

        SKL::ServerInstance WGManager { };
        ASSERT_TRUE( SKL::RSuccess == WGManager.Initialize( std::move( AppWorkersConfig ) ) );

        ASSERT_TRUE( SKL::RSuccess == WGManager.StartServer() );

        ASSERT_TRUE( SKL::RSuccess == SKL::Skylake_TerminateLibrary() );
    }
}

int main( int argc, char** argv )
{
    testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS( );
}
