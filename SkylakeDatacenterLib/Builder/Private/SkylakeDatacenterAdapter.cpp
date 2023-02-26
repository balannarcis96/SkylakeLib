//!
//! \file SkylakeDatacenterAdapter.cpp
//! 
//! \brief Skylake Datacenter adapter abstractions and utilities
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#include "SkylakeDatacenterAdapter.h"

namespace SKL::DC
{
    void RawAttribute::BuildHash() noexcept
    {
        Hash = 0; //@TODO
    }

    RawElement::~RawElement() noexcept
    {
        for( size_t i = 0; i < Children.size(); ++i )
        {
            if( nullptr == Children[i] || Children[i] == this )
            {
                continue;
            }

            if( true == Children[i]->RemoveReference() )
            {
                delete Children[i];
            }

            Children[i] = nullptr;
        }
        Children.clear();
    }

    void RawElement::BuildHash() noexcept
    {
        Hash = 0; //@TODO
    }
    
    uint64_t RawElement::BuildKey() const noexcept
    {
        return 0; //@TODO
    }

    bool DatacenterAdapter::IsLanguageAttributeByName( const std::string_view& InString ) const noexcept 
    {
        return 0 == SKL_STRICMP( InString.data(), "language", 8 );
    }
}
