# Транпортный справочник
## Краткое описание
Справочник является системой хранения транспортных маршрутов и обработки запросов к ней. Справочник умеет сериализовать и десериализовать переданные ему данные, что позволяет обращаться к справочнику несколько раз без необходимости каждый раз инициилизировать его, хранить сериализованные данные нескольких районов, городов и т.д.

Транспортный справочник принимает на вход данные в JSON-формате, выдает в том же формате ответы на запросы, кратчайшие маршруты между остановками и визуализацию карты с маршрутами в формате SVG. В справочнике реализованы парсеры SVG и JSON файлов. Объекты JSON поддерживают цепочки вызовов (method chaining) при конструировании, превращая ошибки применения данных формата JSON в ошибки компиляции.
## Компиляция
Сборка проекта осуществляется при помощи CMake с предустановленным libprotoc (либо аналогичным proto-генератором):
```
cmake . -DCMAKE_PREFIX_PATH=/path/to/protobuf/package
cmake --build .
```
## Запуск программы
Для создания базы данных транспортного справочника необходимо передать в качестве параметра make_base, данные в JSON-формате. Пример:
```
> transport_catalogue make_base <data.json
```
Пример содержания файла `data.json`:
```
{
    "serialization_settings": {
        "file": "transport_catalogue.db"
    },
    "routing_settings": {
        "bus_wait_time": 2,
        "bus_velocity": 30
    },
    "render_settings": {
        "width": 1200,
        "height": 500,
        "padding": 50,
        "stop_radius": 5,
        "line_width": 14,
        "bus_label_font_size": 20,
        "bus_label_offset": [
            7,
            15
        ],
        "stop_label_font_size": 18,
        "stop_label_offset": [
            7,
            -3
        ],
        "underlayer_color": [
            255,
            255,
            255,
            0.85
        ],
        "underlayer_width": 3,
        "color_palette": [
            "green",
            [
                255,
                160,
                0
            ],
            "red"
        ]
    },
    "base_requests": [
        {
            "type": "Bus",
            "name": "14",
            "stops": [
                "Улица Докучаева",
                "Улица Лизы Чайкиной"
            ],
            "is_roundtrip": true
        },
        {
            "type": "Stop",
            "name": "Улица Лизы Чайкиной",
            "latitude": 43.590317,
            "longitude": 39.746833,
            "road_distances": {
                "Улица Докучаева": 2000
            }
        },
        {
            "type": "Stop",
            "name": "Улица Докучаева",
            "latitude": 43.585586,
            "longitude": 39.733879
        }
    ]
}
  
```
Для отправки запросов на построение маршрутов и карты, необходимо передать параметр process_requests и сами запросы в json-формате:
```
> transport_catalogue process_requests <requests.json >out.json
```
Пример возможного содержимого `requests.json`:
```
{
    "serialization_settings": {
        "file": "transport_catalogue.db"
    },
    "stat_requests": [
        {
            "id": 218563507,
            "type": "Bus",
            "name": "14"
        },
        {
            "id": 508658276,
            "type": "Stop",
            "name": "Электросети"
        },
        {
            "id": 1964680131,
            "type": "Route",
            "from": "Морской вокзал",
            "to": "Параллельная улица"
        },
        {
            "id": 1359372752,
            "type": "Map"
        }
    ]
}
  
```
## Зависимости
* C++17
* Компилятор с поддержкой 17-ого стандарта
* libprotoc 3.12.4
* CMake 3.10
