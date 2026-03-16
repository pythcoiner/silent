cmake_minimum_required(VERSION 3.22)

set(BIN silent)
project(${BIN} VERSION 0.1.0 LANGUAGES CXX)

# Build silent Rust crate before main project
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
add_library(silent_rust STATIC IMPORTED)
set_target_properties(silent_rust PROPERTIES
    IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/lib/libsilent.a
)

find_package(Qt6 REQUIRED COMPONENTS Widgets Gui Core Svg)

# qontrol framework (vendored in lib/qontrol)
add_subdirectory(lib/qontrol)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

set(SOURCES
    src/main.cpp
    src/theme/Theme.h
    src/theme/Theme.cpp
    src/theme/Palette.h
    src/theme/Palette.cpp
    src/theme/Button.h
    src/theme/Button.cpp
    src/theme/Label.h
    src/theme/Label.cpp
    src/theme/Input.h
    src/theme/Input.cpp
    src/theme/Display.h
    src/theme/Display.cpp
    src/AppController.h
    src/AppController.cpp
    src/MainWindow.h
    src/MainWindow.cpp
    src/AccountController.h
    src/AccountController.cpp
    src/AccountWidget.h
    src/AccountWidget.cpp
    src/StatusBar.h
    src/StatusBar.cpp
    src/screens/MenuTab.h
    src/screens/MenuTab.cpp
    src/screens/modals/CreateAccount.h
    src/screens/modals/CreateAccount.cpp
    src/screens/utils.h
    src/screens/utils.cpp
    src/screens/Coins.h
    src/screens/Coins.cpp
    src/screens/Receive.h
    src/screens/Receive.cpp
    src/screens/Send.h
    src/screens/Send.cpp
    src/screens/Settings.h
    src/screens/Settings.cpp
    src/screens/modals/ConfirmDelete.h
    src/screens/modals/ConfirmDelete.cpp
    src/screens/modals/ConfirmSend.h
    src/screens/modals/ConfirmSend.cpp
    src/theme/Icon.h
    src/theme/Icon.cpp
    src/theme/Tab.h
    src/theme/Tab.cpp
    src/theme/Checkbox.h
    src/theme/Checkbox.cpp
    src/theme/Toggle.h
    src/theme/Toggle.cpp
    src/theme/ComboBox.h
    src/theme/ComboBox.cpp
    src/theme/TextEdit.h
    src/theme/TextEdit.cpp
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
    Qt6::Svg
    qontrol
    silent_rust
    ssl
    crypto
    pthread
    dl
    udev
)

set_target_properties(${BIN} PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
