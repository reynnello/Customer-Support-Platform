cmake_minimum_required(VERSION 3.16)
project(customer_support LANGUAGES C)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
find_package(raylib 5.5 QUIET)
if(NOT raylib_FOUND)
    include(FetchContent)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    FetchContent_Declare(raylib
        GIT_REPOSITORY https://github.com/raysan5/raylib.git
        GIT_TAG 5.5
        GIT_SHALLOW TRUE)
    FetchContent_MakeAvailable(raylib)
endif()
add_executable(customer_support src/main.c src/app.c src/ui.c
    src/ticket_create.c src/ticket_query.c src/ticket_edit.c src/storage.c)
target_link_libraries(customer_support PRIVATE raylib)
if(MSVC)
    target_compile_options(customer_support PRIVATE /W4)
else()
    target_compile_options(customer_support PRIVATE -Wall -Wextra -Wpedantic)
endif()
