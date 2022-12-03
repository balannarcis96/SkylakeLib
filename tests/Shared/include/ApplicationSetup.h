#include <gtest/gtest.h>
#include <SkylakeLib.h>

class TestApplication : public SKL::ServerInstance 
{
public:
    TestApplication( const wchar_t* Name ) noexcept 
        : ServerInstance{} 
        , ServerInstanceConfig{ Name }
    {}

    virtual bool Start( bool bIncludeCallignThread = true ) noexcept;
    virtual bool Stop() noexcept;

    template<typename TFunctor>
    bool AddNewWorkerGroup( const SKL::WorkerGroupTag& InTag, TFunctor&& InOnTickFunctor ) noexcept
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

        return true;
    }

protected:
    virtual bool InitializeTestApplication() noexcept  { return true; }

    SKL::ServerInstanceConfig::ServerInstanceConfig ServerInstanceConfig;
};

