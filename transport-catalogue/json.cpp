#include "json.h"

using namespace std;

namespace json {

namespace {

Node LoadNode(istream& input);
Node LoadString(istream& input);

Node LoadArray(istream& input) {
    Array result;

    for (char c; input >> c;) {
        if (c == ']') {
            return Node(std::move(result));
        }
        if (c != ',') {
            input.putback(c);
        }
        result.push_back(LoadNode(input));
    }

    throw ParsingError("Error while parsing Array"s);
}

Node LoadDict(istream& input) {
    Dict result;

    for (char c; input >> c;) {
        if (c == '}') {
            return Node(std::move(result));
        }
        if (c == ',') {
            input >> c;
        }

        string key = LoadString(input).AsString();
        input >> c;
        result.insert({ std::move(key), LoadNode(input) });
    }

    throw ParsingError("Error while parsing Dict"s);
}

Node LoadNumber(istream& input) {
    string parsed_num;

    // Считывает в parsed_num очередной символ из input
    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    // Считывает одну или более цифр в parsed_num из input
    auto read_digits = [&input, read_char] {
        if (!isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }
    // Парсим целую часть числа
    if (input.peek() == '0') {
        read_char();
        // После 0 в JSON не могут идти другие цифры
    }
    else {
        read_digits();
    }

    bool is_int = true;
    // Парсим дробную часть числа
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    // Парсим экспоненциальную часть числа
    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            // Сначала пробуем преобразовать строку в int
            try {
                return Node(stoi(parsed_num));
            }
            catch (...) {
                // В случае неудачи, например, при переполнении,
                // код ниже попробует преобразовать строку в double
            }
        }
        return Node(stod(parsed_num));
    }
    catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

// Считывает содержимое строкового литерала JSON-документа
// Функцию следует использовать после считывания открывающего символа ":
Node LoadString(istream& input) {
    auto it = std::istreambuf_iterator<char>(input);
    auto end = std::istreambuf_iterator<char>();
    string s;
    while (true) {
        if (it == end) {
            // Поток закончился до того, как встретили закрывающую кавычку?
            throw ParsingError("String parsing error"s);
        }
        const char ch = *it;
        if (ch == '"') {
            // Встретили закрывающую кавычку
            ++it;
            break;
        }
        else if (ch == '\\') {
            // Встретили начало escape-последовательности
            ++it;
            if (it == end) {
                // Поток завершился сразу после символа обратной косой черты
                throw ParsingError("String parsing error"s);
            }
            const char escaped_char = *(it);
            // Обрабатываем одну из последовательностей: \\, \n, \t, \r, \"
            switch (escaped_char) {
            case 'n':
                s.push_back('\n');
                break;
            case 't':
                s.push_back('\t');
                break;
            case 'r':
                s.push_back('\r');
                break;
            case '"':
                s.push_back('"');
                break;
            case '\\':
                s.push_back('\\');
                break;
            default:
                // Встретили неизвестную escape-последовательность
                throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        }
        else if (ch == '\n' || ch == '\r') {
            // Строковый литерал внутри- JSON не может прерываться символами \r или \n
            throw ParsingError("Unexpected end of line"s);
        }
        else {
            // Просто считываем очередной символ и помещаем его в результирующую строку
            s.push_back(ch);
        }
        ++it;
    }

    return Node(std::move(s));
}
// Метод возвращает цельное слово из текущего потока ввода
string LoadLiterals(istream& input) {
    string output;
    while (isalpha(input.peek())) {
        output += static_cast<char>(input.get());
    }
    return output;
}
Node LoadBool(istream& input) {
    const string& s = LoadLiterals(input);
    if (s == "true") {
        return Node(true);
    }
    else if (s == "false") {
        return Node(false);
    }

    throw ParsingError("Error while parsing bool"s);
}
Node LoadNull(istream& input) {
    const string& s = LoadLiterals(input);
    if (s == "null") {
        return Node(nullptr_t());
    }

    throw ParsingError("Error while parsing null"s);
}

Node LoadNode(istream& input) {
    char c;
    if (!(input >> c)) {
        throw ParsingError("Wrong input stream to parse"s);
    }

    switch (c) {
    case '[':
        return LoadArray(input);
    case '{':
        return LoadDict(input);
    case '"':
        return LoadString(input);
    case 'f':
        [[fallthrough]];
    case 't':
        input.putback(c);
        return LoadBool(input);
    case 'n':
        input.putback(c);
        return LoadNull(input);
    default:
        input.putback(c);
        return LoadNumber(input);
    }
}

}  // namespace

const Array& Node::AsArray() const {
    if (!IsArray()) {
        throw logic_error("Value doesn't contain Array"s);
    }
    return get<Array>(*this);
}
const Dict& Node::AsMap() const {
    if (!IsMap()) {
        throw logic_error("Value doesn't contain Dict"s);
    }
    return get<Dict>(*this);
}
bool Node::AsBool() const {
    if (!IsBool()) {
        throw logic_error("Value doesn't contain bool"s);
    }
    return get<bool>(*this);
}
int Node::AsInt() const {
    if (!IsInt()) {
        throw logic_error("Value doesn't contain int"s);
    }
    return get<int>(*this);
}
double Node::AsDouble() const {
    if (!IsDouble()) {
        throw logic_error("Value doesn't contain double or int"s);
    }
    if (IsPureDouble()) {
        return get<double>(*this);
    }
    return get<int>(*this);
}
const string& Node::AsString() const {
    if (!IsString()) {
        throw logic_error("Value doesn't contain std::string"s);
    }
    return get<string>(*this);
}

bool Node::IsNull() const { return this->index() == 0; }
bool Node::IsArray() const { return this->index() == 1; }
bool Node::IsMap() const { return this->index() == 2; }
bool Node::IsBool() const { return this->index() == 3; }
bool Node::IsInt() const { return this->index() == 4; }
bool Node::IsDouble() const {
    return this->index() == 4
        || this->index() == 5;
}
bool Node::IsPureDouble() const { return this->index() == 5; }
bool Node::IsString() const { return this->index() == 6; }

const NodeValue& Node::GetValue() const { return *this; }

bool operator==(const Node& left, const Node& right) {
    return left.GetValue() == right.GetValue();
}
bool operator!=(const Node& left, const Node& right) {
    return !(left == right);
}

Document::Document(Node root)
    : root_(std::move(root)) {
}

const Node& Document::GetRoot() const {
    return root_;
}

void DocumentPrinter::operator()(nullptr_t) { os << "null"s; }
void DocumentPrinter::operator()(const Array& array) {
    os << '[';
    bool is_first = true;
    for (const Node& node : array) {
        if (!is_first) {
            os << ", "sv;
        }
        is_first = false;
        visit(DocumentPrinter{ os }, node.GetValue());
    }
    os << ']';
}
void DocumentPrinter::operator()(const Dict& map) {
    os << '{';
    bool is_first = true;
    for (const auto& [key, node] : map) {
        if (!is_first) {
            os << ", "s;
        }
        is_first = false;
        os << '"' << key << "\": ";
        visit(DocumentPrinter{ os }, node.GetValue());
    }
    os << '}';
}
void DocumentPrinter::operator()(bool value) {
    os << (value ? "true"s : "false"s);
}
void DocumentPrinter::operator()(int value) { os << value; }
void DocumentPrinter::operator()(double value) { os << value; }
void DocumentPrinter::operator()(const string& line) {
    os << '"';
    for (const char ch : line) {
        switch (ch) {
        case '\\':
            os << "\\\\"s;
            break;
        case '\n':
            os << "\\n"s;
            break;
        case '\r':
            os << "\\r"s;
            break;
        case '"':
            os << "\\\""s;
            break;
        default:
            os << ch;
        }
    }
    os << '"';
}

bool operator==(const Document& left, const Document& right) {
    return left.GetRoot() == right.GetRoot();
}
bool operator!=(const Document& left, const Document& right) {
    return !(left == right);
}

Document Load(istream& input) {
    return Document{ LoadNode(input) };
}

void Print(const Document& doc, std::ostream& output) {
    visit(DocumentPrinter{ output }, doc.GetRoot().GetValue());
}

}  // namespace json
