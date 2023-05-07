#pragma once

#include "json.h"

#include <string>
#include <vector>

namespace json {

class Builder {
public:
    class ItemContext;
    class KeyItemContext;
    class ValueItemContext;
    class DictItemContext;
    class ArrayItemContext;

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
    // Начинает построение пары словаря [ключ, Node]
    KeyItemContext Key(const std::string& key);

    // Возвращает результирующий узел
    Node Build();

private:
    Node root_; // Конструируемый объект
    std::vector<Node*> nodes_stack_; // Контейнер крайних недостроенных узлов

    // В зависимости от передаваемых параметров вносит значение Value,
    // начинает построение словаря или массива в Builder
    Builder& AddItem(Node node, bool is_insertation_required = false);
};

/*
 * Классы, помогающие выявить часть ошибок еще на этапе компиляции
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