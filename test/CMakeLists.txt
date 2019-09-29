cmake_minimum_required(VERSION 3.8)
project(cpp-expression-tests CXX)

set(CMAKE_CXX_STANDARD 20)

if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fconcepts")
endif()

find_package(Catch2 QUIET)

if (NOT TARGET Catch2::Catch2)

    if (NOT EXISTS ${PROJECT_SOURCE_DIR}/catch.hpp)
        message(STATUS "Downloading catch...")

        file(DOWNLOAD https://github.com/catchorg/Catch2/releases/download/v2.9.2/catch.hpp
                      ${PROJECT_SOURCE_DIR}/catch.hpp
                      STATUS CATCH_STATUS)

        list(GET CATCH_STATUS 0 CATCH_RESULT)

        if (${CATCH_RESULT} EQUAL 0)
            message(STATUS "OK")
        else()
            list(GET CATCH_STATUS 1 CATCH_ERROR)
            message(FATAL_ERROR "Error " ${CATCH_ERROR})
        endif()

    endif()
endif()

add_executable(cpp-expression-test test.cpp)

if (TARGET Catch2::Catch2)
    target_link_libraries(cpp-expression-test Catch2::Catch2)
endif()
