#pragma once

enum class BehaviorType
{
    Wander,
    Chase,
    Flee,
    Attack,
    Stay,
    SeekFood
};

constexpr const char* GetBehaviorName(BehaviorType behavior)
{
    switch (behavior)
    {
    case BehaviorType::Chase:
        return "Chase";
    case BehaviorType::Flee:
        return "Flee";
    case BehaviorType::Attack:
        return "Attack";
    case BehaviorType::Stay:
        return "Stay";
    case BehaviorType::SeekFood:
        return "SeekFood";
    default:
        return "Wander";
    }
}
