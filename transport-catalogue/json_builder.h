#pragma once

#include "json.h"

#include <string>
#include <vector>

namespace json {

class KeyItemContext;
class ValueItemContext;
class DictItemContext;
class ArrayItemContext;

class Builder {
public:
    Builder();

    // Вносит значение в крайний недостроенный узел
    Builder& Value(Node value);

    // Начинает построение массива
    ArrayItemContext StartArray();
    // Завершает построение массива
    Builder& EndArray();

    // Начинает построение словаря
    DictItemContext StartDict();
    // Завершает построение словаря
    Builder& EndDict();
    // Строит пару словаря [ключ, Node]
    KeyItemContext Key(const std::string& key);


    // Возвращает результирующий узел
    Node Build();

private:
    Node root_; // Конструируемый объект
    std::vector<Node*> nodes_stack_; // Контейнер крайних недостроенных узлов
};

/*
 * Классы, помогающие выявить часть ошибок еще на этапе компиляции
 */

class ItemContext : public Builder {
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

class KeyItemContext : public ItemContext {
public:
    KeyItemContext(Builder& builder) : ItemContext(builder) {}

    ValueItemContext Value(Node value);
    Builder& EndArray() = delete;
    Builder& EndDict() = delete;
    KeyItemContext Key(const std::string& key) = delete;
};
class ValueItemContext : public ItemContext {
public:
    ValueItemContext(Builder& builder) : ItemContext(builder) {}

    Builder& Value(Node value) = delete;
    ArrayItemContext StartArray() = delete;
    Builder& EndArray() = delete;
    DictItemContext StartDict() = delete;
};
class DictItemContext : public ItemContext {
public:
    DictItemContext(Builder& builder) : ItemContext(builder) {}

    Builder& Value(Node value) = delete;
    ArrayItemContext StartArray() = delete;
    Builder& EndArray() = delete;
    DictItemContext StartDict() = delete;
};
class ArrayItemContext : public ItemContext {
public:
    ArrayItemContext(Builder& builder) : ItemContext(builder) {}

    Builder& EndDict() = delete;
    KeyItemContext Key(const std::string& key) = delete;
    ArrayItemContext Value(Node value);
};


} // namespace json