add_library(silent_plugin_sdk INTERFACE)

target_include_directories(silent_plugin_sdk
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)

target_link_libraries(silent_plugin_sdk
    INTERFACE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    qontrol
)

add_executable(silent_plugin_sdk_smoke
    smoke/main.cpp
)

set_target_properties(silent_plugin_sdk_smoke PROPERTIES
    AUTOMOC ON
)

target_link_libraries(silent_plugin_sdk_smoke
    PRIVATE
    silent_plugin_sdk
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
)
