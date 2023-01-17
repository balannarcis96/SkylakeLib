#include <gtest/gtest.h>

#include <SkylakeLib.h>

namespace MathTests
{
    TEST( MathTestsSuite, SVector_TestCase )
    {
        {
            SKL::SVector Vec;
            ASSERT_TRUE( CRealZero == Vec.X );
            ASSERT_TRUE( CRealZero == Vec.X );
            ASSERT_TRUE( CRealZero == Vec.X );

            ASSERT_TRUE( true == Vec.IsZero() );
            ASSERT_TRUE( true == Vec.IsNearlyZero() );
        }
    }

    //@TODO more math abstractions tests
}

int main( int argc, char** argv )
{
    testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS( );
}