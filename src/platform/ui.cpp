#include "platform/ui.hpp"
#include "platform/input.hpp"

namespace UI {
InputContext SnapshotCtx() {
  Input::MouseState mouseState = Input::Manager::Mouse();

  InputContext ctx{.mousePos = mouseState.mouseRenderPosition(),
                   .mouseLeftDown = mouseState.isLeftButtonDown(),
                   .mouseLeftPressed = mouseState.isLeftButtonPressed(),
                   .mouseLeftReleased = mouseState.isLeftButtonReleased(),
                   .mouseRightDown = mouseState.isRightButtonDown(),
                   .mouseRightPressed = mouseState.isRightButtonPressed(),
                   .mouseRightReleased = mouseState.isRightButtonReleased()};

  return ctx;
}

Widget::Widget(SDL_FRect bounds)
    : bounds(bounds), hovered(false), focused(false), pressed(false) {}

bool Widget::containsPoint(SDL_FPoint point) const {
  return SDL_PointInRectFloat(&point, &bounds);
}

void Widget::update(const InputContext &ctx) {
  bool wasHovered = hovered;
  hovered = containsPoint(ctx.mousePos);

  if (!hovered && wasHovered && onBlur) {
    focused = false;
    onBlur();
  }

  if (hovered && !wasHovered) {
    if (onFocus) {
      onFocus();
    }
    if (onHover) {
      onHover();
    }
  }

  if (hovered && ctx.mouseLeftDown) {
    pressed = true;
  }

  if (pressed && ctx.mouseLeftReleased) {
    if (hovered && onPressed) {
      onPressed();
    }

    pressed = false;
  }
}

void Layer::update(const InputContext &ctx) {
  for (auto &widget : widgets) {
    widget->update(ctx);
  }
}

void Layer::draw(SDL_Renderer *renderer) {
  for (auto &widget : widgets) {
    widget->draw(renderer);
  }
}

void Layer::clear() { widgets.clear(); }

Button::Button(SDL_FRect bounds, std::string label, Sprites::Sprite *sprite)
    : Widget(bounds), label(label), sprite(sprite) {}

void Button::draw(SDL_Renderer *renderer) const { sprite->draw(renderer); }

Text::Text(std::string text, std::shared_ptr<Font> font)
    : Widget({}), text(text), font(font), texture(nullptr) {
  if (font && !text.empty()) {
    texture = font->renderText(text, nullptr);
  }
}

void Text::update(const InputContext &ctx) {
  // no-op
}

void Text::setText(std::string text) {
  texture = font->renderText(text, nullptr);
}

void Text::draw(SDL_Renderer *renderer) const {
  SDL_FRect dest;
  SDL_GetTextureSize(texture, &dest.w, &dest.h);
  dest.x = 0;
  dest.y = 0;

  SDL_RenderTexture(renderer, texture, nullptr, &dest);
}
} // namespace UI