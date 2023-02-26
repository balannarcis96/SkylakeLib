#include <gtest/gtest.h>

#include <SkylakeLib.h>

namespace MathTests
{
    TEST( Math_Tests_Suite, SVector____TestCase )
    {
        {
            SKL::SVector Vec;
            ASSERT_TRUE( CRealZero == Vec.X );
            ASSERT_TRUE( CRealZero == Vec.X );
            ASSERT_TRUE( CRealZero == Vec.X );

            ASSERT_TRUE( true == Vec.IsZero() );
            ASSERT_TRUE( true == Vec.IsNearlyZero() );
        }

        {
            SKL::SPoint Item { 2.5, 3.5 };
            ASSERT_EQ( false, Item.HasIntegerValues() );
            Item.Floor();
            ASSERT_EQ( Item.X, 2.0 );
            ASSERT_EQ( Item.Y, 3.0 );
            ASSERT_EQ( true, Item.HasIntegerValues() );
        }

        {
            SKL::SVector Vec { 2.5, 3.5, 0.22 };
            ASSERT_EQ( false, Vec.HasIntegerValues() );
            Vec.Floor();
            ASSERT_EQ( Vec.X, 2.0 );
            ASSERT_EQ( Vec.Y, 3.0 );
            ASSERT_EQ( Vec.Z, 0.0 );
            ASSERT_EQ( true, Vec.HasIntegerValues() );
        }
    }

    //@TODO more math abstractions tests
}

int main( int argc, char** argv )
{
    testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS( );
}
