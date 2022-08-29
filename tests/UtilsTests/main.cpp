#include <gtest/gtest.h>

#include <SkylakeLib.h>

namespace UtilsTests
{
    TEST( UtilsTests, GRand_API )
    {
        constexpr size_t IterCount{ 1024 };

        auto RandBuffer  { std::make_unique<uint32_t[]>( IterCount ) };
        auto RandBuffer2 { std::make_unique<uint32_t[]>( IterCount ) };
        auto RandBufferF { std::make_unique<float[]>( IterCount ) };
        auto RandBufferD { std::make_unique<double[]>( IterCount ) };

        memset( RandBuffer.get(), 0, sizeof( uint32_t ) * IterCount );
        memset( RandBuffer2.get(), 0, sizeof( uint32_t ) * IterCount );
        memset( RandBufferF.get(), 0, sizeof( float ) * IterCount );
        memset( RandBufferD.get(), 0, sizeof( double ) * IterCount );

        for( size_t i = 0; i < IterCount; ++i )
        {
            RandBuffer[ i ]  = SKL::GRand::NextRandom();
            RandBuffer2[ i ] = SKL::GRand::NextRandomInRange( 0, std::numeric_limits<uint32_t>::max() / 2 );
            RandBufferF[ i ] = SKL::GRand::NextRandomF();
            RandBufferD[ i ] = SKL::GRand::NextRandomD();
        }
        
        for( size_t i = 0; i < IterCount; ++i )
        {
            for( size_t j = 0; j < IterCount; ++j )
            {
                if( j == i ) continue;  
                ASSERT_TRUE( RandBuffer[ i ] != RandBuffer[ j ] );
                ASSERT_TRUE( RandBuffer2[ i ] != RandBuffer2[ j ] );
                ASSERT_TRUE( RandBufferF[ i ] != RandBufferF[ j ] );
                ASSERT_TRUE( RandBufferD[ i ] != RandBufferD[ j ] );
            }
        }
    }
}

int main( int argc, char** argv )
{
    testing::InitGoogleTest( &argc, argv );
    return RUN_ALL_TESTS( );
}