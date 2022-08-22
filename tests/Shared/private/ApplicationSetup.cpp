#include "ApplicationSetup.h"


bool TestApplication::Start( bool bIncludeCallignThread )
{
    if( false == Initialize() )
    {
        return false;
    }

    ServerInstanceConfig.SetWillCaptureCallingThread( bIncludeCallignThread );
    
    SKL::ServerInstanceConfig::ServerInstanceConfig ConfigCopy{ ServerInstanceConfig };
    
    if( RSuccess != ServerInstance.Initialize( std::move( ConfigCopy ) ) )
    {
        return false;
    }
    
    return ServerInstance.StartServer() == ( bIncludeCallignThread ? RServerInstanceFinalized : RSuccess );
}

void TestApplication::SignalToStop()
{
    ServerInstance.SignalToStop();
}

bool TestApplication::Stop()
{
    ServerInstance.SignalToStop();

    ServerInstance.JoinAllGroups();
    return true;
}
