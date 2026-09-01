#include "TableGenerator.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

namespace {
std::uint64_t coordinateDistance(std::int64_t begin, std::int64_t end) {
  return static_cast<std::uint64_t>(end) - static_cast<std::uint64_t>(begin);
}

std::size_t coordinateIndex(const std::vector<std::int64_t> &coordinates,
                            std::int64_t coordinate) {
  const auto it =
      std::lower_bound(coordinates.begin(), coordinates.end(), coordinate);

  return static_cast<std::size_t>(it - coordinates.begin());
}

long double rectangleArea(const Rect &rect) {
  const auto width = coordinateDistance(rect.topLeft.x, rect.bottomRight.x);
  const auto height = coordinateDistance(rect.topLeft.y, rect.bottomRight.y);

  return static_cast<long double>(width) * static_cast<long double>(height);
}
}

void TableGenerator::load(const std::vector<Rect> &rectangles) {
  calculate(rectangles);
}

void TableGenerator::calculate(const std::vector<Rect> &rectangles) {
  columnWidths_.clear();
  rowHeights_.clear();
  calculatedRects_.clear();

  if (rectangles.empty())
    return;

  const PreparedInput input = prepareInput(rectangles);
  calculateAxisSizes(input);
  calculateRectangles(rectangles, input);

  std::sort(calculatedRects_.begin(), calculatedRects_.end(),
            [](const CalculatedRect &lhs, const CalculatedRect &rhs) {
              if (lhs.row != rhs.row)
                return lhs.row < rhs.row;

              return lhs.column < rhs.column;
            });
}

PreparedInput
TableGenerator::prepareInput(const std::vector<Rect> &rectangles) const {
  PreparedInput input;
  input.xs.reserve(rectangles.size() * 2);
  input.ys.reserve(rectangles.size() * 2);
  input.minArea = std::numeric_limits<long double>::max();
  input.maxArea = 0.0L;

  for (const auto &rect : rectangles) {
    input.xs.push_back(rect.topLeft.x);
    input.xs.push_back(rect.bottomRight.x);
    input.ys.push_back(rect.topLeft.y);
    input.ys.push_back(rect.bottomRight.y);

    const long double area = rectangleArea(rect);
    input.minArea = std::min(input.minArea, area);
    input.maxArea = std::max(input.maxArea, area);
  }

  std::sort(input.xs.begin(), input.xs.end());
  input.xs.erase(std::unique(input.xs.begin(), input.xs.end()), input.xs.end());

  std::sort(input.ys.begin(), input.ys.end());
  input.ys.erase(std::unique(input.ys.begin(), input.ys.end()), input.ys.end());

  return input;
}

void TableGenerator::calculateAxisSizes(const PreparedInput &input) {
  columnWidths_.reserve(input.xs.size() - 1);
  for (std::size_t i = 0; i + 1 < input.xs.size(); ++i)
    columnWidths_.push_back(coordinateDistance(input.xs[i], input.xs[i + 1]));

  rowHeights_.reserve(input.ys.size() - 1);
  for (std::size_t i = 0; i + 1 < input.ys.size(); ++i)
    rowHeights_.push_back(coordinateDistance(input.ys[i], input.ys[i + 1]));
}

void TableGenerator::calculateRectangles(const std::vector<Rect> &rectangles,
                                         const PreparedInput &input) {
  calculatedRects_.reserve(rectangles.size());

  for (const auto &rect : rectangles) {
    const std::size_t left = coordinateIndex(input.xs, rect.topLeft.x);
    const std::size_t right = coordinateIndex(input.xs, rect.bottomRight.x);
    const std::size_t top = coordinateIndex(input.ys, rect.topLeft.y);
    const std::size_t bottom = coordinateIndex(input.ys, rect.bottomRight.y);

    const long double area = rectangleArea(rect);

    long double normalizedArea = 0.5L;
    if (input.minArea != input.maxArea) {
      normalizedArea = (area - input.minArea) / (input.maxArea - input.minArea);
    }

    const int lightness =
        static_cast<int>(std::lround(85.0L - 40.0L * normalizedArea));

    calculatedRects_.push_back(
        {top, left, bottom - top, right - left, lightness});
  }
}

