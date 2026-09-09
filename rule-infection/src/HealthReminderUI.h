#pragma once

#include "raylib.h"

class HealthReminderUI
{
public:
    void Init();
    void Draw() const;
    void Shutdown();
    bool ShouldContinue(Vector2 mousePosition, bool leftPressed, bool enterPressed) const;

private:
    Font font_{};
    bool hasChineseFont_{false};
    Rectangle continueButton_{460.0F, 480.0F, 360.0F, 58.0F};
};
