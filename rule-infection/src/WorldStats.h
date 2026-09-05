#pragma once

#include "CreatureFaction.h"

#include <optional>

struct FactionStats
{
    int deaths{0};
    int kills{0};
};

class WorldStats
{
public:
    void RecordDeath(
        CreatureFaction victimFaction,
        std::optional<CreatureFaction> killerFaction = std::nullopt);
    const FactionStats& GetFactionStats(CreatureFaction faction) const;
    void Reset();

private:
    FactionStats red_;
    FactionStats blue_;
};
