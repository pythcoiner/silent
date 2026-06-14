cmake_minimum_required(VERSION 3.22)

set(BIN silent)
project(${BIN} VERSION 0.1.0 LANGUAGES CXX)

option(SILENT_I18N_TRACE_IDS "Emit i18n ID pragma trace markers" OFF)

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

# qontrol framework (prefer vendored lib/qontrol, otherwise fetch pinned commit)
if(EXISTS "${CMAKE_SOURCE_DIR}/lib/qontrol/CMakeLists.txt")
    add_subdirectory(lib/qontrol)
else()
    include(FetchContent)
    FetchContent_Declare(
        qontrol_src
        GIT_REPOSITORY https://github.com/pythcoiner/qontrol.git
        GIT_TAG 7bc317d
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(qontrol_src)
endif()
add_subdirectory(src/interfaces)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

set(SOURCES
    src/main.cpp
    src/QtAbiSurface.cpp
    src/host/Host.h
    src/host/Host.cpp
    src/host/PluginRegistry.h
    src/host/PluginRegistry.cpp
    src/interfaces/host.h
    src/interfaces/module.h
    src/interfaces/account.h
    src/interfaces/feed.h
    src/interfaces/signer.h
    src/interfaces/theme.h
    src/interfaces/instance.h
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
    src/i18n/I18nManager.h
    src/i18n/I18nManager.cpp
    src/i18n/Tr.h
    src/MainWindow.h
    src/MainWindow.cpp
    src/modules/sp/AccountController.h
    src/modules/sp/AccountController.cpp
    src/modules/sp/AccountWidget.h
    src/modules/sp/AccountWidget.cpp
    src/modules/sp/SpPlugin.h
    src/modules/sp/SpPlugin.cpp
    src/modules/sp/SpModule.h
    src/modules/sp/SpModule.cpp
    src/modules/sp/SpInstance.h
    src/modules/sp/SpInstance.cpp
    src/modules/sp/SpAccount.h
    src/modules/sp/SpAccount.cpp
    src/modules/sp/StatusBar.h
    src/modules/sp/StatusBar.cpp
    src/screens/MenuTab.h
    src/screens/MenuTab.cpp
    src/screens/modals/AppSettings.h
    src/screens/modals/AppSettings.cpp
    src/modules/sp/modals/CreateAccount.h
    src/modules/sp/modals/CreateAccount.cpp
    src/screens/utils.h
    src/screens/utils.cpp
    src/modules/sp/screens/History.h
    src/modules/sp/screens/History.cpp
    src/modules/sp/screens/Coins.h
    src/modules/sp/screens/Coins.cpp
    src/modules/sp/screens/Receive.h
    src/modules/sp/screens/Receive.cpp
    src/modules/sp/screens/Send.h
    src/modules/sp/screens/Send.cpp
    src/modules/sp/screens/Settings.h
    src/modules/sp/screens/Settings.cpp
    src/screens/modals/ConfirmDelete.h
    src/screens/modals/ConfirmDelete.cpp
    src/screens/modals/CustomLaunchers.cpp
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

# Discover all translator-editable catalogs.
# Any new `i18n/silent_<locale>.lang` file is embedded automatically.
file(GLOB TRANSLATION_LANG_FILES CONFIGURE_DEPENDS ${CMAKE_SOURCE_DIR}/i18n/silent_*.lang)

qt_add_executable(${BIN}
    ${SOURCES}
)

if(SILENT_I18N_TRACE_IDS)
    target_compile_definitions(${BIN} PRIVATE I18N_TRACE_IDS=1)
endif()

foreach(LANG_FILE ${TRANSLATION_LANG_FILES})
    get_filename_component(LANG_BASENAME ${LANG_FILE} NAME)
    set_source_files_properties(${LANG_FILE} PROPERTIES QT_RESOURCE_ALIAS ${LANG_BASENAME})
endforeach()

# Embed `.lang` files directly in the executable for single-binary distribution.
qt_add_resources(${BIN} embedded_i18n_lang
    PREFIX "/i18n"
    FILES ${TRANSLATION_LANG_FILES}
)

# Ensure Rust library is built before executable
add_dependencies(${BIN} RunBeforeBuild)

add_dependencies(${BIN}
    qontrol
)

target_include_directories(${BIN}
  PRIVATE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src/modules/sp>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/lib/include>
)

target_link_libraries(${BIN}
    PRIVATE
    silent_rust
    ssl
    crypto
    pthread
    dl
    udev
)

if(UNIX AND NOT APPLE AND (CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang"))
    target_link_options(${BIN} PRIVATE
        "-Wl,--export-dynamic"
    )

    set(SILENT_QT_AND_QONTROL_LIBS
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Svg
        qontrol
    )

    set(SILENT_ELF_LINK_LIBS)
    foreach(SILENT_LIB ${SILENT_QT_AND_QONTROL_LIBS})
        get_target_property(SILENT_LIB_TYPE ${SILENT_LIB} TYPE)
        if(SILENT_LIB_TYPE STREQUAL "STATIC_LIBRARY")
            list(APPEND SILENT_ELF_LINK_LIBS "$<LINK_LIBRARY:WHOLE_ARCHIVE,${SILENT_LIB}>")
        else()
            list(APPEND SILENT_ELF_LINK_LIBS "${SILENT_LIB}")
        endif()
    endforeach()

    target_link_libraries(${BIN} PRIVATE ${SILENT_ELF_LINK_LIBS})
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    # Windows host-symbol export for plugin DLLs. Plugins link only against the
    # host import library, so a single static Qt lives in the host and plugins
    # resolve the host ABI from it. qontrol and the QtCore/Gui/Widgets/Svg modules
    # are whole-archived so their full symbol set is present in the image and can
    # be re-exported: a plugin may use any of those modules, not just the subset
    # the host itself calls. A curated export table (scripts/gen_host_exports.sh)
    # keeps the PE export count under the 64K ordinal cap that plain
    # --export-all-symbols overflows.
    target_link_libraries(${BIN}
        PRIVATE
        "$<LINK_LIBRARY:WHOLE_ARCHIVE,Qt6::Core>"
        "$<LINK_LIBRARY:WHOLE_ARCHIVE,Qt6::Gui>"
        "$<LINK_LIBRARY:WHOLE_ARCHIVE,Qt6::Widgets>"
        "$<LINK_LIBRARY:WHOLE_ARCHIVE,Qt6::Svg>"
        "$<LINK_LIBRARY:WHOLE_ARCHIVE,qontrol>"
    )

    if(NOT CMAKE_NM)
        find_program(CMAKE_NM NAMES x86_64-w64-mingw32-nm nm)
    endif()

    set(SILENT_HOST_DEF "${CMAKE_CURRENT_BINARY_DIR}/silent_host_exports.def")
    add_custom_command(TARGET ${BIN} PRE_LINK
        COMMAND bash "${CMAKE_SOURCE_DIR}/scripts/gen_host_exports.sh"
                "${CMAKE_NM}"
                "$<TARGET_FILE:qontrol>"
                "$<TARGET_FILE_DIR:Qt6::Core>"
                "${SILENT_HOST_DEF}"
                "$<TARGET_OBJECTS:${BIN}>"
        COMMAND_EXPAND_LISTS
        VERBATIM
        COMMENT "Generating curated Windows host export table (plugin ABI)"
    )

    target_link_options(${BIN} PRIVATE
        "-Wl,--exclude-all-symbols"
        "${SILENT_HOST_DEF}"
    )

    # Build the import library that plugin DLLs link against. We generate it from
    # the same .def with dlltool rather than the linker's --out-implib: the latter
    # is tied to ld's auto-export path, which --exclude-all-symbols disables, so it
    # produces nothing here.
    if(NOT CMAKE_DLLTOOL)
        find_program(CMAKE_DLLTOOL NAMES x86_64-w64-mingw32-dlltool dlltool)
    endif()
    set(SILENT_HOST_IMPLIB "${CMAKE_CURRENT_BINARY_DIR}/libsilent_host_exports.a")
    add_custom_command(TARGET ${BIN} POST_BUILD
        COMMAND "${CMAKE_DLLTOOL}"
                --input-def "${SILENT_HOST_DEF}"
                --dllname "$<TARGET_FILE_NAME:${BIN}>"
                --output-lib "${SILENT_HOST_IMPLIB}"
        BYPRODUCTS "${SILENT_HOST_IMPLIB}"
        VERBATIM
        COMMENT "Generating host import library for plugin DLLs"
    )
else()
    target_link_libraries(${BIN}
        PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        Qt6::Svg
        qontrol
    )
endif()

set_target_properties(${BIN} PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
