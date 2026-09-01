#pragma once

#include <cstdint>

struct Point {
  std::int64_t x;
  std::int64_t y;
};

struct Rect {
  Point topLeft;
  Point bottomRight;
};
