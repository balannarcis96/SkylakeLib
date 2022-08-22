#include <gtest/gtest.h>
#include <SkylakeLib.h>

class TestApplication
{
public:
    TestApplication( const wchar_t* Name ) : 
        ServerInstanceConfig{ Name },
        ServerInstance{}
    {}

    virtual bool Start( bool bIncludeCallignThread = true );
    virtual void SignalToStop();
    virtual bool Stop();

    template<typename TFunctor>
    bool AddNewWorkerGroup( const SKL::WorkerGroupTag& InTag, TFunctor&& InOnTickFunctor )
    {
        InTag.Validate();

        if( false == InTag.IsValid() )
        {
            return false;
        }

        SKL::ServerInstanceConfig::WorkerGroupConfig WGConfig{ InTag };  

        WGConfig.SetWorkerTickHandler( std::forward<TFunctor>( InOnTickFunctor ) );

        if( false == WGConfig.IsValid() )
        {
            return false;
        }

        ServerInstanceConfig.AddNewGroup( std::move( WGConfig ) );
        
        if( false == WGConfig.IsValid() )
        {
            return false;
        }

        return true;
    }
protected:
    virtual bool Initialize() = 0;

    SKL::ServerInstanceConfig::ServerInstanceConfig ServerInstanceConfig;
    SKL::ServerInstance                             ServerInstance;
};

