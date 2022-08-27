#include <gtest/gtest.h>

#include <SkylakeLib.h>

namespace ECSTests
{
    struct Component1
    {
        int a { 0 };
    };

    struct Component2
    {
        int b{ 0 };
        int c{ 1 };
    };

    using MyStore = SKL::SymmetricStore<uint16_t
                                      , 1024
                                      , Component1
                                      , Component2>;

    TEST( ECSTests, ECS_SymmetricStore )
    {
       MyStore Store{};
       
       ASSERT_TRUE( 1024 == MyStore::Traits::EntitiesCount );

       for( uint16_t i = 0; i < 1024; ++i )
       {
            ASSERT_TRUE( 1 == Store.GetComponent<Component2>( i ).c );
       }
    }
}

int main( int argc, char** argv )
{
    testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS( );
}