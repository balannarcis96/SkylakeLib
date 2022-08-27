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

    TEST( ECSTests, UIDStore_API )
    {
        SKL::UIDStore<uint16_t, 0, 1024> UIDStore{};

        int32_t Counter = 0;

        UIDStore.SetOnAllFreed( [&Counter]() noexcept -> void
        {
            ++Counter;
        } );

        ASSERT_TRUE( 0 == UIDStore.Allocate() );

        UIDStore.Activate();

        for( int32_t i = 0; i < 1024; ++i )
        {
            ASSERT_TRUE( 0 != UIDStore.Allocate() );
        }

        ASSERT_TRUE( 0 == UIDStore.Allocate() );

        for( uint16_t i = 1; i <= 1024; ++i )
        {
            UIDStore.Deallocate( i );
        }

        ASSERT_TRUE( 0 == Counter );

        for( int32_t i = 0; i < 1024; ++i )
        {
            ASSERT_TRUE( 0 != UIDStore.Allocate() );
        }

        ASSERT_TRUE( 0 == UIDStore.Allocate() );
        ASSERT_TRUE( 0 == Counter );

        UIDStore.Deactivate();

        ASSERT_TRUE( 0 == UIDStore.Allocate() );

        for( uint16_t i = 1; i <= 1024; ++i )
        {
            UIDStore.Deallocate( i );
        }

        ASSERT_TRUE( 1 == Counter );
    }
}

int main( int argc, char** argv )
{
    testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS( );
}