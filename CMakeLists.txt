cmake_minimum_required(VERSION 3.5)

project(zkb)

set(FILE_SOURCES 
    main.cpp 
    src/CommandHandler.cpp
    src/DirectoryHandler.cpp
    src/Application.cpp
)

set(CMAKE_CXX_STANDARD 20)
set(DEBUG_BUILD 1)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Define optimization flags based on build type
if(${DEBUG_BUILD} EQUAL 1)
    add_compile_options(-O0)
    add_compile_options(-g)
else(${DEBUG_BUILD} EQUAL 0)
    add_compile_options(-O2)
endif()

configure_file (src/headers/CMakeVariables.h.in ${CMAKE_CURRENT_SOURCE_DIR}/src/headers/CMakeVariables.h @ONLY)
add_executable(${PROJECT_NAME} ${FILE_SOURCES})
target_compile_options(${PROJECT_NAME} PRIVATE -Wall)

#add_subdirectory(external/glfw)

target_include_directories(${PROJECT_NAME} 
    PRIVATE src/headers
)   

#target_link_directories(${PROJECT_NAME})

#target_link_libraries(${PROJECT_NAME} glfw)
