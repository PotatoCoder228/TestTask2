#pragma once

#include <ostream>
#include <string>
#include <HTMLEntity.h>
#include <cstddef>
#include <string>
#include <vector>
#include <utility>

#include "HTMLEntity.h"
#include "Rect.h"

struct CalculatedRect
{
    std::size_t row_;
    std::size_t column_;
    std::size_t rowspan_;
    std::size_t colspan_;
    double lightness_;
};

class HTMLGenerator
{
public:
    virtual ~HTMLGenerator() = default;

    virtual std::string generate() const = 0;

    bool write(std::ostream &stream) const
    {
        stream << generate();
        return stream.good();
    }
};

class TableGenerator : public HTMLGenerator
{
public:
    void load(const std::vector<Rect> &rectangles);
    std::string generate() const override;

private:
    void calculate(const std::vector<Rect> &rectangles);

    std::vector<int64_t> columnWidths_;
    std::vector<int64_t> rowHeights_;
    std::vector<CalculatedRect> calculatedRects_;
};