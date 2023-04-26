#include "svg.h"

#include <string>
#include <iomanip>

namespace svg {

using namespace std::literals;

Rgb::Rgb(uint8_t r, uint8_t g, uint8_t b) : red(r), green(g), blue(b) {}
Rgba::Rgba(uint8_t r, uint8_t g, uint8_t b, double op) : Rgb(r, g, b), opacity(op) {}

void ColorPrinter::operator()(std::monostate) { os << "none"; }
void ColorPrinter::operator()(std::string color) { os << color; }
void ColorPrinter::operator()(Rgb rgb) {
    os << "rgb(" << std::to_string(rgb.red) << ','
        << std::to_string(rgb.green) << ','
        << std::to_string(rgb.blue) << ')';
}
void ColorPrinter::operator()(Rgba rgba) {
    os << "rgba(" << std::to_string(rgba.red) << ','
        << std::to_string(rgba.green) << ','
        << std::to_string(rgba.blue) << ','
        << rgba.opacity << ')';
}

// Перегрузка оператора вывода для Color
std::ostream& operator<<(std::ostream& os, const Color& color) {
    std::visit(ColorPrinter{ os }, color);
    return os;
}
// Перегрузка оператора вывода для StrokeLineCap
std::ostream& operator<<(std::ostream& os, StrokeLineCap line_cap) {
    switch (line_cap) {
    case StrokeLineCap::BUTT:
        os << "butt"sv;
        break;
    case StrokeLineCap::ROUND:
        os << "round"sv;
        break;
    case StrokeLineCap::SQUARE:
        os << "square"sv;
        break;
    }

    return os;
}
// Перегрузка оператора вывода для StrokeLineJoin
std::ostream& operator<<(std::ostream& os, StrokeLineJoin line_join) {
    switch (line_join) {
    case StrokeLineJoin::ARCS:
        os << "arcs"sv;
        break;
    case StrokeLineJoin::BEVEL:
        os << "bevel"sv;
        break;
    case StrokeLineJoin::MITER:
        os << "miter"sv;
        break;
    case StrokeLineJoin::MITER_CLIP:
        os << "miter-clip"sv;
        break;
    case StrokeLineJoin::ROUND:
        os << "round"sv;
        break;
    }

    return os;
}

/* ---------- Object  ---------- */

void Object::Render(const RenderContext& context) const {
    context.RenderIndent();

    // Делегируем вывод тега своим подклассам
    RenderObject(context);

    context.out << std::endl;
}

/* ---------- Circle  ---------- */

// Задает координаты центра круга
Circle& Circle::SetCenter(Point center) {
    center_ = center;
    return *this;
}
// Задает радиус круга
Circle& Circle::SetRadius(double radius) {
    radius_ = radius;
    return *this;
}

// Рендер объекта
void Circle::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<circle cx=\""sv << center_.x << "\" cy=\""sv << center_.y << "\" "sv;
    out << "r=\""sv << radius_ << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}

/* ---------- Polyline  ---------- */

// Добавляет очередную вершину к ломаной линии
Polyline& Polyline::AddPoint(Point point) {
    points_.emplace_back(point);
    return *this;
}

// Рендер объекта
void Polyline::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    bool is_first = true;

    out << "<polyline points=\""sv;
    for (Point point : points_) {
        if (!is_first) {
            out << ' ';
        }
        is_first = false;
        out << point.x << ',' << point.y;
    }
    out << "\""sv;
    RenderAttrs(out);
    out << "/>"sv;
}

/* ---------- Text  ---------- */

// Задаёт координаты опорной точки (атрибуты x и y)
Text& Text::SetPosition(Point pos) {
    position_ = pos;
    return *this;
}
// Задаёт смещение относительно опорной точки (атрибуты dx, dy)
Text& Text::SetOffset(Point offset) {
    offset_ = offset;
    return *this;
}
// Задаёт размеры шрифта (атрибут font-size)
Text& Text::SetFontSize(uint32_t size) {
    font_.size = size;
    return *this;
}
// Задаёт название шрифта (атрибут font-family)
Text& Text::SetFontFamily(const std::string& font_family) {
    font_.font_family = font_family;
    return *this;
}
// Задаёт толщину шрифта (атрибут font-weight)
Text& Text::SetFontWeight(const std::string& font_weight) {
    font_.font_weight = font_weight;
    return *this;
}
// Задаёт текстовое содержимое объекта (отображается внутри тега text)
Text& Text::SetData(const std::string& data) {
    data_ = data;
    return *this;
}

// Рендер объекта
void Text::RenderObject(const RenderContext& context) const {
    auto& out = context.out;
    out << "<text "sv;

    out << "x=\""sv << position_.x << "\" "sv;
    out << "y=\""sv << position_.y << "\" "sv;

    out << "dx=\""sv << offset_.x << "\" "sv;
    out << "dy=\""sv << offset_.y << "\" "sv;

    out << "font-size=\""sv << font_.size << "\" "sv;
    
    if (font_.font_family) {
        out << "font-family=\""sv << *font_.font_family << "\" "sv;
    }
    if (font_.font_weight) {
        out << "font-weight=\""sv << *font_.font_weight << "\""sv;
    }

    RenderAttrs(out);
    out << ">";

    out << ConvertText(data_);

    out << "</text>"sv;
}

// Преобразует символы с строки, подходящие для SVG
std::string Text::ConvertText(const std::string& text) const {
    std::string output = "";

    for (const char ch : text) {
        switch (ch) {
        case '\"':
            output += "&quot;"sv;
            break;
        case '\'':
            output += "&apos;"sv;
            break;
        case '<':
            output += "&lt;"sv;
            break;
        case '>':
            output += "&gt;"sv;
            break;
        case '&':
            output += "&amp;"sv;
            break;
        default:
            output += ch;
        }
    }

    return output;
}

/* ---------- Document  ---------- */

// Добавляет в svg-документ объект-наследник svg::Object
void Document::AddPtr(std::unique_ptr<Object>&& obj) {
    objects_.emplace_back(std::move(obj));
}

// Выводит в ostream svg-представление документа
void Document::Render(std::ostream& out) const {
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"sv;
    out << "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n"sv;

    RenderContext ctx(out, 2, 1);
    for (const std::unique_ptr<Object>& obj : objects_) {
        ctx.RenderIndent();
        obj->Render(ctx);
    }

    out << "</svg>"sv;
}

}  // namespace svg
