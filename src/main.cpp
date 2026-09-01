#include "Rect.h"
#include "TableGenerator.h"

#include <fstream>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

std::optional<std::vector<Rect>> readRects(const char *filepath) {
  std::ifstream file(filepath);
  if (!file) {
    std::cerr << "Cannot open file: " << filepath << std::endl;
    return std::nullopt;
  }

  std::vector<Rect> rects;
  std::string line;
  std::size_t lineNumber = 0;

  while (std::getline(file, line)) {
    ++lineNumber;

    std::istringstream stream(line);

    Point topLeft{};
    Point bottomRight{};

    if (!(stream >> topLeft.x >> topLeft.y >> bottomRight.x >> bottomRight.y)) {
      std::cerr << "Invalid input at line " << lineNumber << ": " << line
                << std::endl;
      return std::nullopt;
    }

    std::string extra;
    if (stream >> extra) {
      std::cerr << "Invalid input at line " << lineNumber << ": " << line
                << std::endl;
      return std::nullopt;
    }

    if (topLeft.x >= bottomRight.x || topLeft.y >= bottomRight.y) {
      std::cerr << "Invalid rectangle at line " << lineNumber
                << ": top-left coordinates must be less than "
                   "bottom-right coordinates"
                << std::endl;
      return std::nullopt;
    }

    rects.push_back({topLeft, bottomRight});
  }

  if (!file.eof()) {
    std::cerr << "Cannot read file: " << filepath << std::endl;
    return std::nullopt;
  }

  if (rects.empty()) {
    std::cerr << "Input file contains no rectangles" << std::endl;
    return std::nullopt;
  }

  return rects;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
    return 1;
  }

  const auto rects = readRects(argv[1]);
  if (!rects)
    return 1;

  TableGenerator generator;
  generator.load(*rects);

  std::ofstream output("output.html");
  if (!output) {
    std::cerr << "Cannot open output.html for writing" << std::endl;
    return 1;
  }

  generator.write(output);
  output.flush();

  if (!output) {
    std::cerr << "Cannot write output.html" << std::endl;
    return 1;
  }

  std::cout << "output.html generated successfully" << std::endl;
  return 0;
}