void TableGenerator::write(std::ostream &stream) const {
  HTMLEntity html;
  html.root().attributes_["lang"] = "en";

  HTMLTag body{"body"};
  body.children_.push_back(makeTable());

  html.root().children_.push_back(makeHead());
  html.root().children_.push_back(std::move(body));

  html.write(stream);
}

HTMLTag TableGenerator::makeHead() const {
  HTMLTag head{"head"};
  head.children_.push_back(HTMLTag{"meta", {{"charset", "utf-8"}}});
  head.children_.push_back(
      HTMLTag{"meta",
              {{"content", "width=device-width,initial-scale=1"},
               {"name", "viewport"}}});
  head.children_.push_back(HTMLTag{"title", {}, "Rectangle Table"});
  head.children_.push_back(
      HTMLTag{"style",
              {},
              "html,body{margin:0;}"
              "body{padding:1px;}"
              "table{border-collapse:collapse;border-spacing:0;"
              "table-layout:fixed;}"
              "td{padding:0;}"
              ".rect{box-shadow:inset 0 0 0 1px rgba(0,0,0,.35);}"});

  return head;
}

HTMLTag TableGenerator::makeTable() const {
  HTMLTag table{"table", {{"role", "presentation"}}};

  if (columnWidths_.empty() || rowHeights_.empty())
    return table;

  const std::uint64_t tableWidth = std::accumulate(
      columnWidths_.begin(), columnWidths_.end(), std::uint64_t{0});

  table.attributes_["style"] = "width:" + std::to_string(tableWidth) + "px";

  table.children_.push_back(makeColumnGroup());
  appendRows(table);

  return table;
}

HTMLTag TableGenerator::makeColumnGroup() const {
  HTMLTag colgroup{"colgroup"};

  for (const auto width : columnWidths_) {
    HTMLTag col{"col"};
    col.attributes_["style"] = "width:" + std::to_string(width) + "px";
    colgroup.children_.push_back(std::move(col));
  }

  return colgroup;
}

void TableGenerator::appendRows(HTMLTag &table) const {
  ActiveRects activeRects;
  std::size_t nextRect = 0;

  for (std::size_t row = 0; row < rowHeights_.size(); ++row) {
    updateActiveRects(row, activeRects, nextRect);
    table.children_.push_back(makeRow(row, activeRects));
  }
}

void TableGenerator::updateActiveRects(std::size_t row,
                                       ActiveRects &activeRects,
                                       std::size_t &nextRect) const {
  for (auto it = activeRects.begin(); it != activeRects.end();) {
    if (it->second.bottom <= row)
      it = activeRects.erase(it);
    else
      ++it;
  }

  while (nextRect < calculatedRects_.size() &&
         calculatedRects_[nextRect].row == row) {
    const auto &rect = calculatedRects_[nextRect];

    activeRects.emplace(
        rect.column,
        ActiveRect{&rect, rect.column + rect.colspan, rect.row + rect.rowspan});

    ++nextRect;
  }
}

HTMLTag TableGenerator::makeRow(std::size_t row,
                                const ActiveRects &activeRects) const {
  HTMLTag tr{"tr"};
  tr.attributes_["style"] = "height:" + std::to_string(rowHeights_[row]) + "px";

  std::size_t column = 0;

  for (const auto &[left, active] : activeRects) {
    if (column < left)
      tr.children_.push_back(makeEmptyCell(left - column));

    if (active.rect->row == row)
      tr.children_.push_back(makeRectangleCell(*active.rect));

    column = active.right;
  }

  if (column < columnWidths_.size())
    tr.children_.push_back(makeEmptyCell(columnWidths_.size() - column));

  return tr;
}

HTMLTag TableGenerator::makeRectangleCell(const CalculatedRect &rect) const {
  HTMLTag td{"td", {{"class", "rect"}}};

  if (rect.rowspan > 1)
    td.attributes_["rowspan"] = std::to_string(rect.rowspan);

  if (rect.colspan > 1)
    td.attributes_["colspan"] = std::to_string(rect.colspan);

  td.attributes_["style"] =
      "background-color:hsl(210,70%," + std::to_string(rect.lightness) + "%)";

  return td;
}

HTMLTag TableGenerator::makeEmptyCell(std::size_t colspan) const {
  HTMLTag td{"td"};

  if (colspan > 1)
    td.attributes_["colspan"] = std::to_string(colspan);

  return td;
}
