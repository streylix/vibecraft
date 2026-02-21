cmake_minimum_required(VERSION 3.20)
project(VibeCraft VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- Dependencies via vcpkg ---
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)
if(NOT APPLE)
    find_package(OpenGL REQUIRED)
endif()
find_package(GTest CONFIG REQUIRED)
find_package(ZLIB REQUIRED)

# --- Game library (shared between game executable and tests) ---
file(GLOB_RECURSE VIBECRAFT_SOURCES CONFIGURE_DEPENDS "src/*.cpp")

# Filter out main.cpp so we can build a library without the entry point
list(FILTER VIBECRAFT_SOURCES EXCLUDE REGEX ".*main\\.cpp$")

add_library(vibecraft_lib STATIC ${VIBECRAFT_SOURCES})
target_include_directories(vibecraft_lib PUBLIC "${CMAKE_SOURCE_DIR}/include")
target_link_libraries(vibecraft_lib PUBLIC
    glfw
    glm::glm
    ZLIB::ZLIB
)
if(APPLE)
    target_link_libraries(vibecraft_lib PUBLIC "-framework OpenGL")
else()
    target_link_libraries(vibecraft_lib PUBLIC OpenGL::GL)
endif()

# --- Game executable ---
add_executable(vibecraft src/main.cpp)
target_link_libraries(vibecraft PRIVATE vibecraft_lib)

# --- Tests ---
enable_testing()

file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "tests/*.cpp")

foreach(TEST_SOURCE ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
    add_executable(${TEST_NAME} ${TEST_SOURCE})
    target_link_libraries(${TEST_NAME} PRIVATE
        vibecraft_lib
        GTest::gtest_main
    )
    target_compile_definitions(${TEST_NAME} PRIVATE
        VIBECRAFT_ASSET_DIR="${CMAKE_SOURCE_DIR}/assets"
    )
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()
