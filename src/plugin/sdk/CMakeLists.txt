add_library(silent_plugin_sdk INTERFACE)

target_include_directories(silent_plugin_sdk
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
)

add_executable(silent_plugin_sdk_smoke
    interfaces/module.h
    interfaces/account.h
    interfaces/feed.h
    interfaces/signer.h
    interfaces/theme.h
    interfaces/instance.h
    host.h
    smoke/mock.cpp
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
