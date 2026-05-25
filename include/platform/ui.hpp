#pragma once

#include "platform/spritesheet.hpp"
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace UI {
struct InputContext {
  SDL_FPoint mousePos;
  bool mouseLeftDown;
  bool mouseLeftPressed;
  bool mouseLeftReleased;
  bool mouseRightDown;
  bool mouseRightPressed;
  bool mouseRightReleased;
};

/**
 * Returns a snapshot of the input state within the current frame, to be used by
 * the UI Layer.
 *
 * Currently, only supports mouse position and left/right button tracking
 */
InputContext SnapshotCtx();

class Widget {
public:
  explicit Widget(SDL_FRect bounds);
  virtual ~Widget() = default;

  virtual void update(const InputContext &ctx);
  virtual void draw(SDL_Renderer *renderer) = 0;

  bool containsPoint(SDL_FPoint p) const;

  std::function<void()> onFocus;
  std::function<void()> onBlur;
  std::function<void()> onHover;
  std::function<void()> onPressed;

protected:
  SDL_FRect bounds;
  bool hovered;
  bool focused;
  bool pressed;
};

class Layer {
public:
  // Adds a widget to the layer, this function will take ownership of the widget
  template <typename T, typename... Args> T *add(Args &&...args) {
    static_assert(std::is_base_of_v<Widget, T>,
                  "T must derive from UI::Widget");
    widgets.push_back(std::make_unique<T>(std::forward<Args>(args)...));
    return static_cast<T *>(widgets.back().get());
  }

  void update(const InputContext &ctx);
  void draw(SDL_Renderer *renderer);
  void clear();

private:
  std::vector<std::unique_ptr<Widget>> widgets;
};

class Button : public Widget {
public:
  Button(SDL_FRect bounds, std::string label, Sprites::Sprite *sprite);
  void draw(SDL_Renderer *renderer) override;

private:
  std::string label;
  Sprites::Sprite *sprite;
};
} // namespace UI