#include "vibecraft/screen.h"

namespace vibecraft {

void ScreenManager::Push(std::unique_ptr<Screen> screen) {
    if (screen) {
        screen->OnEnter();
        stack_.push_back(std::move(screen));
    }
}

bool ScreenManager::Pop() {
    if (stack_.empty()) {
        return false;
    }
    stack_.back()->OnExit();
    stack_.pop_back();
    return true;
}

Screen* ScreenManager::Top() const {
    if (stack_.empty()) {
        return nullptr;
    }
    return stack_.back().get();
}

int ScreenManager::Size() const {
    return static_cast<int>(stack_.size());
}

bool ScreenManager::IsEmpty() const {
    return stack_.empty();
}

}  // namespace vibecraft
