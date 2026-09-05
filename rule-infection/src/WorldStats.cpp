#include "WorldStats.h"

void WorldStats::RecordDeath(
    CreatureFaction victimFaction,
    std::optional<CreatureFaction> killerFaction)
{
    FactionStats& victim =
        victimFaction == CreatureFaction::Red ? red_ : blue_;
    ++victim.deaths;

    if (killerFaction.has_value())
    {
        FactionStats& killer =
            *killerFaction == CreatureFaction::Red ? red_ : blue_;
        ++killer.kills;
    }
}

const FactionStats& WorldStats::GetFactionStats(CreatureFaction faction) const
{
    return faction == CreatureFaction::Red ? red_ : blue_;
}

void WorldStats::Reset()
{
    red_ = {};
    blue_ = {};
}
