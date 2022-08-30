//!
//! \file EntityId.h
//! 
//! \brief Entity Id abstraction
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    using TEntityType                        = uint8_t;
    using TEntityIdBase                      = uint64_t;
    constexpr TEntityIdBase CInvalidEntityId = 0;
    constexpr TEntityType CInvalidEntityType = 0;

    template<typename Variant, bool bExtendedIndex = false, bool bAtomic = false>
    struct EntityId
    {
        using TIndexType = std::conditional_t<true == bExtendedIndex, uint32_t, uint16_t>;
        static constexpr uint32_t CBasicIdMask        = 0xFFFF0000U;
        static constexpr TIndexType CBasicIdMaxValue  = 0xFFFFU;
        static constexpr uint32_t CExtendedIdMask     = 0xFFFFFF00U;
        static constexpr uint32_t CExtendedIdMaxValue = 0x00FFFFFFU;
        using TBaseType  = std::conditional_t<true == bAtomic, std::relaxed_value<TEntityIdBase>, TEntityIdBase>;

        static_assert( sizeof( Variant ) == sizeof( uint32_t ) );
        
        struct Description
        {
            Description( TEntityIdBase Id ) noexcept : Id{ Id }{}
            ~Description() noexcept = default;

            union 
            {
                struct 
                {
                    TEntityType Type;
                    uint8_t     ExtendedIndexPart;
                    uint16_t    BasicIndexPart;
                    uint32_t    Component;
                };
                
                TEntityIdBase Id;
            };
        };

        constexpr EntityId() noexcept : Id{ 0 }  {}
        constexpr EntityId( TEntityIdBase Id ) noexcept : Id{ Id }  {}
        constexpr EntityId( TEntityType Type, TIndexType Index, Variant InVariant ) noexcept  
        {
            if constexpr( bAtomic )
            {
                using TempId = EntityId<Variant, bExtendedIndex, false>;
                TempId Temp{ Type };
                auto& Desc{ Temp.GetDescription() };
                Desc.Component = *reinterpret_cast<uint32_t*>( &InVariant );

                if constexpr( true == bExtendedIndex )
                {
                    SKL_ASSERT_ALLWAYS( Index <= CExtendedIdMaxValue );
                    reinterpret_cast<uint32_t&>( Desc.Id ) = (
                        static_cast<uint32_t>( Type ) | ( ( Index << 8 ) & CExtendedIdMask )
                    );
                }
                else
                {
                    Desc.Type           = Type;
                    Desc.BasicIndexPart = Index;
                }

                Id = Desc.Id;
            }
            else
            {
                auto& Desc{ GetDescription() };
                Desc.Component = *reinterpret_cast<uint32_t*>( &InVariant );

                if constexpr( true == bExtendedIndex )
                {
                    SKL_ASSERT_ALLWAYS( Index <= CExtendedIdMaxValue );
                    reinterpret_cast<uint32_t&>( Desc.Id ) = (
                        static_cast<uint32_t>( Type ) | ( ( Index << 8 ) & CExtendedIdMask )
                    );
                }
                else
                {
                    Desc.Type           = Type;
                    Desc.BasicIndexPart = Index;
                }
            }
        }
        constexpr ~EntityId() noexcept = default;

        constexpr EntityId( const EntityId& Other ) noexcept
            : Id{ Other.Id } {}
        constexpr EntityId& operator=( const EntityId& Other ) noexcept
        {
            SKL_ASSERT( this != &Other );
            Id = Other.Id;
            return *this;
        }

        SKL_FORCEINLINE constexpr bool operator==( const EntityId& Other ) const noexcept { return Id == Other.Id; }
        SKL_FORCEINLINE constexpr bool operator!=( const EntityId& Other ) const noexcept { return Id != Other.Id; }
        SKL_FORCEINLINE constexpr void operator=( TEntityIdBase InId ) noexcept { return Id = InId; }
        SKL_FORCEINLINE constexpr operator TEntityIdBase() const noexcept { return Id; }
        SKL_FORCEINLINE constexpr operator EntityId<Variant, bExtendedIndex, !bAtomic>() const noexcept { return { Id }; }
        SKL_FORCEINLINE constexpr operator bool() const noexcept { return false == IsNone(); }

        SKL_FORCEINLINE constexpr TEntityIdBase GetId() const noexcept { return Id; };
        SKL_FORCEINLINE constexpr bool IsValid() const noexcept { return CInvalidEntityType != GetType();  }
        SKL_FORCEINLINE constexpr bool IsNone() const noexcept { return CInvalidEntityId != Id; }
        SKL_FORCEINLINE constexpr bool IsOfType( TEntityType InType ) const noexcept { return GetType() == InType;  }
        SKL_FORCEINLINE constexpr TEntityType GetType() const noexcept
        { 
            if constexpr( true == bAtomic )
            {
                const auto Desc{ ReadAsDescription() };
                return Desc.Type;
            }
            else
            {
                return GetDescription().Type; 
            }
        } 
        SKL_FORCEINLINE constexpr TIndexType GetIndex() const noexcept
        { 
            if constexpr( true == bExtendedIndex )
            {
                const auto Desc{ ReadAsDescription() };
                const uint32_t FirstPart{ *reinterpret_cast<const uint32_t*>( &Desc.Id ) };
                const uint32_t Index    { ( FirstPart & CExtendedIdMask ) >> 8 };
                return Index;
            }
            else
            {
                return GetDescription().BasicIndexPart;
            }
        } 
        SKL_FORCEINLINE constexpr Variant GetVariant() const noexcept
        {
            if constexpr( true == bAtomic )
            {
                const auto Desc{ ReadAsDescription() };
                return *reinterpret_cast<const Variant*>( &Desc.Component ); 
            }
            else
            {
                return *reinterpret_cast<const Variant*>( &GetDescription().Component ); 
            }
        }

    private:
        SKL_FORCEINLINE constexpr Description ReadAsDescription() const noexcept { return Description{ Id }; }
        SKL_FORCEINLINE constexpr const Description& GetDescription() const noexcept 
        { 
            SKL_ASSERT_ALLWAYS( false == bAtomic );
            return *reinterpret_cast<const Description*>( &Id );
        }
        SKL_FORCEINLINE constexpr Description& GetDescription() noexcept 
        { 
            SKL_ASSERT_ALLWAYS( false == bAtomic );
            return *reinterpret_cast<Description*>( &Id );
        }
        SKL_FORCEINLINE constexpr Description SetIdDescriptionFromDescription( Description InDescription ) const noexcept 
        { 
            Id = InDescription.Id;
        }

        TBaseType Id;

        friend EntityId<Variant, !bExtendedIndex,  bAtomic>;
        friend EntityId<Variant,  bExtendedIndex, !bAtomic>;
        friend EntityId<Variant, !bExtendedIndex, !bAtomic>;
    };
}