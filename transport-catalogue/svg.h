#pragma once

#include <cstdint>
#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace svg {

// Структура хранит информацию о цвете в формате RGB
struct Rgb {
    Rgb() = default;
    Rgb(uint8_t r, uint8_t g, uint8_t b);

    uint8_t red = 0;
    uint8_t green = 0;
    uint8_t blue = 0;
};
// Структура хранит информацию о цвете в формате RGBA
struct Rgba : public Rgb {
    Rgba() = default;
    Rgba(uint8_t r, uint8_t g, uint8_t b, double op);

    double opacity = 1.0;
};

using Color = std::variant<std::monostate, std::string, Rgb, Rgba>;
inline const Color NoneColor{ std::monostate() };
// Вспомонательный класс для вывода цвета
struct ColorPrinter {
    std::ostream& os;
    void operator()(std::monostate);
    void operator()(std::string color);
    void operator()(Rgb rgb);
    void operator()(Rgba rgba);
};

// Тип форма конца линии
enum class StrokeLineCap {
    BUTT,
    ROUND,
    SQUARE,
};
// Тип формы соединения линии
enum class StrokeLineJoin {
    ARCS,
    BEVEL,
    MITER,
    MITER_CLIP,
    ROUND,
};

// Перегрузка оператора вывода для Color
std::ostream& operator<<(std::ostream& os, const Color& color);
// Перегрузка оператора вывода для StrokeLineCap
std::ostream& operator<<(std::ostream& os, StrokeLineCap line_cap);
// Перегрузка оператора вывода для StrokeLineJoin
std::ostream& operator<<(std::ostream& os, StrokeLineJoin line_join);

template <typename Owner>
class PathProps {
public:
    Owner& SetFillColor(Color color) {
        fill_color_ = std::move(color);
        return AsOwner();
    }
    Owner& SetStrokeColor(Color color) {
        stroke_color_ = std::move(color);
        return AsOwner();
    }
    Owner& SetStrokeWidth(double width) {
        stroke_width_ = width;
        return AsOwner();
    }
    Owner& SetStrokeLineCap(StrokeLineCap line_cap) {
        line_cap_ = line_cap;
        return AsOwner();
    }
    Owner& SetStrokeLineJoin(StrokeLineJoin line_join) {
        line_join_ = line_join;
        return AsOwner();
    }

protected:
    ~PathProps() = default;

    void RenderAttrs(std::ostream& out) const {
        if (fill_color_) {
            out << " fill=\"" << *fill_color_ << "\"";
        }
        if (stroke_color_) {
            out << " stroke=\"" << *stroke_color_ << "\"";
        }
        if (stroke_width_) {
            out << " stroke-width=\"" << *stroke_width_ << "\"";
        }
        if (line_cap_) {
            out << " stroke-linecap=\"" << *line_cap_ << "\"";
        }
        if (line_join_) {
            out << " stroke-linejoin=\"" << *line_join_ << "\"";
        }
    }

private:
    Owner& AsOwner() {
        return static_cast<Owner&>(*this);
    }

    std::optional<Color> fill_color_;
    std::optional<Color> stroke_color_;
    std::optional<double> stroke_width_;
    std::optional<StrokeLineCap> line_cap_;
    std::optional<StrokeLineJoin> line_join_;
};

struct Point {
    Point() = default;
    Point(double x, double y)
        : x(x)
        , y(y) {
    }
    double x = 0;
    double y = 0;
    bool operator==(const Point& other) const { return x == other.x && y == other.y; }
};

/*
 * Вспомогательная структура, хранящая контекст для вывода SVG-документа с отступами.
 * Хранит ссылку на поток вывода, текущее значение и шаг отступа при выводе элемента
 */
struct RenderContext {
    RenderContext(std::ostream& out)
        : out(out) {
    }

    RenderContext(std::ostream& out, int indent_step, int indent = 0)
        : out(out)
        , indent_step(indent_step)
        , indent(indent) {
    }

