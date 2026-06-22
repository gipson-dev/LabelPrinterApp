cmake_minimum_required(VERSION 3.20)

project(LabelPrinterApp VERSION 5.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(LABELPRINTERAPP_FETCH_NLOHMANN "Fetch nlohmann/json when no package is installed" ON)
option(LABELPRINTERAPP_BUILD_APP "Build the Qt desktop application" ON)

find_package(nlohmann_json CONFIG QUIET)
if (NOT nlohmann_json_FOUND AND LABELPRINTERAPP_FETCH_NLOHMANN)
    include(FetchContent)
    FetchContent_Declare(
        nlohmann_json
        URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_MakeAvailable(nlohmann_json)
endif()

if (NOT TARGET nlohmann_json::nlohmann_json)
    message(FATAL_ERROR "nlohmann/json was not found. Install nlohmann_json or configure with LABELPRINTERAPP_FETCH_NLOHMANN=ON.")
endif()

set(CORE_SOURCES
    core/ZplGenerator.cpp
    core/ZebraPrinter.cpp
    core/TemplateStorage.cpp
    core/CsvImporter.cpp
    core/VariableResolver.cpp
)

set(UI_SOURCES
    ui/MainWindow.cpp
    ui/PreviewWidget.cpp
    ui/ElementEditorWidget.cpp
)

if (LABELPRINTERAPP_BUILD_APP)
    find_package(Qt6 QUIET COMPONENTS Widgets)
    if (NOT Qt6_FOUND)
        message(FATAL_ERROR "Qt 6 Widgets was not found. Install Qt 6 for MSVC and set CMAKE_PREFIX_PATH, for example: -DCMAKE_PREFIX_PATH=C:/Qt/6.7.3/msvc2022_64")
    endif()

    add_executable(LabelPrinterApp WIN32
        main.cpp
        ${CORE_SOURCES}
        ${UI_SOURCES}
    )

    target_compile_definitions(LabelPrinterApp PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
    target_include_directories(LabelPrinterApp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    set_target_properties(LabelPrinterApp PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
    )
    target_link_libraries(LabelPrinterApp PRIVATE
        Qt6::Widgets
        nlohmann_json::nlohmann_json
    )

    add_custom_command(TARGET LabelPrinterApp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/templates
            $<TARGET_FILE_DIR:LabelPrinterApp>/templates
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/examples
            $<TARGET_FILE_DIR:LabelPrinterApp>/examples
    )

    if (WIN32)
        target_link_libraries(LabelPrinterApp PRIVATE Winspool)
        get_filename_component(QT_BIN_DIR "${Qt6_DIR}/../../../bin" ABSOLUTE)
        find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${QT_BIN_DIR}")
        if (WINDEPLOYQT_EXECUTABLE)
            add_custom_command(TARGET LabelPrinterApp POST_BUILD
                COMMAND "${WINDEPLOYQT_EXECUTABLE}" --no-translations --no-system-d3d-compiler --no-opengl-sw $<TARGET_FILE:LabelPrinterApp>
                COMMENT "Deploying Qt runtime files"
            )
        endif()
    else()
        message(FATAL_ERROR "LabelPrinterApp currently targets Windows RAW printer APIs.")
    endif()
endif()

include(CTest)
if (BUILD_TESTING)
    add_executable(LabelPrinterAppCoreTests
        tests/model_zpl_tests.cpp
        ${CORE_SOURCES}
    )
    target_compile_definitions(LabelPrinterAppCoreTests PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
    target_include_directories(LabelPrinterAppCoreTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
    target_link_libraries(LabelPrinterAppCoreTests PRIVATE nlohmann_json::nlohmann_json Winspool)
    add_test(NAME LabelPrinterAppCoreTests COMMAND LabelPrinterAppCoreTests)
endif()
