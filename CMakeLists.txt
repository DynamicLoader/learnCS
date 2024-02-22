cmake_minimum_required(VERSION 3.16)
project(SimpleCompiler )
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_INTPR "Enable ASMJIT for interpreter mode" FALSE)
option(BUILD_BZIP2 "Build Bzip2 atomatically" FALSE)
option(WIN32 FALSE)

if(NOT CMAKE_RC_COMPILER) 
    set(CMAKE_RC_COMPILER "x86_64-w64-mingw32-windres")
endif()

set(GEN_PATH ${CMAKE_CURRENT_SOURCE_DIR}/generated)
set(LANG_PATH ${CMAKE_CURRENT_SOURCE_DIR}/lang)

# execute_process(
#     COMMAND 
#         sh -c "${CMAKE_C_COMPILER} -print-prog-name=ld | tr -d '\n'"
#     OUTPUT_VARIABLE LD
# )

# execute_process(
#     COMMAND 
#         sh -c "${CMAKE_C_COMPILER} -print-prog-name=objcopy | tr -d '\n'"
#     OUTPUT_VARIABLE OBJCOPY
# )

set(LD ${CMAKE_LINKER})
set(OBJCOPY ${CMAKE_OBJCOPY})

message("Found: LD=" ${LD} ", OBJCOPY=" ${OBJCOPY})

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

