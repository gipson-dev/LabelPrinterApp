cmake_minimum_required(VERSION 3.20)

project(LabelPrinterApp LANGUAGES CXX)

set(LABEL_PRINTER_APP_SOURCES
    src/main.cpp
    src/ZebraPrinter.cpp
    src/ui/MainWindow.cpp
    src/ui/PreviewWidget.cpp
)

add_executable(LabelPrinterApp WIN32 ${LABEL_PRINTER_APP_SOURCES})

target_compile_features(LabelPrinterApp PRIVATE cxx_std_17)
target_compile_definitions(LabelPrinterApp PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
target_include_directories(LabelPrinterApp PRIVATE include)

if (WIN32)
    target_link_libraries(LabelPrinterApp PRIVATE Gdi32 User32 Winspool)
else()
    message(FATAL_ERROR "LabelPrinterApp requires Windows WinSpool APIs.")
endif()

include(CTest)

if (BUILD_TESTING)
    add_executable(LabelPrinterAppTests
        tests/model_zpl_tests.cpp
    )

    target_compile_features(LabelPrinterAppTests PRIVATE cxx_std_17)
    target_include_directories(LabelPrinterAppTests PRIVATE include)

    add_test(NAME LabelPrinterAppTests COMMAND LabelPrinterAppTests)
endif()
