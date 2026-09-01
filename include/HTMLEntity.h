#pragma once

#include <map>
#include <ostream>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

inline const std::unordered_set<std::string> selfClosingTags{
    "area",  "base", "br",   "col",    "embed", "hr", "img",
    "input", "link", "meta", "source", "track", "wbr"};

struct HTMLTag {
  HTMLTag(std::string name, std::map<std::string, std::string> attributes = {},
          std::string text = {})
      : name_(std::move(name)), attributes_(std::move(attributes)),
        text_(std::move(text)) {}

  void write(std::ostream &stream) const {
    stream << '<' << name_;

    for (const auto &[name, value] : attributes_)
      stream << ' ' << name << "=\"" << value << '"';

    stream << '>';

    if (selfClosingTags.find(name_) != selfClosingTags.end())
      return;

    stream << text_;

    for (const auto &child : children_)
      child.write(stream);

    stream << "</" << name_ << '>';
  }

  std::string name_;
  std::map<std::string, std::string> attributes_;
  std::string text_;
  std::vector<HTMLTag> children_;
};

class HTMLEntity {
public:
  HTMLEntity() : doctype_("<!doctype html>"), root_{"html"} {}

  void write(std::ostream &stream) const {
    stream << doctype_;
    root_.write(stream);
  }

  HTMLTag &root() { return root_; }

private:
  std::string doctype_;
  HTMLTag root_;
};
