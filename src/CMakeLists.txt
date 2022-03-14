find_package(fmt CONFIG)
find_package(spdlog CONFIG)

add_subdirectory(relay)

add_executable(example main.cpp)

target_link_libraries(
        example
        PUBLIC
        relay
        PRIVATE project_options
        project_warnings
        fmt::fmt
        spdlog::spdlog
)
