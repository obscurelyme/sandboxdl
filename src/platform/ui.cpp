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

void Widget::setBounds(SDL_FRect newBounds) { bounds = newBounds; }

SDL_FRect Widget::getBounds() const { return bounds; }

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

Backdrop::Backdrop(SDL_Color color)
    : Widget({.x = 0, .y = 0, .w = 800, .h = 450}), color(color) {}

void Backdrop::draw(SDL_Renderer *renderer) const {
  Uint8 r, g, b, a;
  SDL_BlendMode blendMode;
  SDL_GetRenderDrawBlendMode(renderer, &blendMode);
  SDL_GetRenderDrawColor(renderer, &r, &g, &b, &a); // cache current draw color

  SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

  SDL_RenderFillRect(renderer, &bounds);

  SDL_SetRenderDrawBlendMode(renderer, blendMode);
  SDL_SetRenderDrawColor(renderer, r, g, b, a); // reset draw color
}

Container::Container(const ContainerProps &props) : Widget(props.bounds) {
  vAlign = props.vAlign;
  hAlign = props.hAlign;
}

void Container::layout() {
  const float gap = 4;
  float totalH = gap * (children.size() - 1);
  for (auto &child : children)
    totalH += child->getSize().y;

  float startY;
  switch (vAlign) {
  case Align::Start:
    startY = bounds.y;
    break;
  case Align::Center:
    startY = bounds.y + (bounds.h - totalH) * 0.5f;
    break;
  case Align::End:
    startY = bounds.y + bounds.h - totalH;
    break;
  }

  float curY = startY;
  for (auto &child : children) {
    auto size = child->getSize();
    float x;
    switch (hAlign) {
    case Align::Start:
      x = bounds.x;
      break;
    case Align::Center:
      x = bounds.x + (bounds.w - size.x) * 0.5f;
      break;
    case Align::End:
      x = bounds.x + bounds.w - size.x;
      break;
    }
    child->setPosition(x, curY);
    curY += size.y + gap;
  }
}

void Container::update(const InputContext &ctx) {
  for (auto &child : children) {
    layout();
    child->update(ctx);
  }
}

void Container::draw(SDL_Renderer *renderer) const {
  for (auto &child : children) {
    child->draw(renderer);
  }
}

void Button::setPosition(float x, float y) {
  bounds.x = x;
  bounds.y = y;
  sprite.dest.x = x;
  sprite.dest.y = y;
  label->setPosition(x + paddingX, y + paddingY);
}

void Button::setBounds(SDL_FRect newBounds) {
  bounds = newBounds;

  sprite.dest.x = bounds.x;
  sprite.dest.y = bounds.y;
  sprite.scaleX = bounds.w / sprite.width();
  sprite.scaleY = bounds.h / sprite.height();

  label->setBounds(SDL_FRect{
      .x = bounds.x + paddingX,
      .y = bounds.y + paddingY,
      .w = bounds.w - paddingX * 2,
      .h = bounds.h - paddingY * 2,
  });
}

Button::Button(const ButtonProps &btnProps, const TextProps &textProps)
    : Widget(btnProps.bounds), sprite(btnProps.sprite),
      paddingX(btnProps.paddingX), paddingY(btnProps.paddingY) {
  sprite.dest.x = bounds.x;
  sprite.dest.y = bounds.y;
  sprite.scaleX = bounds.w / sprite.width();
  sprite.scaleY = bounds.h / sprite.height();
  label = std::make_shared<Text>(
      SDL_FRect{
          .x = bounds.x + paddingX,
          .y = bounds.y + paddingY,
          .w = bounds.w - paddingX * 2,
          .h = bounds.h - paddingY * 2,
      },
      textProps.label, FontManager::GetFont(textProps.fontName));
}

void Button::draw(SDL_Renderer *renderer) const {
  sprite.draw(renderer);
  label->draw(renderer);
}

Text::Text(SDL_FRect bounds, std::string text, std::shared_ptr<Font> font)
    : Widget(bounds), text(text), font(font), fitted(true) {
  if (font && !text.empty()) {
    texture = font->renderTextFitted(text, &color, bounds);
  }
}

Text::Text(SDL_FPoint position, std::string text, std::shared_ptr<Font> font,
           float fontSize)
    : Widget({.x = position.x, .y = position.y, .w = 0, .h = 0}), text(text),
      font(font), fontSize(fontSize), fitted(false) {
  if (font && !text.empty()) {
    texture = font->renderText(text, fontSize, &color);
    if (!fitted) {
      SDL_GetTextureSize(texture, &bounds.w, &bounds.h);
    }
  }
}

Text::Text(std::string text, std::shared_ptr<Font> font)
    : Widget({}), text(text), font(font), texture(nullptr), fitted(false) {
  if (font && !text.empty()) {
    texture = font->renderText(text, &color);
    if (!fitted) {
      SDL_GetTextureSize(texture, &bounds.w, &bounds.h);
    }
  }
}

void Text::update(const InputContext &ctx) { Widget::update(ctx); }

void Text::setText(std::string newText) {
  text = newText;
  SDL_DestroyTexture(texture);

  if (fitted && bounds.w > 0 && bounds.h > 0) {
    texture = font->renderTextFitted(text, &color, bounds);
  } else {
    texture = font->renderText(text, &color);
    if (!fitted) {
      SDL_GetTextureSize(texture, &bounds.w, &bounds.h);
    }
  }
}

void Text::setColor(SDL_Color newColor) {
  color = newColor;
  SDL_DestroyTexture(texture);

  if (fitted && bounds.w > 0 && bounds.h > 0) {
    texture = font->renderTextFitted(text, &color, bounds);
  } else {
    texture = font->renderText(text, &color);
    if (!fitted) {
      SDL_GetTextureSize(texture, &bounds.w, &bounds.h);
    }
  }
}

void Text::setFontSize(float size) {
  fontSize = size;

  if (font && !text.empty()) {
    SDL_DestroyTexture(texture);
    texture = font->renderText(text, &color);
    if (!fitted) {
      SDL_GetTextureSize(texture, &bounds.w, &bounds.h);
    }
  }
}

void Text::draw(SDL_Renderer *renderer) const {
  float texW, texH;
  SDL_GetTextureSize(texture, &texW, &texH);

  SDL_FRect dest;
  if (fitted && bounds.w > 0 && bounds.h > 0) {
    dest.x = bounds.x + (bounds.w - texW) * 0.5f;
    dest.y = bounds.y + (bounds.h - texH) * 0.5f;
    dest.w = texW;
    dest.h = texH;
  } else {
    dest = {bounds.x, bounds.y, texW, texH};
  }

  SDL_RenderTexture(renderer, texture, nullptr, &dest);
}
} // namespace UI