    RenderContext Indented() const {
        return { out, indent_step, indent + indent_step };
    }

    void RenderIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    std::ostream& out;
    int indent_step = 0;
    int indent = 0;
};

/*
 * Абстрактный базовый класс Object служит для унифицированного хранения
 * конкретных тегов SVG-документа
 * Реализует паттерн "Шаблонный метод" для вывода содержимого тега
 */
class Object {
public:
    void Render(const RenderContext& context) const;

    virtual ~Object() = default;

private:
    virtual void RenderObject(const RenderContext& context) const = 0;
};

/*
 * Класс Circle моделирует элемент <circle> для отображения круга
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/circle
 */
class Circle final : public Object, public PathProps<Circle> {
public:
    // Задает координаты центра круга
    Circle& SetCenter(Point center);
    // Задает радиус круга
    Circle& SetRadius(double radius);

private:
    // Рендер объекта
    void RenderObject(const RenderContext& context) const override;

    Point center_; // Координаты центра круга 
    double radius_ = 1.0; // Радиус
};

/*
 * Класс Polyline моделирует элемент <polyline> для отображения ломаных линий
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/polyline
 */
class Polyline final : public Object, public PathProps<Polyline> {
public:
    // Добавляет очередную вершину к ломаной линии
    Polyline& AddPoint(Point point);

private:
    // Рендер объекта
    void RenderObject(const RenderContext& context) const override;

    std::vector<Point> points_;
};

/*
 * Класс Text моделирует элемент <text> для отображения текста
 * https://developer.mozilla.org/en-US/docs/Web/SVG/Element/text
 */
class Text final : public Object, public PathProps<Text> {
public:
    // Задаёт координаты опорной точки (атрибуты x и y) 
    Text& SetPosition(Point pos);
    // Задаёт смещение относительно опорной точки (атрибуты dx, dy) 
    Text& SetOffset(Point offset);
    // Задаёт размеры шрифта (атрибут font-size)
    Text& SetFontSize(uint32_t size);
    // Задаёт название шрифта (атрибут font-family) 
    Text& SetFontFamily(const std::string& font_family);
    // Задаёт толщину шрифта (атрибут font-weight)
    Text& SetFontWeight(const std::string& font_weight);
    // Задаёт текстовое содержимое объекта (отображается внутри тега text)
    Text& SetData(const std::string& data);

private:
    // Информация о шрифте
    struct Font {
        uint32_t size = 1; // Размер шрифта
        std::optional<std::string> font_family; // Название шрифта
        std::optional<std::string> font_weight; // Толщина шрифта
    };

    Point position_; // Координаты опорной точки
    Point offset_; // Смещение относительно опорной точки
    Font font_; // Информация о шрифте
    std::string data_; // Содержимое текста

    // Рендер объекта
    void RenderObject(const RenderContext& context) const override;

    // Преобразует символы с строки, подходящие для SVG
    std::string ConvertText(const std::string& text) const;
};

class ObjectContainer {
public:
    // Добавляет в svg-контейнер любой объект-наследник svg::Object
    template <typename T>
    void Add(T obj) {
        AddPtr(std::make_unique<T>(std::move(obj)));
    }

    // Добавляет в svg-документ объект-наследник svg::Object
    virtual void AddPtr(std::unique_ptr<Object>&& ptr) = 0;

    virtual ~ObjectContainer() = default;
};

class Document : public ObjectContainer {
public:
    // Добавляет в svg-документ объект-наследник svg::Object
    void AddPtr(std::unique_ptr<Object>&& obj) override;

    // Выводит в ostream svg-представление документа
    void Render(std::ostream& out) const;

private:
    std::vector<std::unique_ptr<Object>> objects_;
};

class Drawable {
public:
    virtual void Draw(ObjectContainer& container) const = 0;

    virtual ~Drawable() = default;
};

}  // namespace svg
