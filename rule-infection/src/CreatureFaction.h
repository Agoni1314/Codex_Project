#pragma once

enum class CreatureFaction
{
    Red,
    Blue
};

constexpr const char* GetFactionName(CreatureFaction faction)
{
    return faction == CreatureFaction::Red ? "Red" : "Blue";
}
