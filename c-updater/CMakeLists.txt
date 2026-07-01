cmake_minimum_required(VERSION 3.15)
project(labelprinterapp_update LANGUAGES CXX)

option(LABELPRINTERAPP_UPDATE_BUILD_TESTS "Build unit tests" OFF)
option(LABELPRINTERAPP_UPDATE_BUILD_DOWNLOADER
       "Build the full downloader/updater target" OFF)
option(LABELPRINTERAPP_UPDATE_FETCH_HTTPLIB
       "Fetch cpp-httplib when no package is installed" ON)

# Manager-only library for launchers and update application.
add_library(labelprinterapp_update_manager INTERFACE)
target_include_directories(labelprinterapp_update_manager INTERFACE ./include)
if(WIN32)
    target_link_libraries(labelprinterapp_update_manager
        INTERFACE runtimeobject ole32 shell32 uuid)
endif()

if(LABELPRINTERAPP_UPDATE_BUILD_TESTS AND NOT LABELPRINTERAPP_UPDATE_BUILD_DOWNLOADER)
    message(FATAL_ERROR "LABELPRINTERAPP_UPDATE_BUILD_TESTS requires LABELPRINTERAPP_UPDATE_BUILD_DOWNLOADER=ON.")
endif()

if(LABELPRINTERAPP_UPDATE_BUILD_DOWNLOADER)
    # Full updater library with downloader and manager dependencies.
    add_library(labelprinterapp_update INTERFACE)
    target_include_directories(labelprinterapp_update INTERFACE ./include)
    target_link_libraries(labelprinterapp_update INTERFACE labelprinterapp_update_manager)

    find_package(httplib CONFIG QUIET)
    if(NOT TARGET httplib::httplib AND LABELPRINTERAPP_UPDATE_FETCH_HTTPLIB)
        include(FetchContent)
        FetchContent_Declare(
            cpp_httplib
            GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
            GIT_TAG v0.47.0)
        FetchContent_MakeAvailable(cpp_httplib)
    endif()
    if(NOT TARGET httplib::httplib)
        message(FATAL_ERROR "cpp-httplib was not found. Install httplib or configure with LABELPRINTERAPP_UPDATE_FETCH_HTTPLIB=ON.")
    endif()
    target_link_libraries(labelprinterapp_update INTERFACE httplib::httplib)
    target_compile_definitions(labelprinterapp_update INTERFACE CPPHTTPLIB_OPENSSL_SUPPORT)
    target_compile_definitions(labelprinterapp_update INTERFACE UPDATE_WITH_GITHUB)
    target_compile_definitions(labelprinterapp_update
        INTERFACE $<$<CXX_COMPILER_ID:MSVC>:_SILENCE_EXPERIMENTAL_COROUTINE_DEPRECATION_WARNINGS>)
    target_include_directories(labelprinterapp_update
                               INTERFACE third_party/nlohmann+json/include)
    if(WIN32 AND NOT DEFINED OPENSSL_ROOT_DIR)
        foreach(candidate "C:/Program Files/OpenSSL-Win64" "C:/Program Files/OpenSSL" "C:/OpenSSL-Win64" "C:/OpenSSL")
            if(EXISTS "${candidate}/include/openssl/ssl.h")
                set(OPENSSL_ROOT_DIR "${candidate}" CACHE PATH "OpenSSL root directory")
                break()
            endif()
        endforeach()
    endif()
    find_package(OpenSSL REQUIRED)
    target_link_libraries(labelprinterapp_update INTERFACE OpenSSL::SSL OpenSSL::Crypto)

    # minizip on windows for extracting release files
    if(WIN32)
        find_package(ZLIB QUIET)
        set(MZ_ZLIB ON CACHE BOOL "" FORCE)
        set(MZ_DECOMPRESS_ONLY ON CACHE BOOL "" FORCE)
        set(MZ_COMPAT OFF CACHE BOOL "" FORCE)
        set(MZ_BZIP2 OFF CACHE BOOL "" FORCE)
        set(MZ_LZMA OFF CACHE BOOL "" FORCE)
        set(MZ_ZSTD OFF CACHE BOOL "" FORCE)
        if(TARGET ZLIB::ZLIB)
            set(MZ_FETCH_LIBS OFF CACHE BOOL "" FORCE)
            target_link_libraries(labelprinterapp_update INTERFACE ZLIB::ZLIB)
        else()
            set(MZ_FETCH_LIBS ON CACHE BOOL "" FORCE)
        endif()
        set(MZ_LIBCOMP OFF CACHE BOOL "" FORCE)
        set(MZ_PKCRYPT OFF CACHE BOOL "" FORCE)
        set(MZ_WZAES OFF CACHE BOOL "" FORCE)
        set(SKIP_INSTALL_ALL ON CACHE BOOL "" FORCE)
        add_subdirectory(third_party/zlib-ng+minizip-ng)
        target_link_libraries(labelprinterapp_update INTERFACE MINIZIP::minizip)
    endif()

    set_property(TARGET labelprinterapp_update PROPERTY CXX_STANDARD 17)
    set_property(TARGET labelprinterapp_update PROPERTY CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

set_property(TARGET labelprinterapp_update_manager PROPERTY CXX_STANDARD 17)
set_property(TARGET labelprinterapp_update_manager PROPERTY CMAKE_CXX_STANDARD_REQUIRED ON)

if(LABELPRINTERAPP_UPDATE_BUILD_TESTS)
    # Prevent overriding the parent project's compiler/linker settings (win)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    add_subdirectory(third_party/google+googletest EXCLUDE_FROM_ALL)
    include(GoogleTest)
    enable_testing()
    set(TEST_SOURCES test/update_test.cpp test/common_test.cpp
                     test/integration_test.cpp)
    if(WIN32)
        list(APPEND TEST_SOURCES test/win_test.cpp)
    endif()

    add_executable(labelprinterapp_update_test ${TEST_SOURCES})
    target_compile_definitions(labelprinterapp_update_test PRIVATE LABELPRINTERAPP_UPDATE_TEST_BUILD)
    target_link_libraries(labelprinterapp_update_test labelprinterapp_update)
    target_link_libraries(labelprinterapp_update_test GTest::gmock_main)
    target_include_directories(
        labelprinterapp_update_test PRIVATE ${gtest_SOURCE_DIR}/include
                                   ${gtest_SOURCE_DIR})
    # We need to be able to parse JSON in tests.
    target_include_directories(labelprinterapp_update_test
                               INTERFACE third_party/nlohmann+json/include)

    set_property(TARGET labelprinterapp_update_test PROPERTY CXX_STANDARD 17)
    set_property(TARGET labelprinterapp_update_test PROPERTY CMAKE_CXX_STANDARD_REQUIRED
                                                    ON)

    add_custom_command(
        TARGET labelprinterapp_update_test
        POST_BUILD
        COMMAND
            ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:labelprinterapp_update_test>
            $<TARGET_FILE_DIR:labelprinterapp_update_test>
        COMMAND_EXPAND_LISTS)

    add_test(labelprinterapp_update_test labelprinterapp_update_test)
    gtest_discover_tests(labelprinterapp_update_test)
endif()
