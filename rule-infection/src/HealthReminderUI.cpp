#include "HealthReminderUI.h"

#include <cstdlib>
#include <string>

namespace
{
constexpr const char* Title = "健康游戏提醒";
constexpr const char* Message =
    "这里的世界可以暂停，你也可以。\n\n"
    "看着红蓝生物追逐、觅食，很容易忘记时间。\n"
    "眼睛累了、肩颈紧了，就按下暂停，起身走走，看看远处。\n\n"
    "一条规则没调好，可以明天再想；别让再试一次挤掉睡眠。\n"
    "照顾好屏幕外的自己，再来照顾这个小世界。";
constexpr const char* ContinueText = "已阅读，进入游戏";
constexpr const char* Hint = "点击按钮或按 Enter 进入游戏    ESC 退出";
}

void HealthReminderUI::Init()
{
    // raylib 默认字体不含汉字，且不支持直接加载 TTC 字体集合。
    // 使用 Windows 自带的黑体 TTF，不复制或分发系统字体文件。
    const char* windowsDirectory = std::getenv("WINDIR");
    const std::string fontPath = windowsDirectory != nullptr
                                     ? std::string(windowsDirectory) + "/Fonts/simhei.ttf"
                                     : "";
    if (!fontPath.empty() && FileExists(fontPath.c_str()))
    {
        const std::string text = std::string(Title) + Message + ContinueText + Hint;
        int count = 0;
        int* codepoints = LoadCodepoints(text.c_str(), &count);
        font_ = LoadFontEx(fontPath.c_str(), 28, codepoints, count);
        UnloadCodepoints(codepoints);
        hasChineseFont_ = font_.texture.id != 0
                          && font_.texture.id != GetFontDefault().texture.id;
    }
}

bool HealthReminderUI::ShouldContinue(
    Vector2 mousePosition, bool leftPressed, bool enterPressed) const
{
    return enterPressed
           || (leftPressed && CheckCollisionPointRec(mousePosition, continueButton_));
}

void HealthReminderUI::Draw() const
{
    DrawText("Rule Infection", 100, 65, 38, RAYWHITE);
    const Font font = hasChineseFont_ ? font_ : GetFontDefault();
    DrawTextEx(font, hasChineseFont_ ? Title : "A moment before you begin",
               {100, 140}, 28, 1, Color{231, 195, 116, 255});
    DrawTextEx(font, hasChineseFont_ ? Message :
                   "This world can pause. So can you.\n\n"
                   "It is easy to lose track of time watching this little world.\n"
                   "When your eyes or shoulders feel tired, pause and take a break.\n\n"
                   "An unfinished rule can wait until tomorrow. Leave time for sleep.\n"
                   "Look after yourself outside the screen, too.",
               {100, 220}, 24, 1, Color{220, 226, 235, 255});

    const bool hovered = CheckCollisionPointRec(GetMousePosition(), continueButton_);
    DrawRectangleRounded(continueButton_, 0.16F, 8,
                         hovered ? Color{66, 131, 126, 255} : Color{47, 99, 96, 255});
    const char* buttonText = hasChineseFont_ ? ContinueText : "Enter game";
    const Vector2 size = MeasureTextEx(font, buttonText, 24, 1);
    DrawTextEx(font, buttonText,
               {continueButton_.x + (continueButton_.width - size.x) / 2,
                continueButton_.y + (continueButton_.height - size.y) / 2},
               24, 1, RAYWHITE);
    DrawTextEx(font, hasChineseFont_ ? Hint : "Click or press Enter to start. ESC to exit.",
               {350, 568}, 18, 1, LIGHTGRAY);
}

void HealthReminderUI::Shutdown()
{
    if (hasChineseFont_)
    {
        UnloadFont(font_);
        hasChineseFont_ = false;
    }
}
