#include "HealthReminderUI.h"

#include <cassert>

int main()
{
    HealthReminderUI reminder;
    assert(!reminder.ShouldContinue({0, 0}, false, false));
    assert(!reminder.ShouldContinue({0, 0}, true, false));
    assert(!reminder.ShouldContinue({640, 509}, false, false));
    assert(reminder.ShouldContinue({640, 509}, true, false));
    assert(!reminder.ShouldContinue({459, 509}, true, false));
    assert(!reminder.ShouldContinue({821, 509}, true, false));
    assert(!reminder.ShouldContinue({640, 479}, true, false));
    assert(!reminder.ShouldContinue({640, 539}, true, false));
    assert(reminder.ShouldContinue({0, 0}, false, true));
}
