#pragma once

#include "json.h"

#include <string>
#include <vector>

namespace json {

/**
 * Класс создает json-документ
*/
class Builder {
public:
    class ItemContext;
    class KeyItemContext;
    class ValueItemContext;
    class DictItemContext;
    class ArrayItemContext;

    Builder();

    Builder& Value(Node value);

    ArrayItemContext StartArray();
    Builder& EndArray();

    DictItemContext StartDict();
    Builder& EndDict();
    KeyItemContext Key(const std::string& key);

    Node Build();

private:
    Node root_; // Конструируемый объект
    std::vector<Node*> nodes_stack_; // Контейнер крайних недостроенных узлов

    Builder& AddItem(Node node, bool is_insertation_required = false);
};

/**
 * Классы, помогающие выявить часть ошибок в создании json-документа на этапе компиляции
 */

class Builder::ItemContext : public Builder {
public:
    ItemContext(Builder& builder) : builder_(builder) {}

    Builder& Value(Node value);
    ArrayItemContext StartArray();
    Builder& EndArray();
    DictItemContext StartDict();
    Builder& EndDict();
    KeyItemContext Key(const std::string& key);

    Node Build() = delete;
private:
    Builder& builder_;
};

class Builder::KeyItemContext : public ItemContext {
public:
    KeyItemContext(Builder& builder) : ItemContext(builder) {}

    ValueItemContext Value(Node value);
    Builder& EndArray() = delete;
    Builder& EndDict() = delete;
    KeyItemContext Key(const std::string& key) = delete;
};
class Builder::ValueItemContext : public ItemContext {
public:
    ValueItemContext(Builder& builder) : ItemContext(builder) {}

    Builder& Value(Node value) = delete;
    ArrayItemContext StartArray() = delete;
    Builder& EndArray() = delete;
    DictItemContext StartDict() = delete;
};
class Builder::DictItemContext : public ItemContext {
public:
    DictItemContext(Builder& builder) : ItemContext(builder) {}

    Builder& Value(Node value) = delete;
    ArrayItemContext StartArray() = delete;
    Builder& EndArray() = delete;
    DictItemContext StartDict() = delete;
};
class Builder::ArrayItemContext : public ItemContext {
public:
    ArrayItemContext(Builder& builder) : ItemContext(builder) {}

    Builder& EndDict() = delete;
    KeyItemContext Key(const std::string& key) = delete;
    ArrayItemContext Value(Node value);
};


} // namespace json