#ifndef VIBECRAFT_SCREEN_H
#define VIBECRAFT_SCREEN_H

#include <memory>
#include <string>
#include <vector>

namespace vibecraft {

/// Base class for game screens (game, pause menu, settings, etc.).
/// Subclasses override the virtual methods to implement screen behavior.
class Screen {
public:
    virtual ~Screen() = default;

    /// Called when this screen becomes the top of the stack.
    virtual void OnEnter() {}

    /// Called when this screen is removed from the stack.
    virtual void OnExit() {}

    /// Update logic for this screen (called each tick).
    virtual void Update(float dt) { (void)dt; }

    /// Render this screen (called each frame).
    virtual void Render() {}

    /// Handle input events for this screen.
    virtual void HandleInput() {}

    /// Get the name of this screen (for debugging).
    virtual std::string GetName() const { return "Screen"; }
};

/// Manages a stack of screens. The topmost screen is the active one.
///
/// Uses std::vector<std::unique_ptr<Screen>> internally.
/// Push adds a screen on top; Pop removes the topmost screen.
class ScreenManager {
public:
    ScreenManager() = default;

    /// Push a screen onto the top of the stack.
    /// Calls OnEnter() on the new screen.
    void Push(std::unique_ptr<Screen> screen);

    /// Pop the topmost screen off the stack.
    /// Calls OnExit() on the removed screen.
    /// Returns false if the stack was already empty.
    bool Pop();

    /// Get the topmost screen, or nullptr if the stack is empty.
    Screen* Top() const;

    /// Return the number of screens on the stack.
    int Size() const;

    /// Return true if the stack is empty.
    bool IsEmpty() const;

private:
    std::vector<std::unique_ptr<Screen>> stack_;
};

}  // namespace vibecraft

#endif  // VIBECRAFT_SCREEN_H
