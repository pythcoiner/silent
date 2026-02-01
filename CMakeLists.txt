cmake_minimum_required(VERSION 3.22)

set(BIN templar)
project(${BIN} VERSION 0.1.0 LANGUAGES CXX)

# Build templar Rust crate before main project
add_custom_target(
    RunBeforeBuild ALL
    COMMAND bash build.sh
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    VERBATIM
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Import Rust static library
add_library(templar_rust STATIC IMPORTED)
set_target_properties(templar_rust PROPERTIES
    IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/libtemplar.a
)

find_package(Qt6 REQUIRED COMPONENTS Widgets Gui Core)

# Fetch qontrol framework from GitHub
include(FetchContent)
if(NOT DEFINED ENV{OFFLINE} OR "$ENV{OFFLINE}" STREQUAL "false")
    FetchContent_Declare(qontrol
        GIT_REPOSITORY https://github.com/pythcoiner/qontrol.git
        GIT_TAG 4444fc3
    )
else()
    FetchContent_Declare(qontrol
        SOURCE_DIR ./_deps/qontrol-src/
    )
endif()
FetchContent_MakeAvailable(qontrol)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

set(SOURCES
    src/main.cpp
    src/AppController.h
    src/AppController.cpp
    src/MainWindow.h
    src/MainWindow.cpp
    src/AccountController.h
    src/AccountController.cpp
    src/AccountWidget.h
    src/AccountWidget.cpp
    src/screens/MenuTab.h
    src/screens/MenuTab.cpp
    src/screens/modals/CreateAccount.h
    src/screens/modals/CreateAccount.cpp
)

qt_add_executable(${BIN}
    ${SOURCES}
)

# Ensure Rust library is built before executable
add_dependencies(${BIN} RunBeforeBuild)

add_dependencies(${BIN}
    qontrol
)

target_include_directories(${BIN}
  PRIVATE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/lib/include>
)

target_link_libraries(${BIN}
    PRIVATE
    Qt6::Gui
    Qt6::Core
    Qt6::Widgets
    qontrol
    templar_rust
    ssl
    crypto
    pthread
    dl
)

set_target_properties(${BIN} PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
