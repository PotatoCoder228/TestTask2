#pragma once

#include <map>
#include <string>
#include <unordered_set>
#include <vector>
#include <utility>

inline static const std::unordered_set<std::string> selfClosingTags{
    "area", "base", "br", "col", "embed", "hr", "img",
    "input", "link", "meta", "source", "track", "wbr"};

struct HTMLTag
{
    HTMLTag(
        std::string name,
        std::map<std::string, std::string> attributes = {},
        std::string text = {})
        : name_(std::move(name)),
          attributes_(std::move(attributes)),
          text_(std::move(text))
    {
    }

    std::string name_;
    std::map<std::string, std::string> attributes_;
    std::string text_;
    std::vector<HTMLTag> children_;
};

class HTMLEntity
{
public:
    HTMLEntity()
        : doctype_("<!doctype html>"),
          root_{"html"}
    {
    }

    std::string toString() const
    {
        std::string result = doctype_ + '\n';
        appendTag(root_, result);
        return result;
    }

    HTMLTag &root()
    {
        return root_;
    }

private:
    void appendTag(const HTMLTag &tag, std::string &result) const
    {
        result += '<';
        result += tag.name_;

        for (const auto &[name, value] : tag.attributes_)
        {
            result += ' ';
            result += name;
            result += "=\"";
            result += value;
            result += '"';
        }

        result += '>';

        if (selfClosingTags.contains(tag.name_))
            return;

        result += tag.text_;

        for (const auto &child : tag.children_)
            appendTag(child, result);

        result += "</";
        result += tag.name_;
        result += '>';
    }

    std::string doctype_;
    HTMLTag root_;
};