# Минимальная версия CMake 3.10
cmake_minimum_required(VERSION 3.10)

project(TransportCatalogue CXX)

# Флаги компиляции и стандарт
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "-Werror -Wall -Wextra -pedantic")

# Необходим Protobuf для сериализации
find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

# Необходимые proto файлы для сериализации справочника
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS 
    transport_catalogue.proto map_renderer.proto svg.proto graph.proto transport_router.proto)

# Файлы траснпортного справочника
set(TC_FILES domain.cpp domain.h transport_catalogue.cpp transport_catalogue.h 
    transport_router.cpp transport_router.h)
# Файлы сериализации
set(SER_FILES serialization.cpp serialization.h transport_catalogue.proto)
# Файлы рендера карт
set(RENDERER_FILES geo.cpp geo.h map_renderer.cpp map_renderer.h map_renderer.proto)
# Файлы маршрутизатора
set(ROUTER_FILES graph.h ranges.h router.h graph.proto transport_router.proto)
# Файлы JSON
set(JSON_FILES json_builder.cpp json_builder.h json_reader.cpp json_reader.h json.cpp json.h)
# Файлы SVG
set(SVG_FILES svg.cpp svg.h svg.proto)
# Файлы запуска
set(RUN_FILES main.cpp request_handler.cpp request_handler.h)

# Создаем исполняемый файл transport_catalogue
add_executable(transport_catalogue 
    ${PROTO_SRCS} ${PROTO_HDRS} ${TC_FILES} ${SER_FILES} ${RENDERER_FILES} 
    ${ROUTER_FILES} ${JSON_FILES} ${SVG_FILES} ${RUN_FILES})

# Подсключаем Protobuf и нежные библиотеки
target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(transport_catalogue ${Protobuf_LIBRARY} Threads::Threads)