#include "json_builder.h"

using namespace std;

namespace json {

Builder::Builder() {
    nodes_stack_.emplace_back(&root_);
}

// Вносит значение в крайний недостроенный узел
Builder& Builder::Value(Node value) {
    // Если стек недостроенных узлов пуст - выбрасываем исключение
    if (nodes_stack_.empty()) {
        throw logic_error("Nodes stack is empty while adding value"s);
    }

    // Если крайний узел - массив, вставляем в него элемент value
    if (nodes_stack_.back()->IsArray()) {
        const_cast<Array&>(nodes_stack_.back()->AsArray()).push_back(value);
    }
    // Иначе - присываиваем крайнему узлу значение элемента value
    else {
        *nodes_stack_.back() = value;
        nodes_stack_.pop_back();
    }

    return *this;
}

// Начинает построение узла массива
ArrayItemContext Builder::StartArray() {
    // Если стек недостроенных узлов пуст - выбрасываем исключение
    if (nodes_stack_.empty()) {
        throw logic_error("Nodes stack is empty while starting array"s);
    }

    // Если крайний элемент является массивом - добавляем в него массив,
    // добавляем указатель на этот массив в nodes_stack_
    if (nodes_stack_.back()->IsArray()) {
        const_cast<Array&>(nodes_stack_.back()->AsArray()).push_back(Array());
        nodes_stack_.emplace_back(&const_cast<Array&>(nodes_stack_.back()->AsArray()).back());
    }
    // Иначе - присываиваем указателю крайнего узла ссылку на новый массив
    else {
        *nodes_stack_.back() = Array();
    }

    return *this;
}
// Завершает построение узла массива
Builder& Builder::EndArray() {
    // Если стек недостроенных узлов пуст или крайний незаконченный элемент
    // не является массивом - выбрасываем исключение logic_error
    if (nodes_stack_.empty() || !nodes_stack_.back()->IsArray()) {
        throw logic_error("Can't end array node"s);
    }
    nodes_stack_.pop_back();

    return *this;
}

// Начинает построение словаря
DictItemContext Builder::StartDict() {
    // Если стек недостроенных узлов пуст - выбрасываем исключение
    if (nodes_stack_.empty()) {
        throw logic_error("Nodes stack is empty while starting map"s);
    }

    // Если крайний элемент является массивом - добавляем в него словарь,
    // добавляем указатель на этот словарь в nodes_stack_
    if (nodes_stack_.back()->IsArray()) {
        const_cast<Array&>(nodes_stack_.back()->AsArray()).push_back(Dict());
        nodes_stack_.emplace_back(&const_cast<Array&>(nodes_stack_.back()->AsArray()).back());
    }
    // Иначе - присываиваем указателю крайнего узла ссылку на новый словарь
    else {
        *nodes_stack_.back() = Dict();
    }

    return *this;
}
// Завершает построение словаря
Builder& Builder::EndDict() {
    // Если стек недостроенных узлов пуст или крайний незаконченный элемент
    // не является словарем - выбрасываем исключение logic_error
    if (nodes_stack_.empty() || !nodes_stack_.back()->IsDict()) {
        throw logic_error("Can't end dict node"s);
    }
    nodes_stack_.pop_back();

    return *this;
}
// Строит пару словаря [ключ, Node]
KeyItemContext Builder::Key(const std::string& key) {
    // Если стек недостроенных узлов пуст или крайний незаконченный элемент
    // не является словарем - выбрасываем исключение logic_error
    if (nodes_stack_.empty() || !nodes_stack_.back()->IsDict()) {
        throw logic_error("Key isn't inside the map"s);
    }
    nodes_stack_.emplace_back(&const_cast<Dict&>(nodes_stack_.back()->AsDict())[key]);

    return *this;
}

// Возвращает результирующий узел
Node Builder::Build() {
    // Если есть незаконченные узлы - выбрасываем исключение logic_error
    if (!nodes_stack_.empty()) {
        throw logic_error("Can't build with unfinished nodes"s);
    }

    return root_;
}

/* 
 * Реализации вспомогательных классов
 */

Builder& ItemContext::Value(Node value) { return builder_.Value(move(value)); }
ArrayItemContext ItemContext::StartArray() { return builder_.StartArray(); }
Builder& ItemContext::EndArray() { return builder_.EndArray(); }
DictItemContext ItemContext::StartDict() { return builder_.StartDict(); }
Builder& ItemContext::EndDict() { return builder_.EndDict(); }
KeyItemContext ItemContext::Key(const std::string& key) { return builder_.Key(key); }

ValueItemContext KeyItemContext::Value(Node value) { return ItemContext::Value(value); }

ArrayItemContext ArrayItemContext::Value(Node value) { return ItemContext::Value(value); }

} // namespace json
