#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

#include <Rect.h>
#include <TableGenerator.h>

namespace fs = std::filesystem;

std::optional<std::vector<Rect>> readRects(const char *filepath)
{
    if (!fs::exists(filepath) || !fs::is_regular_file(filepath))
    {
        std::cerr << "Invalid file: " << filepath << std::endl;
        return std::nullopt;
    }

    std::ifstream file(filepath);
    if (!file)
    {
        std::cerr << "Cannot open file: " << filepath << std::endl;
        return std::nullopt;
    }

    std::vector<Rect> rects;
    std::string line;
    std::size_t lineNumber = 0;

    while (std::getline(file, line))
    {
        ++lineNumber;

        if (line.empty())
            continue;

        std::istringstream stream(line);

        Point tl;
        Point br;

        if (!(stream >> tl.x_ >> tl.y_ >> br.x_ >> br.y_))
        {
            std::cerr << "Invalid input at line "
                      << lineNumber << ": " << line << std::endl;
            return std::nullopt;
        }

        std::string extra;
        if (stream >> extra)
        {
            std::cerr << "Invalid input at line "
                      << lineNumber << ": " << line << std::endl;
            return std::nullopt;
        }

        rects.emplace_back(tl, br);
    }

    if (rects.empty())
    {
        std::cerr << "Input file contains no rectangles" << std::endl;
        return std::nullopt;
    }

    return rects;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
        return 1;
    }

    auto rects = readRects(argv[1]);
    if (!rects)
        return 1;

    TableGenerator generator;
    generator.load(*rects);

    std::ofstream output("output.html");
    if (!output)
    {
        std::cerr << "Cannot open output.html for writing" << std::endl;
        return 1;
    }

    if (!generator.write(output))
    {
        std::cerr << "Cannot write output.html" << std::endl;
        return 1;
    }

    std::cout << "output.html generated successfully" << std::endl;
    return 0;
}