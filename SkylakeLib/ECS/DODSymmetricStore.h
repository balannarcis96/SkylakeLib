//!
//! \file DODSymmetricStore.h
//! 
//! \brief (Data Oriented Desing)Symmetric Store
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    template<typename TIndexType, TIndexType TCount, typename ...TComponents>
    struct SymmetricStore: public MultiArray<TCount, TComponents...>
    {
        struct Traits
        {
            using AllComponents = std::tuple<TComponents...>;
            using IndexType     = TIndexType;

            static constexpr IndexType EntitiesCount = TCount;
        };

        template<typename TComponent>
        SKL_FORCEINLINE TComponent& GetComponent( TIndexType InIndex ) noexcept
        {
            SKL_ASSERT( Traits::EntitiesCount > InIndex );
            return this->template GetArray<TComponent>()[ InIndex ];
        }

        template<typename TComponent>
        SKL_FORCEINLINE const TComponent& GetComponent( TIndexType InIndex ) const noexcept
        {
            SKL_ASSERT( Traits::EntitiesCount > InIndex );
            return this->template GetArray<TComponent>()[ InIndex ];
        }
    };
}
