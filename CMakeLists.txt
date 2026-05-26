# cmake needs this line
cmake_minimum_required(VERSION 3.10)

project(obj_tracker_cpp)  # automatically sets var $PROJECT_NAME

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

set(SOURCES  
    src/main.cpp)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # for manually analyzing any of the files using clang-tidy

option(ENABLE_CLANG_TIDY "" OFF)

function(enable_clang_tidy_for target)  # helper fuction for clang tidy
    if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_PATH clang-tidy REQUIRED)
    set_target_properties(${target}
        PROPERTIES
        CXX_CLANG_TIDY "${CLANG_TIDY_PATH}")
    endif()
endfunction()

add_executable(${PROJECT_NAME} ${SOURCES})

enable_clang_tidy_for(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PRIVATE
    ${OpenCV_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(${PROJECT_NAME} ${OpenCV_LIBS})
