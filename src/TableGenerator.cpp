#include "TableGenerator.h"
#include <algorithm>
#include <limits>
#include <vector>
#include <algorithm>
#include <limits>
#include <cmath>
#include <utility>
#include <vector>

void TableGenerator::load(const std::vector<Rect> &rectangles)
{
    calculate(rectangles);
}

void TableGenerator::calculate(const std::vector<Rect> &rectangles)
{
    columnWidths_.clear();
    rowHeights_.clear();
    calculatedRects_.clear();

    if (rectangles.empty())
        return;

    std::vector<int64_t> xs;
    std::vector<int64_t> ys;

    xs.reserve(rectangles.size() * 2);
    ys.reserve(rectangles.size() * 2);
    calculatedRects_.reserve(rectangles.size());

    long double minArea = std::numeric_limits<long double>::max();
    long double maxArea = 0.0;

    for (const auto &rect : rectangles)
    {
        xs.push_back(rect.tl_.x_);
        xs.push_back(rect.br_.x_);

        ys.push_back(rect.tl_.y_);
        ys.push_back(rect.br_.y_);

        const long double width =
            static_cast<long double>(rect.br_.x_) - rect.tl_.x_;

        const long double height =
            static_cast<long double>(rect.br_.y_) - rect.tl_.y_;

        const long double area = width * height;

        minArea = std::min(minArea, area);
        maxArea = std::max(maxArea, area);
    }

    std::sort(xs.begin(), xs.end());
    xs.erase(std::unique(xs.begin(), xs.end()), xs.end());

    std::sort(ys.begin(), ys.end());
    ys.erase(std::unique(ys.begin(), ys.end()), ys.end());

    columnWidths_.reserve(xs.size() - 1);

    for (std::size_t i = 0; i + 1 < xs.size(); ++i)
        columnWidths_.push_back(xs[i + 1] - xs[i]);

    rowHeights_.reserve(ys.size() - 1);

    for (std::size_t i = 0; i + 1 < ys.size(); ++i)
        rowHeights_.push_back(ys[i + 1] - ys[i]);

    for (const auto &rect : rectangles)
    {
        const std::size_t left =
            std::lower_bound(xs.begin(), xs.end(), rect.tl_.x_) - xs.begin();

        const std::size_t right =
            std::lower_bound(xs.begin(), xs.end(), rect.br_.x_) - xs.begin();

        const std::size_t top =
            std::lower_bound(ys.begin(), ys.end(), rect.tl_.y_) - ys.begin();

        const std::size_t bottom =
            std::lower_bound(ys.begin(), ys.end(), rect.br_.y_) - ys.begin();

        const long double width =
            static_cast<long double>(rect.br_.x_) - rect.tl_.x_;

        const long double height =
            static_cast<long double>(rect.br_.y_) - rect.tl_.y_;

        const long double area = width * height;

        double normalizedArea = 0.5;

        if (minArea != maxArea)
        {
            normalizedArea = static_cast<double>(
                (area - minArea) / (maxArea - minArea));
        }

        calculatedRects_.push_back({top,
                                    left,
                                    bottom - top,
                                    right - left,
                                    85.0 - 40.0 * normalizedArea});
    }

    std::sort(
        calculatedRects_.begin(),
        calculatedRects_.end(),
        [](const CalculatedRect &lhs, const CalculatedRect &rhs)
        {
            if (lhs.row_ != rhs.row_)
                return lhs.row_ < rhs.row_;

            return lhs.column_ < rhs.column_;
        });
}

std::string TableGenerator::generate() const
{
    HTMLEntity html;

    HTMLTag head{"head"};
    head.children_.push_back(
        HTMLTag{"meta", {{"charset", "utf-8"}}});

    head.children_.push_back(
        HTMLTag{
            "style",
            {},
            "html,body{margin:0;padding:0;}"
            "table{border-collapse:collapse;border-spacing:0;}"
            "td{padding:0;}"});

    HTMLTag body{"body"};
    HTMLTag table{"table"};

    if (!columnWidths_.empty() && !rowHeights_.empty())
    {
        HTMLTag colgroup{"colgroup"};

        for (const auto width : columnWidths_)
        {
            HTMLTag col{"col"};
            col.attributes_["style"] =
                "width:" + std::to_string(width) + "px";

            colgroup.children_.push_back(std::move(col));
        }

        table.children_.push_back(std::move(colgroup));

        std::vector<std::size_t> occupiedUntilRow(
            columnWidths_.size(), 0);

        std::size_t nextRect = 0;

        for (std::size_t row = 0; row < rowHeights_.size(); ++row)
        {
            HTMLTag tr{"tr"};
            tr.attributes_["style"] =
                "height:" + std::to_string(rowHeights_[row]) + "px";

            std::size_t column = 0;

            while (column < columnWidths_.size())
            {
                if (occupiedUntilRow[column] > row)
                {
                    ++column;
                    continue;
                }

                if (nextRect < calculatedRects_.size() &&
                    calculatedRects_[nextRect].row_ == row &&
                    calculatedRects_[nextRect].column_ == column)
                {
                    const auto &rect = calculatedRects_[nextRect];

                    HTMLTag td{"td"};

                    if (rect.rowspan_ > 1)
                        td.attributes_["rowspan"] =
                            std::to_string(rect.rowspan_);

                    if (rect.colspan_ > 1)
                        td.attributes_["colspan"] =
                            std::to_string(rect.colspan_);

                    const auto lightness =
                        static_cast<int>(std::lround(rect.lightness_));

                    td.attributes_["style"] =
                        "background-color:hsl(210,70%," +
                        std::to_string(lightness) + "%)";

                    for (std::size_t i = column;
                         i < column + rect.colspan_;
                         ++i)
                    {
                        occupiedUntilRow[i] = row + rect.rowspan_;
                    }

                    tr.children_.push_back(std::move(td));

                    column += rect.colspan_;
                    ++nextRect;
                    continue;
                }

                tr.children_.push_back(HTMLTag{"td"});
                ++column;
            }

            table.children_.push_back(std::move(tr));
        }
    }

    body.children_.push_back(std::move(table));

    html.root().children_.push_back(std::move(head));
    html.root().children_.push_back(std::move(body));

    return html.toString();
}