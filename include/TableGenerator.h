#pragma once

#include "HTMLEntity.h"
#include "Rect.h"

#include <cstddef>
#include <cstdint>
#include <map>
#include <ostream>
#include <vector>

struct CalculatedRect {
  std::size_t row;
  std::size_t column;
  std::size_t rowspan;
  std::size_t colspan;
  int lightness;
};

struct PreparedInput {
  std::vector<std::int64_t> xs;
  std::vector<std::int64_t> ys;
  long double minArea;
  long double maxArea;
};

struct ActiveRect {
  const CalculatedRect *rect;
  std::size_t right;
  std::size_t bottom;
};

class HTMLGenerator {
public:
  virtual ~HTMLGenerator() = default;
  virtual void write(std::ostream &stream) const = 0;
};

class TableGenerator : public HTMLGenerator {
public:
  void load(const std::vector<Rect> &rectangles);
  void write(std::ostream &stream) const override;

private:
  using ActiveRects = std::map<std::size_t, ActiveRect>;

  void calculate(const std::vector<Rect> &rectangles);
  PreparedInput prepareInput(const std::vector<Rect> &rectangles) const;
  void calculateAxisSizes(const PreparedInput &input);
  void calculateRectangles(const std::vector<Rect> &rectangles,
                           const PreparedInput &input);

  HTMLTag makeHead() const;
  HTMLTag makeTable() const;
  HTMLTag makeColumnGroup() const;
  void appendRows(HTMLTag &table) const;
  void updateActiveRects(std::size_t row, ActiveRects &activeRects,
                         std::size_t &nextRect) const;
  HTMLTag makeRow(std::size_t row, const ActiveRects &activeRects) const;
  HTMLTag makeRectangleCell(const CalculatedRect &rect) const;
  HTMLTag makeEmptyCell(std::size_t colspan) const;

  std::vector<std::uint64_t> columnWidths_;
  std::vector<std::uint64_t> rowHeights_;
  std::vector<CalculatedRect> calculatedRects_;
};
