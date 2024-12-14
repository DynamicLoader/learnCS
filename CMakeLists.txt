cmake_minimum_required(VERSION 3.5...3.26)
project(pybindTest)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)

if(WIN32 AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    add_compile_options(-Wa,-mbig-obj)
endif()

set(Python_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/.conda/)
find_package(Python 3.10 COMPONENTS Interpreter Development REQUIRED)
set(PYTHON_EXECUTABLE ${Python_EXECUTABLE})
add_subdirectory(deps/pybind11)


file(GLOB CPP_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/*.cpp
)

# The source should be in C but it does require g++ to compile, so simply use C++ for now
file(GLOB CLIQUE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/glue/*.c
)

set(BINDING_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/bindings)

file(GLOB BINDING_SOURCES 
    ${BINDING_PREFIX}/*.cpp
)

pybind11_add_module(xmldb ${BINDING_PREFIX}/xmldb_binding.cpp)

pybind11_add_module(dbwrapper 
    ${BINDING_PREFIX}/dbWrapper_binding.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/DBWrapper.cpp
)

add_library(entry SHARED ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/Entry.cpp)
add_library(clique SHARED ${CLIQUE_SOURCES})
target_link_libraries(dbwrapper PUBLIC entry clique)
target_link_libraries(xmldb PUBLIC entry)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cpp/inc/
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/cereal/include/
    ${CMAKE_CURRENT_SOURCE_DIR}/src/glue/
)

add_executable(storage_test 
    ${CMAKE_CURRENT_SOURCE_DIR}/test/storage.cpp
)

add_executable(trieTest ${CMAKE_CURRENT_SOURCE_DIR}/test/trieTest.cpp) 

add_custom_target(pybundle
    # this will need to run in the virtual env...
    COMMAND python -m PyInstaller -F --distpath pybundle -p . -i ${CMAKE_CURRENT_SOURCE_DIR}/src/web/public/favicon.ico --add-data ${CMAKE_CURRENT_SOURCE_DIR}/src/web/dist:src/web/dist ../src/py/main.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)