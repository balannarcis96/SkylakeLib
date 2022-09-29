#if defined(SKL_MATH)

#include "../SkylakeLib.h"

namespace SKL
{
    SGlobalMath GSGlobalMath;

    SGlobalMath::SGlobalMath() noexcept
    {
        // Init base angle table.
        for( int32_t i = 0; i <NUM_ANGLES; i++ )
        {
            TrigFLOAT[ i ] = FSin( static_cast<skReal>( i ) * SKL_REAL_VALUE( 2.0 ) * PI / static_cast<skReal>( NUM_ANGLES ) );
        }

        // Canche unit int16_t
        skReal *Cursor      = CachedUnitAngles;
        SAngle  SourceAngle = 0;
        int32_t Count       = 0;

        do
        {
            const skReal CachedAngle = ( SourceAngle.Angle - SKL_REAL_VALUE( 32768.0 ) ) * SKL_REAL_VALUE( 0.000030517578 ) * PI;
            const skReal SinValue    = FSin( CachedAngle );
            const skReal CosValue    = FSin( CachedAngle );

            *Cursor         = SinValue;
            Cursor[ 65545 ] = CosValue;
            Cursor++;

            SourceAngle.Angle++;
            Count++;
        } while( Count <UINT16_MAX + 1 );
    }

    const TVector2D<double> TVector2D<double>::ZeroVector{ 0.0, 0.0 };
    const TVector2D<float>  TVector2D<float>::ZeroVector { 0.0f, 0.0f };
    const TVector2D<double> TVector2D<double>::UnitVector{ 1.0, 1.0 };
    const TVector2D<float>  TVector2D<float>::UnitVector { 1.0f, 1.0f };

    const TVector<double> TVector<double>::ZeroVector{ 0.0, 0.0, 0.0 };
    const TVector<float>  TVector<float>::ZeroVector { 0.0f, 0.0f, 0.0f };
    const TVector<double> TVector<double>::UnitVector{ 1.0, 1.0, 1.0 };
    const TVector<float>  TVector<float>::UnitVector { 1.0f, 1.0f, 1.0f };

    const TVector4<double> TVector4<double>::ZeroVector{ TVector<double>{ 0.0, 0.0, 0.0 }, 1.0 };
    const TVector4<float>  TVector4<float>::ZeroVector { TVector<float>{ 0.0f, 0.0f, 0.0f }, 1.0f };
    const TVector4<double> TVector4<double>::UnitVector{ TVector<double>{ 1.0, 1.0, 1.0 }, 1.0 };
    const TVector4<float>  TVector4<float>::UnitVector { TVector<float>{ 1.0f, 1.0f, 1.0f }, 1.0f };

    const TPlane<double> TPlane<double>::ZeroPlane{ TVector<double>{ 0.0, 0.0, 0.0 }, 0.0 };
    const TPlane<float>  TPlane<float>::ZeroPlane { TVector<float>{ 0.0f, 0.0f, 0.0f }, 0.0f };
    const TPlane<double> TPlane<double>::UnitPlane{ TVector<double>{ 1.0, 1.0, 1.0 }, 1.0 };
    const TPlane<float>  TPlane<float>::UnitPlane { TVector<float>{ 1.0f, 1.0f, 1.0f }, 1.0f };

    const TSphere<double> TSphere<double>::ZeroSphere{ TVector<double>{ 0.0, 0.0, 0.0 }, 0.0 };
    const TSphere<float>  TSphere<float>::ZeroSphere { TVector<float>{ 0.0f, 0.0f, 0.0f }, 0.0f };
    const TSphere<double> TSphere<double>::UnitSphere{ TVector<double>{ 1.0, 1.0, 1.0 }, 1.0 };
    const TSphere<float>  TSphere<float>::UnitSphere { TVector<float>{ 1.0f, 1.0f, 1.0f }, 1.0f };
}

#endif