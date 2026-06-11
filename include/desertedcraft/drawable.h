#pragma once

class Drawable {
public:
  virtual void Draw() const = 0;

protected:
  unsigned int VAO, VBO;
};
