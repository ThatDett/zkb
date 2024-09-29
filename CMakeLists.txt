cmake_minimum_required(VERSION 3.5)

project(zkb)

set(tool
    src/tooling/
)

set(FILE_SOURCES 
    main.cpp 
    src/Application.cpp
    ${tool}CommandHandler.cpp
    ${tool}Directory.cpp
    src/Utils.cpp
    src/Helper.cpp
)

set(CMAKE_CXX_STANDARD 20)
set(DEBUG_BUILD 1)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Define optimization flags based on build type
if(${DEBUG_BUILD} EQUAL 1)
    add_compile_options(-O0 -g)
else(${DEBUG_BUILD} EQUAL 0)
    add_compile_options(-O3)
endif()


configure_file (src/headers/other/CMakeVariables.h.in ${CMAKE_CURRENT_SOURCE_DIR}/src/headers/other/CMakeVariables.h @ONLY)
add_executable(${PROJECT_NAME} ${FILE_SOURCES})
add_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
#add_subdirectory(external/glfw)

target_include_directories(${PROJECT_NAME} 
    PRIVATE src/headers
    PRIVATE src/headers/tooling
)   

#target_link_directories(${PROJECT_NAME})

#target_link_libraries(${PROJECT_NAME} glfw)