if(${BUILD_BZIP2})
    add_custom_target(Bzip2
    COMMAND rm ${CMAKE_CURRENT_SOURCE_DIR}/deps/bzip2/*.o ${CMAKE_CURRENT_SOURCE_DIR}/deps/bzip2/*.a || true
    COMMAND make bzip2 CC=${CMAKE_C_COMPILER} AR=${CMAKE_AR}
        BYPRODUCTS ${CMAKE_CURRENT_SOURCE_DIR}/deps/bzip2/*.o ${CMAKE_CURRENT_SOURCE_DIR}/deps/bzip2/*.a ${CMAKE_CURRENT_SOURCE_DIR}/deps/bzip2/*.exe
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/deps/bzip2
    )
    set(BZIP2_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/deps/bzip2)
else()
    find_package(BZip2 REQUIRED)
endif()

if( ${ENABLE_INTPR})

set(ASMJIT_STATIC TRUE)
set(ASMJIT_NO_AARCH64 TRUE)
add_subdirectory(
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/asmjit
)
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/asmjit/src
    ${CMAKE_CURRENT_SOURCE_DIR}/deps/cereal/include
)
endif()


file(GLOB SRC_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/interpreter/*.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/preproc/*.cpp
)

include_directories(
    ${BZIP2_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/inc
    ${CMAKE_CURRENT_SOURCE_DIR}/src/interpreter
    ${CMAKE_CURRENT_SOURCE_DIR}/src/preproc
    ${GEN_PATH} 
)

# standalone interpreter
if(${ENABLE_INTPR})

file(GLOB STDINTPR_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/interpreter/*.cpp)
add_executable(stdintpr 
    ${STDINTPR_SRCS}
)

target_compile_definitions(stdintpr
    PUBLIC INTPR_STANDALONE=1
    PUBLIC ENABLE_INTPR=1 ASMJIT_STATIC ASMJIT_NO_AARCH64
)

target_link_options(stdintpr
    PUBLIC -s
    PUBLIC -static-libgcc
    PUBLIC -static-libstdc++
)

target_link_libraries(stdintpr PUBLIC asmjit)

if(WIN32)
    set(STDINTPR_EXE "stdintpr.exe")

    add_custom_command(OUTPUT ${GEN_PATH}/win.o
        COMMAND
            ${CMAKE_RC_COMPILER} -i ${PROJECT_SOURCE_DIR}/builtin/win.rc -o win.o
        DEPENDS ${PROJECT_SOURCE_DIR}/builtin/win.rc
        WORKING_DIRECTORY ${GEN_PATH}
    )   
else()
    set(STDINTPR_EXE "stdintpr")
endif()

add_custom_command(OUTPUT ${GEN_PATH}/stdintpr.res
    COMMAND 
        cd ${PROJECT_BINARY_DIR} && tar -cjf ${GEN_PATH}/stdintpr.tar.bz2 ${STDINTPR_EXE}
    COMMAND 
        ${LD} -r -b binary stdintpr.tar.bz2 -o stdintpr.res
    COMMAND
        ${OBJCOPY} --rename-section .data=.resource,alloc,contents,readonly,data stdintpr.res
    # DEPENDS ${PROJECT_BINARY_DIR}/${STDINTPR_EXE} stdintpr
    BYPRODUCTS ${GEN_PATH}/stdintpr.tar.bz2
    WORKING_DIRECTORY ${GEN_PATH}
)

endif()

# scanner
add_custom_command(OUTPUT ${GEN_PATH}/scanner.cpp
    COMMAND 
        ${FLEX_EXECUTABLE} -+ -o ${GEN_PATH}/scanner.cpp ${LANG_PATH}/scanner.l
    COMMAND 
        cp ${FLEX_INCLUDE_DIRS}/FlexLexer.h ${GEN_PATH}/
    DEPENDS ${LANG_PATH}/scanner.l
    BYPRODUCTS ${GEN_PATH}/FlexLexer.h
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)

# parser
add_custom_command(OUTPUT ${GEN_PATH}/parser.cpp
    COMMAND
        ${BISON_EXECUTABLE} -v ${LANG_PATH}/parser.y -o ${GEN_PATH}/parser.cpp
    COMMAND # conflict info
        echo Dumping conflicts from bison output... & cat ${GEN_PATH}/parser.output | grep -B 5 -A 2 -n '\\[' || true 
    DEPENDS ${LANG_PATH}/parser.y
    BYPRODUCTS ${GEN_PATH}/location.hh ${GEN_PATH}/parser.hpp ${GEN_PATH}/parser.output
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)


# preproc parser
add_custom_command(OUTPUT ${GEN_PATH}/preproc.cpp
    COMMAND
        ${BISON_EXECUTABLE} -v ${LANG_PATH}/preproc.y -o ${GEN_PATH}/preproc.cpp

    COMMAND # conflict info
        echo Dumping conflicts from bison output... & cat ${GEN_PATH}/preproc.output | grep -B 5 -A 2 -n '\\[' || true 
    DEPENDS ${LANG_PATH}/preproc.y
    BYPRODUCTS ${GEN_PATH}/preproc.hpp ${GEN_PATH}/preproc.output
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)

# pre-scanner
add_custom_command(OUTPUT ${GEN_PATH}/prescanner.cpp
    COMMAND 
        ${FLEX_EXECUTABLE} -+ -o ${GEN_PATH}/prescanner.cpp ${LANG_PATH}/preproc.l
    DEPENDS ${LANG_PATH}/preproc.l
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)

# builtin resource: target lib
file(GLOB_RECURSE BUILTIN_TARGET_FILES ${PROJECT_SOURCE_DIR}/builtin/target/*)

# message(${BUILTIN_TARGET_FILES})

add_custom_command(OUTPUT ${GEN_PATH}/target.res
    COMMAND 
        cd ${PROJECT_SOURCE_DIR}/builtin/target/ && tar -cjf ${GEN_PATH}/target.tar.bz2 *
    COMMAND 
        ${LD} -r -b binary target.tar.bz2 -o target.res
    COMMAND
        ${OBJCOPY} --rename-section .data=.resource,alloc,contents,readonly,data target.res
    DEPENDS ${BUILTIN_TARGET_FILES}
    BYPRODUCTS ${GEN_PATH}/target.tar.bz2
    WORKING_DIRECTORY ${GEN_PATH}
)

# builtin resource: other
file(GLOB BUILTIN_OTHER_FILES ${PROJECT_SOURCE_DIR}/builtin/other/*)

add_custom_command(OUTPUT ${GEN_PATH}/other.res
    COMMAND 
        ${LD} -r -b binary * -o ${GEN_PATH}/other.res
    COMMAND
        ${OBJCOPY} --rename-section .data=.resource,alloc,contents,readonly,data ${GEN_PATH}/other.res
    DEPENDS ${BUILTIN_OTHER_FILES}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/builtin/other/
)

add_executable(splc
    
    ${SRC_FILES}

    # Single list with generated file
    ${GEN_PATH}/prescanner.cpp
    ${GEN_PATH}/preproc.cpp
    ${GEN_PATH}/parser.cpp
    ${GEN_PATH}/scanner.cpp
    

    # used for compiling resource file
    ${GEN_PATH}/target.res
    ${GEN_PATH}/other.res
)

target_link_options(splc
    PUBLIC -static-libgcc
    PUBLIC -static-libstdc++
)

target_link_libraries(splc PUBLIC
    -lm 
    -lbz2
    ${GEN_PATH}/target.res
    ${GEN_PATH}/other.res
)

if(WIN32)
    target_sources(splc 
        PUBLIC ${GEN_PATH}/win.o
    )
    target_link_libraries(splc PUBLIC ${GEN_PATH}/win.o)
endif()

if(${ENABLE_INTPR})
    # add_dependencies(splc stdintpr)
    target_compile_definitions(splc 
        PUBLIC ENABLE_INTPR=1 ASMJIT_STATIC ASMJIT_NO_AARCH64
    )
    target_sources(splc 
        PUBLIC ${GEN_PATH}/stdintpr.res
    )   
    target_link_libraries(splc 
        PUBLIC asmjit
        PUBLIC ${GEN_PATH}/stdintpr.res
    )
    
endif()

if(${BUILD_BZIP2})
    add_dependencies(splc Bzip2)
    target_link_directories(splc PUBLIC ${PROJECT_SOURCE_DIR}/deps/bzip2)
endif()



