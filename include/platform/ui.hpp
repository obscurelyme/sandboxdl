#pragma once

#include "platform/spritesheet.hpp"
#include "platform/text.hpp"
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
  virtual void draw(SDL_Renderer *renderer) const = 0;

  bool containsPoint(SDL_FPoint p) const;
  virtual void setBounds(SDL_FRect newBounds);
  SDL_FRect getBounds() const;
  SDL_FPoint getSize() const { return {bounds.w, bounds.h}; }
  virtual void setPosition(float x, float y) {
    bounds.x = x;
    bounds.y = y;
  }

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

class Text; // Forward Ref

struct TextProps {
  std::string label;
  std::string fontName;
};

struct ButtonProps {
  SDL_FRect bounds;
  int paddingX;
  int paddingY;
  Sprites::Sprite *sprite;
};

enum class Align { Start, Center, End };

struct ContainerProps {
  SDL_FRect bounds;
  Align hAlign;
  Align vAlign;
};

class Backdrop : public Widget {
public:
  explicit Backdrop(SDL_Color color);
  void draw(SDL_Renderer *renderer) const;

private:
  SDL_Color color;
};

class Container : public Widget {
public:
  explicit Container(const ContainerProps &props);

  template <typename T, typename... Args> T *add(Args &&...args) {
    static_assert(std::is_base_of_v<Widget, T>,
                  "T must derive from UI::Widget");
    children.push_back(std::make_unique<T>(std::forward<Args>(args)...));
    return static_cast<T *>(children.back().get());
  }

  void update(const InputContext &ctx) override;
  void draw(SDL_Renderer *renderer) const override;

private:
  Align hAlign;
  Align vAlign;
  std::vector<std::unique_ptr<Widget>> children;

  void layout();
};

class Button : public Widget {
public:
  Button(const ButtonProps &btnProps, const TextProps &textProps);

  void draw(SDL_Renderer *renderer) const override;
  void setBounds(SDL_FRect newBounds) override;
  void setPosition(float x, float y) override;

private:
  std::shared_ptr<Text> label;
  Sprites::Sprite *sprite;
  int paddingX;
  int paddingY;
};

class Text : public Widget {
public:
  Text(std::string text, std::shared_ptr<Font> font);
  Text(SDL_FRect bounds, std::string text, std::shared_ptr<Font> font);
  void update(const InputContext &ctx) override;
  void draw(SDL_Renderer *renderer) const override;
  void setText(std::string newText);

private:
  std::string text;
  std::shared_ptr<Font> font;
  SDL_Texture *texture;
};
} // namespace UI