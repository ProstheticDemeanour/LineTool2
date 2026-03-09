cmake_minimum_required(VERSION 3.21)
project(LineTool VERSION 2.0.0 LANGUAGES CXX)

# ── C++ standard ──────────────────────────────────────────────────────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ── Qt auto-tools ─────────────────────────────────────────────────────────────
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC OFF)   # We use code-only UI

# ── Find Qt6 (fallback Qt5) ───────────────────────────────────────────────────
find_package(Qt6 QUIET COMPONENTS Core Widgets Sql)
if (Qt6_FOUND)
    set(QT_VERSION_MAJOR 6)
    set(QT Qt6)
    message(STATUS "Using Qt6")
else()
    find_package(Qt5 REQUIRED COMPONENTS Core Widgets Sql)
    set(QT_VERSION_MAJOR 5)
    set(QT Qt5)
    message(STATUS "Using Qt5")
endif()

# ── Find SQLite3 ──────────────────────────────────────────────────────────────
# On macOS/Linux: use system sqlite3; on Windows prefer Qt's bundled one
find_package(SQLite3 QUIET)
if (NOT SQLite3_FOUND)
    # Fallback: assume sqlite3 headers/libs are alongside Qt (common on Windows)
    message(STATUS "SQLite3 not found via CMake — relying on Qt SQL SQLite driver")
endif()

# ── Platform-specific ─────────────────────────────────────────────────────────
if (APPLE)
    set(APP_ICON_MACOSX "${CMAKE_SOURCE_DIR}/resources/AppIcon.icns")
    set(MACOSX_BUNDLE_ICON_FILE AppIcon.icns)
    set_source_files_properties(${APP_ICON_MACOSX} PROPERTIES
        MACOSX_PACKAGE_LOCATION "Resources")
endif()

if (WIN32)
    # set(WIN_RC "${CMAKE_SOURCE_DIR}/resources/winres.rc")
endif()

# ── Sources ───────────────────────────────────────────────────────────────────
set(CORE_SOURCES
    src/core/TransmissionLine.cpp
    src/core/Database.cpp
)

set(MODEL_SOURCES
    src/models/ConductorTableModel.cpp
)

set(WIDGET_SOURCES
    src/widgets/TowerGeometryWidget.cpp
)

set(TAB_SOURCES
    src/tabs/LineParametersTab.cpp
    src/tabs/ConductorDatabaseTab.cpp
)

set(APP_SOURCES
    src/MainWindow.cpp
    src/main.cpp
    ${CORE_SOURCES}
    ${MODEL_SOURCES}
    ${WIDGET_SOURCES}
    ${TAB_SOURCES}
)

# Add platform resources
if (APPLE)
    list(APPEND APP_SOURCES ${APP_ICON_MACOSX})
endif()
if (WIN32)
    list(APPEND APP_SOURCES ${WIN_RC})
endif()

# ── Executable ────────────────────────────────────────────────────────────────
if (APPLE)
    add_executable(LineTool MACOSX_BUNDLE ${APP_SOURCES})
    set_target_properties(LineTool PROPERTIES
        MACOSX_BUNDLE_GUI_IDENTIFIER  "com.linetool.app"
        MACOSX_BUNDLE_BUNDLE_VERSION  "${PROJECT_VERSION}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${PROJECT_VERSION}"
    )
elseif (WIN32)
    add_executable(LineTool WIN32 ${APP_SOURCES})
else()
    add_executable(LineTool ${APP_SOURCES})
endif()

# ── Include dirs ──────────────────────────────────────────────────────────────
target_include_directories(LineTool PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/tabs
    ${CMAKE_SOURCE_DIR}/src/models
    ${CMAKE_SOURCE_DIR}/src/widgets
)

# ── Link libraries ────────────────────────────────────────────────────────────
target_link_libraries(LineTool PRIVATE
    ${QT}::Core
    ${QT}::Widgets
    ${QT}::Sql
)

if (SQLite3_FOUND)
    target_link_libraries(LineTool PRIVATE SQLite::SQLite3)
endif()

# ── Compiler warnings ─────────────────────────────────────────────────────────
if (MSVC)
    target_compile_options(LineTool PRIVATE /W4)
else()
    target_compile_options(LineTool PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ── Install (optional) ────────────────────────────────────────────────────────
include(GNUInstallDirs)
install(TARGETS LineTool
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# ── macOS: run macdeployqt after build ────────────────────────────────────────
if (APPLE)
    find_program(MACDEPLOYQT macdeployqt
        HINTS "${Qt6_DIR}/../../../bin" "${Qt5_DIR}/../../../bin")
    if (MACDEPLOYQT)
        add_custom_command(TARGET LineTool POST_BUILD
            COMMAND ${MACDEPLOYQT} "$<TARGET_BUNDLE_DIR:LineTool>"
            COMMENT "Running macdeployqt...")
    endif()
endif()

# ── Windows: run windeployqt after build ─────────────────────────────────────
if (WIN32)
    find_program(WINDEPLOYQT windeployqt
        HINTS "${Qt6_DIR}/../../../bin" "${Qt5_DIR}/../../../bin")
    if (WINDEPLOYQT)
        add_custom_command(TARGET LineTool POST_BUILD
            COMMAND ${WINDEPLOYQT}
                --no-translations
                "$<TARGET_FILE:LineTool>"
            COMMENT "Running windeployqt...")
    endif()
endif()
