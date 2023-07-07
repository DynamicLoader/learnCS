
cmake_minimum_required(VERSION 3.16)

project(cppmain.testRNAM VERSION 0.1 LANGUAGES CXX)

set(buildGUI True)

file(GLOB TEST_SRC_FILES 
        ../../src/RNAM/*.cpp
        ../../src/*.cpp
        *.cpp
    )

list(REMOVE_ITEM TEST_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/../../src/main.cpp)
list(REMOVE_ITEM TEST_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/guiRnam.cpp)
list(REMOVE_ITEM TEST_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/RNAM_cli.cpp)

list(APPEND TEST_SRC_FILES 
    ${CMAKE_CURRENT_SOURCE_DIR}/../../inc/WrapperRNAM.hpp 
    ${CMAKE_CURRENT_SOURCE_DIR}/../../inc/QImageMemory.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../inc/RNAMWorker.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../inc/QUtils.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../inc/QmlExport.hpp
    )

if (buildGUI)
    qt_add_executable(test_RNAM
    guiRnam.cpp
    ${TEST_SRC_FILES}
)
    message("Building GUI version of RNAM")
else()
    add_executable(test_RNAM
    RNAM_cli.cpp
    ${TEST_SRC_FILES})
    message("Building CLI version of RNAM")
endif()

qt_add_qml_module(test_RNAM
    URI /
    VERSION 1.0
    RESOURCE_PREFIX /
    QML_FILES main.qml junkTimer.js

)

target_include_directories(test_RNAM PRIVATE ../../inc)

set (Qtlibs 
    PRIVATE Qt6::Widgets
    PRIVATE Qt6::Quick
    PRIVATE Qt6::QuickControls2
    PRIVATE Qt6::Qml)

target_link_libraries(test_RNAM
    PRIVATE ${OpenCV_LIBS}
    PRIVATE ${Qtlibs}
)
