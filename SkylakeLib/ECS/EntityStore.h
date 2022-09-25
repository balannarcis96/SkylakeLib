//!
//! \file EntityStore.h
//! 
//! \brief Thread safe Entity Store
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    template<typename TEntityStore>
    struct SharedEntityDeallocator
    {
        using RootComponent = typename TEntityStore::Traits::RootComponent;
        void operator()( RootComponent* InPtr ) const noexcept;
    };

    template<typename TEntityId, size_t TMaxEnitites, typename TRootComponent, typename ...TComponents>
    struct EntityStore
    {
        struct Traits
        {
            using EntityId                          = TEntityId;
            using RootComponent                     = TRootComponent;
            using Componenets                       = std::tuple<TComponents...>;
            static constexpr size_t MaxEntities     = TMaxEnitites;
            static constexpr size_t ComponentsCount = 1 + sizeof...( TComponents );
            using IndexType                         = typename EntityId::TIndexType;

            using MyType           = EntityStore<EntityId, MaxEntities, RootComponent, TComponents...>;
            using TEntitySharedPtr = TSharedPtr<RootComponent, SharedEntityDeallocator<MyType>>;

            struct RootComponentWrapper: MemoryPolicy::ControlBlock, TRootComponent {};

            using Store = SymmetricStore<IndexType, TMaxEnitites, RootComponentWrapper, TComponents...>;
        };

        using TEntityPtr       = TRootComponent *;
        using TEntitySharedPtr = typename Traits::TEntitySharedPtr;
        using MyStore          = typename Traits::Store;

        template<typename ...TArgs>
        TEntityPtr AllocateEntity( TArgs... InArgs ) noexcept
        {

        }
        void DeallocateEntity( TEntityPtr InEntity ) noexcept
        {
            
        }

    private:
        MyStore Store; 
    };

    template<typename TEntityStore>
    void SharedEntityDeallocator<TEntityStore>::operator()( RootComponent* InPtr ) const noexcept
    {
        
    }
}
