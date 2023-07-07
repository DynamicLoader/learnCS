cmake_minimum_required(VERSION 3.16)

project(cppmain.testTNAM VERSION 0.1 LANGUAGES CXX)

file(GLOB TEST_SRC_FILES ./*.cpp)
file(GLOB TEST_QML_FILES ./*.qml )
list(APPEND TEST_SRC_FILES 
    ../../inc/WrapperTNAM.hpp 
    ../../inc/QImageMemory.hpp 
    ../../inc/QUtils.hpp 
    ../../src/TNAM.cpp
    ../../inc/QmlExport.hpp
)

foreach(qmlfile in ${TEST_QML_FILES})
    get_filename_component(fn ${qmlfile} NAME)
    set_source_files_properties(${qmlfile} PROPERTIES QT_RESOURCE_ALIAS ${fn} )
endforeach()


qt_add_executable(test_TNAM
    ${TEST_SRC_FILES}
    ${RCC_FILES}
)

target_include_directories(test_TNAM
    PUBLIC ../../inc
    PUBLIC ${OpenCV_INCLUDE_DIRS} # inherited from main
)

qt_add_qml_module(test_TNAM
    URI ui/
    VERSION 1.0
    RESOURCE_PREFIX /
    QML_FILES ${TEST_QML_FILES}
)

target_compile_options(test_TNAM
    PRIVATE ${OpenMP_CXX_FLAGS})

target_link_libraries(test_TNAM
    PRIVATE Qt6::Widgets
    PRIVATE Qt6::Quick
    PRIVATE Qt6::QuickControls2
    PRIVATE Qt6::Qml
    PRIVATE ${OpenCV_LIBS}
    PRIVATE OpenMP::OpenMP_CXX

)