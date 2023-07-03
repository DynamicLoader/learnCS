cmake_minimum_required(VERSION 3.5)
project(HRCore LANGUAGES CXX)

# options
option(BUILD_TEST "Build Test" OFF)
option(BUILD_EXAMPLE "Build Examples" ON)
option(BUILD_UI "Build UI (for example only)" OFF)

# Pre-configure check
if(CMAKE_COMPILER_IS_GNUCXX)
    SET(BUILD_MERGE ON)
    SET(MERGE_ARGS -D__MERGE__ -E ./HRCore.h)
    if(CMAKE_HOST_UNIX)
        SET(GREP_CMD "grep -vE \"^$$|^#\"")
    elseif(CMAKE_HOST_WIN32)
        SET(GREP_CMD "findstr /V \"^$$ ^#\"")
    else()
        message("Build Merge: Only support for Windows and UNIX(Linux) with make!")
        SET(BUILD_MERGE OFF)
    endif()
else()
SET(BUILD_MERGE OFF)
    message(WARNING "Not using GNU/GCC, will not build merged header file!")
endif()

# General compiler args
include_directories(.)
set(CMAKE_CXX_STANDARD 11)

# Target: merge
if(BUILD_MERGE)
    add_custom_target(
        merge
        ALL
        COMMAND ${CMAKE_COMMAND} -E cat merge_header > HRCore.hpp
        COMMAND ${CMAKE_CXX_COMPILER} ${MERGE_ARGS} | ${GREP_CMD}  >> HRCore.hpp
        COMMAND ${CMAKE_COMMAND} -E cat merge_footer >> HRCore.hpp
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
        BYPRODUCTS "${PROJECT_SOURCE_DIR}/HRCore.hpp" 
    )
    install(FILES ${PROJECT_SOURCE_DIR}/HRCore.hpp DESTINATION ${CMAKE_INSTALL_PREFIX})
endif()

# Target: test
if(BUILD_TEST)
    SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY test)
    aux_source_directory(./test SRC_TEST)
    foreach(src ${SRC_TEST})
        get_filename_component(file ${src} NAME_WLE)
        add_executable(${file} ${src})
    endforeach()
    install(DIRECTORY ${CMAKE_BINARY_DIR}/test  DESTINATION ${CMAKE_INSTALL_PREFIX})
endif()

# Target: examples
if(BUILD_EXAMPLE)
    SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY examples) 
    aux_source_directory(./examples SRC_EXAMPLES)
    foreach(src ${SRC_EXAMPLES})
        get_filename_component(file ${src} NAME_WLE)
        add_executable(${file} ${src})
    endforeach()
    install(DIRECTORY ${CMAKE_BINARY_DIR}/examples  DESTINATION ${CMAKE_INSTALL_PREFIX})
endif()

# Target: ui
if(BUILD_UI)
    SET(FLTK_SKIP_OPENGL ON)
    SET(FLTK_SKIP_FORMS ON)
    SET(FLTK_SKIP_FLUID ON)
    find_package(
        FLTK 1.3.8
        REQUIRED
    )
    if(FLTK_FOUND)
        message("Found FLTK!")
        message("FLTK_INCLUDE_DIR: " ${FLTK_INCLUDE_DIR})
        message("FLTK_LIBRARIES: " ${FLTK_LIBRARIES})
        SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ui) 
        aux_source_directory(./ui SRC_UI)
        add_executable(ui ${SRC_UI})
        target_include_directories(ui BEFORE 
            PUBLIC ${FLTK_INCLUDE_DIR}
            PUBLIC ${FLTK_INCLUDE_DIR}/../ # May be the parent directory
        )
        target_link_directories(ui 
            PUBLIC ${FLTK_LIBRARIES}
        )
        target_link_libraries(ui fltk_png ${FLTK_LIBRARIES})
        if(WIN32)
            if(MSVC)
                target_link_options(ui PUBLIC "/SUBSYSTEM:WINDOWS")
            elseif(CMAKE_COMPILER_IS_GNUCXX)
                target_link_options(ui PUBLIC "-mwindows")
                target_link_options(ui PUBLIC -static -static-libgcc -static-libstdc++ -lwinpthread -s)
                target_compile_options(ui PUBLIC -O2)
            else()
                message("Not using MSVC/MinGW/GCC, will not choose subsystem.")
            endif()
        endif()
        install(DIRECTORY ${CMAKE_BINARY_DIR}/ui DESTINATION ${CMAKE_INSTALL_PREFIX})
    endif()
endif()

# Target: install
install(FILES ${PROJECT_SOURCE_DIR}/HRCore.h  DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY ${PROJECT_SOURCE_DIR}/inc  DESTINATION ${CMAKE_INSTALL_PREFIX})
