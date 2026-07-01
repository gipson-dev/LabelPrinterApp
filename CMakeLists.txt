cmake_minimum_required(VERSION 3.20)

project(LabelPrinterApp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(LABELPRINTERAPP_FETCH_NLOHMANN "Fetch nlohmann/json when no package is installed" ON)
option(LABELPRINTERAPP_BUILD_APP "Build the Qt desktop application" ON)
option(LABELPRINTERAPP_ENABLE_XLSX "Enable .xlsx import/export through QXlsx" ON)

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
    core/ExcelImporter.cpp
    core/RecordRangeParser.cpp
    core/AppUpdater.cpp
    ui/MainWindow.cpp
    ui/PreviewWidget.cpp
    ui/ElementEditorWidget.cpp
    ui/ExcelTableModel.cpp
    ui/ExcelRecordsWidget.cpp
)

if (LABELPRINTERAPP_BUILD_APP)
    find_package(Qt6 QUIET COMPONENTS Widgets)
    if (NOT Qt6_FOUND)
        message(FATAL_ERROR "Qt 6 Widgets was not found. Install Qt 6 for MSVC and set CMAKE_PREFIX_PATH, for example: -DCMAKE_PREFIX_PATH=C:/Qt/6.7.3/msvc2022_64")
    endif()

    configure_file(
        ${CMAKE_CURRENT_SOURCE_DIR}/core/AppVersion.h.in
        ${CMAKE_CURRENT_BINARY_DIR}/generated/core/AppVersion.h
        @ONLY
    )

    set(LABELPRINTERAPP_UPDATE_BUILD_DOWNLOADER ON CACHE BOOL "" FORCE)
    add_subdirectory(c-updater)

    if (LABELPRINTERAPP_ENABLE_XLSX)
        include(FetchContent)
        FetchContent_Declare(
            qxlsx
            GIT_REPOSITORY https://github.com/QtExcel/QXlsx.git
            GIT_TAG master
        )
        FetchContent_MakeAvailable(qxlsx)
        if (NOT TARGET QXlsx::QXlsx AND NOT TARGET QXlsx)
            add_subdirectory(${qxlsx_SOURCE_DIR}/QXlsx ${qxlsx_BINARY_DIR}/QXlsx)
        endif()
    endif()

    add_executable(LabelPrinterApp WIN32
        main.cpp
        ${CORE_SOURCES}
        ${UI_SOURCES}
    )

    target_compile_definitions(LabelPrinterApp PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
    target_include_directories(LabelPrinterApp PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_BINARY_DIR}/generated
    )
    set_target_properties(LabelPrinterApp PROPERTIES
        AUTOMOC ON
        AUTOUIC ON
        AUTORCC ON
    )
    target_link_libraries(LabelPrinterApp PRIVATE
        Qt6::Widgets
        nlohmann_json::nlohmann_json
        labelprinterapp_update
    )
    if (LABELPRINTERAPP_ENABLE_XLSX)
        target_compile_definitions(LabelPrinterApp PRIVATE LABELPRINTERAPP_ENABLE_XLSX)
        target_include_directories(LabelPrinterApp PRIVATE ${qxlsx_SOURCE_DIR}/QXlsx/header)
        if (TARGET QXlsx::QXlsx)
            target_link_libraries(LabelPrinterApp PRIVATE QXlsx::QXlsx)
        elseif (TARGET QXlsx)
            target_link_libraries(LabelPrinterApp PRIVATE QXlsx)
        endif()
    endif()

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

        # Small, Qt-free helper that applies a downloaded update
        # (via labelprinterapp_update_manager) and relaunches LabelPrinterApp.
        # Statically linked so it ships with no extra DLL dependencies,
        # since it must keep running while its own directory is replaced.
        add_executable(LabelPrinterAppLauncher WIN32 launcher/main.cpp)
        target_compile_definitions(LabelPrinterAppLauncher PRIVATE
            WIN32_LEAN_AND_MEAN
            NOMINMAX
            $<$<CXX_COMPILER_ID:MSVC>:_SILENCE_EXPERIMENTAL_COROUTINE_DEPRECATION_WARNINGS>
        )
        target_link_libraries(LabelPrinterAppLauncher PRIVATE labelprinterapp_update_manager)
        set_property(TARGET LabelPrinterAppLauncher PROPERTY
            MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
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
