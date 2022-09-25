#include "ApplicationSetup.h"

bool TestApplication::Start( bool bIncludeCallignThread ) noexcept
{
    ServerInstanceConfig.SetWillCaptureCallingThread( bIncludeCallignThread );
    SKL::ServerInstanceConfig::ServerInstanceConfig ConfigCopy{ ServerInstanceConfig };
    if( SKL::RSuccess != Initialize( std::move( ConfigCopy ) ) )
    {
        return false;
    }

    if( true != InitializeTestApplication() )
    {
        return false;
    }
    
    return StartServer() == ( bIncludeCallignThread ? SKL::RServerInstanceFinalized : SKL::RSuccess );
}

bool TestApplication::Stop() noexcept
{
    SignalToStop();
    JoinAllGroups();

    return true;
}
