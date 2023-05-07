#include "json_builder.h"

namespace json {

Builder::Builder() {
    nodes_stack_.emplace_back(&root_);
}

// Вносит значение в крайний недостроенный узел
Builder& Builder::Value(Node value) {
    return AddItem(std::move(value));
}

// Начинает построение узла массива
Builder::ArrayItemContext Builder::StartArray() {
    return AddItem(Array(), true);
}
// Завершает построение узла массива
Builder& Builder::EndArray() {
    // Если стек недостроенных узлов пуст или крайний незаконченный элемент
    // не является массивом - выбрасываем исключение logic_error
    if (nodes_stack_.empty() || !nodes_stack_.back()->IsArray()) {
        using namespace std::literals;
        throw std::logic_error("Can't end array node"s);
    }
    nodes_stack_.pop_back();

    return *this;
}

// Начинает построение словаря
Builder::DictItemContext Builder::StartDict() {
    return AddItem(Dict(), true);
}
// Завершает построение словаря
Builder& Builder::EndDict() {
    // Если стек недостроенных узлов пуст или крайний незаконченный элемент
    // не является словарем - выбрасываем исключение logis_insertation_requiredic_error
    if (nodes_stack_.empty() || !nodes_stack_.back()->IsDict()) {
        using namespace std::literals;
        throw std::logic_error("Can't end dict node"s);
    }
    nodes_stack_.pop_back();

    return *this;
}
// Строит пару словаря [ключ, Node]
Builder::KeyItemContext Builder::Key(const std::string& key) {
    // Если стек недостроенных узлов пуст или крайний незаконченный элемент
    // не является словарем - выбрасываем исключение logic_error
    if (nodes_stack_.empty() || !nodes_stack_.back()->IsDict()) {
        using namespace std::literals;
        throw std::logic_error("Key isn't inside the map"s);
    }
    nodes_stack_.emplace_back(&const_cast<Dict&>(nodes_stack_.back()->AsDict())[key]);

    return *this;
}

// Возвращает результирующий узел
Node Builder::Build() {
    // Если есть незаконченные узлы - выбрасываем исключение logic_error
    if (!nodes_stack_.empty()) {
        using namespace std::literals;
        throw std::logic_error("Can't build with unfinished nodes"s);
    }

    return root_;
}

// В зависимости от передаваемых параметров вносит значение Value,
// начинает построение словаря или массива в Builder
Builder& Builder::AddItem(Node node, bool is_insertation_required) {
    // Если стек недостроенных узлов пуст - выбрасываем исключение
    if (nodes_stack_.empty()) {
        using namespace std::literals;
        throw std::logic_error("Nodes stack is empty while adding item"s);
    }

    // Если крайний узел - массив, вставляем в него узел
    if (nodes_stack_.back()->IsArray()) {
        const_cast<Array&>(nodes_stack_.back()->AsArray()).push_back(std::move(node));
        // Если требуется вставка в стек недостроенных узлов - реализуем это
        if (is_insertation_required) {
            nodes_stack_.emplace_back(&const_cast<Array&>(nodes_stack_.back()->AsArray()).back());
        }
    }
    // Иначе - присываиваем значению по указателю крайнего узла значение нового узла
    else {
        *nodes_stack_.back() = std::move(node);
        // Если не требуется вставка в стек недостроенных узлов - удаляем краний указатель стека
        if (!is_insertation_required) {
            nodes_stack_.pop_back();
        }
    }

    return *this;
}

/* 
 * Реализации вспомогательных классов
 */

Builder& Builder::ItemContext::Value(Node value) { return builder_.Value(std::move(value)); }
Builder::ArrayItemContext Builder::ItemContext::StartArray() { return builder_.StartArray(); }
Builder& Builder::ItemContext::EndArray() { return builder_.EndArray(); }
Builder::DictItemContext Builder::ItemContext::StartDict() { return builder_.StartDict(); }
Builder& Builder::ItemContext::EndDict() { return builder_.EndDict(); }
Builder::KeyItemContext Builder::ItemContext::Key(const std::string& key) { return builder_.Key(key); }

Builder::ValueItemContext Builder::KeyItemContext::Value(Node value) { return ItemContext::Value(std::move(value)); }

Builder::ArrayItemContext Builder::ArrayItemContext::Value(Node value) { return ItemContext::Value(std::move(value)); }

} // namespace json
