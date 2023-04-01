//!
//! \file Counters.h
//! 
//! \brief SkylakeLib KPI - Key Performance indicator
//! 
//! \author Balan Narcis (balannarcis96@gmail.com)
//! 
#pragma once

namespace SKL
{
    class CountersContext: public ITLSSingleton<CountersContext>
    {
    public:
        RStatus Initialize() noexcept override;
        const char *GetName() const noexcept override { return "[CountersContext]"; }

    private:
    };
}