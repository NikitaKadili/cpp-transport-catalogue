#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>
#include <vector>

namespace json {

class Node;

using Dict = std::map<std::string, Node>;
using Array = std::vector<Node>;
using NodeValue = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;
using Number = std::variant<int, double>;

// Эта ошибка должна выбрасываться при ошибках парсинга JSON
class ParsingError : public std::runtime_error {
public:
    using runtime_error::runtime_error;
};

// class Node {
class Node final : private std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string> {
public:
    using variant::variant;

    const Array& AsArray() const;
    const Dict& AsMap() const;
    bool AsBool() const;
    int AsInt() const;
    double AsDouble() const;
    const std::string& AsString() const;

    bool IsNull() const;
    bool IsArray() const;
    bool IsMap() const;
    bool IsBool() const;
    bool IsInt() const;
    bool IsDouble() const;
    bool IsPureDouble() const;
    bool IsString() const;

    const NodeValue& GetValue() const;

private:
    // NodeValue value_;
};

bool operator==(const Node& left, const Node& right);
bool operator!=(const Node& left, const Node& right);

class Document {
public:
    explicit Document() = default;
    explicit Document(Node root);

    const Node& GetRoot() const;

private:
    Node root_;
};

struct DocumentPrinter {
    std::ostream& os;
    void operator()(std::nullptr_t);
    void operator()(const Array& array);
    void operator()(const Dict& map);
    void operator()(bool value);
    void operator()(int value);
    void operator()(double value);
    void operator()(const std::string& line);
};

bool operator==(const Document& left, const Document& right);
bool operator!=(const Document& left, const Document& right);

Document Load(std::istream& input);

void Print(const Document& doc, std::ostream& output);

}  // namespace json
