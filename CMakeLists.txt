cmake_minimum_required(VERSION 3.16)

project(cppmain VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)
SET(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#################

# To setup your own build environment, you can create a file named 
# <localSettings.cmake> in the same directory as CMakeLists.txt. This file 
# will be included by CMakeLists.txt and can be used to override the default
# settings. For example, you can use it to set the Qt6_DIR variable to the
# path of your Qt6 installation. The directory you point to must 
# contain a xxxConfig.cmake file.

# Example

# in <localSettings.cmake>

# set(Qt6_DIR C:/Users/Someuser/Qt/6.5.0/mingw_64/lib/cmake/Qt6)
# set(OpenCV_DIR C:/Users/Someuser/opencv4.6/opencv/sources/build)
# <your other settings>

#################

# options for compiling
set(Qt6_DIR "/lib/path-to-qt6" CACHE PATH "Directory that contains Qt6 of selected version")
set(OpenCV_DIR "/lib/path-to-opencv" CACHE PATH "Directory that contains OpenCV")
#set(CMAKE_CXX_FLAGS "" CACHE STRING "Extra C++ compile argument")
option(STATIC_BUILD "Enable static build" OFF)
option(WITH_TESTS "Build Tests" ON)

include(${CMAKE_CURRENT_SOURCE_DIR}/localSettings.cmake OPTIONAL RESULT_VARIABLE INC_RET)

if(${INC_RET} STREQUAL "NOTFOUND")
    message(NOTICE "No local configure found")
endif()

find_package(Qt6 6.4 COMPONENTS Qml Quick QuickControls2 Widgets REQUIRED)
find_package(OpenCV COMPONENTS core imgproc highgui imgcodecs REQUIRED)
find_package(OpenMP REQUIRED)

# Resource File List
SET(RCC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/ui/resource.qrc ${CMAKE_CURRENT_SOURCE_DIR}/res/embed.qrc)
file(GLOB QML_FILES ${CMAKE_CURRENT_SOURCE_DIR}/ui/*.qml )
file(GLOB QML_HANDLER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/ui/handler/*.js)
file(GLOB SRC_FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TNAM/*.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/RNAM/*.cpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/*.h 
    ${CMAKE_CURRENT_SOURCE_DIR}/inc/*.hpp
)

if(MINGW AND WIN32)
    set(CMAKE_RC_COMPILER_INIT windres)
    ENABLE_LANGUAGE(RC)
    SET(CMAKE_RC_COMPILE_OBJECT" -O coff -i -o ")
    list(APPEND SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/res/windres/win.rc)
endif()


foreach(qmlfile in ${QML_FILES})
    get_filename_component(fn ${qmlfile} NAME)
    set_source_files_properties(${qmlfile} PROPERTIES QT_RESOURCE_ALIAS ${fn} )
endforeach()

foreach(qjsfile in ${QML_HANDLER_FILES})
    get_filename_component(fn ${qjsfile} NAME)
    set_source_files_properties(${qjsfile} PROPERTIES QT_RESOURCE_ALIAS handler/${fn})
endforeach()

qt_add_executable(app
    ${SRC_FILES}
    ${RCC_FILES}
)

qt_add_qml_module(app
    URI ui/
    VERSION 1.0
    RESOURCE_PREFIX /
    QML_FILES ${QML_FILES} ${QML_HANDLER_FILES}

)

set_target_properties(app PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
#    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
#    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
#    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE ${STATIC_BUILD}
)

target_include_directories(app
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/inc
    PUBLIC ${OpenCV_INCLUDE_DIRS}
)

set (Qtlibs 
    # PRIVATE Qt6::Widgets
    PRIVATE Qt6::Quick
    PRIVATE Qt6::QuickControls2
    PRIVATE Qt6::Qml)

if(MINGW AND STATIC_BUILD)
    target_link_options(app
        PRIVATE -static-libgcc -static-libstdc++ -static -lgomp
    )
    set(OPENMP_LIBS )
    message("Static Build Enabled")
else()
    set(OPENMP_LIBS OpenMP::OpenMP_CXX)
endif()

target_link_libraries(app
    PRIVATE ${Qtlibs}
    PRIVATE ${OpenCV_LIBS}
    PRIVATE ${OPENMP_LIBS}
)



install(TARGETS app
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

# ================ Test targets ======================
# Ref: https://blog.csdn.net/heihei36/article/details/127068864

macro(SUBDIRLIST result curdir)
    file(GLOB children RELATIVE ${curdir} ${curdir}/*)
    set(dirlist "")
    foreach(child ${children})
        if(IS_DIRECTORY ${curdir}/${child})
            LIST(APPEND dirlist ${child})
        endif()
    endforeach()
    set(${result} ${dirlist})
endmacro()
 

if(WITH_TESTS)
    set(TEST_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/test)
    SUBDIRLIST(SUBDIRS ${TEST_ROOT_DIR})
    
    foreach(CURRENT_FOLDER ${SUBDIRS})
        if(EXISTS ${TEST_ROOT_DIR}/${CURRENT_FOLDER}/CMakeLists.txt)
            add_subdirectory(${TEST_ROOT_DIR}/${CURRENT_FOLDER})
            message("[Test.Configure] Found a test at ${TEST_ROOT_DIR}/${CURRENT_FOLDER}")
        endif()
        
    endforeach()
endif()
