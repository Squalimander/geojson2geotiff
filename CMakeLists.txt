cmake_minimum_required(VERSION 3.16)
project(geojson2geotiff C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
find_package(GDAL REQUIRED)
find_package(OpenMP REQUIRED)

add_library(cjson STATIC third_party/cjson/cJSON.c)

target_include_directories(cjson PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/cjson
)

add_executable(geojson2geotiff src/main.c src/cli.c src/geojson.c)

target_compile_options(geojson2geotiff PRIVATE
    $<$<C_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
)

target_link_libraries(
    geojson2geotiff PRIVATE GDAL::GDAL OpenMP::OpenMP_C cjson
)

target_include_directories(geojson2geotiff PRIVATE src)