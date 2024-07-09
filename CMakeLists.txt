cmake_minimum_required(VERSION 3.1)

project(quick-cpp)

# 可在外部指定下列选项

## 指定的输出文件名
set(OUTPUT_APPNAME)
# 指定输出目录(默认在out目录,主目录是在编译目录中，需要使用"../"或者"绝对路径")
set (OUTPUT_DIRNAME)

## 额外的C编译选项
set (ADD_C_FLAGS
    "-Wall -g"
    )
## 额外的CXX编译选项
set (ADD_CXX_FLAGS
    "-Wall"
    )

## 指定的交叉工具链
set (CROSS_COMPILE)

## 指定的源文件目录是否加入头文件跟踪
set (INLUCDE_CODE_DIR OFF)


#set(CMAKE_VERBOSE_MAKEFILE ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O0 -fPIC -fpermissive -Wall -Wno-packed-bitfield-compat")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -lstdc++ -D__UAPI_DEF_IF_IFREQ=1")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 ")

set(CXX_STANDARD_REQUIRED ON)

# 添加c/c++文件目录(自动识别其中的.c, .cpp, .cxx)
set(src_dirs
    "./"
    "./core"
    "core/3rd/minilog"
    "core/3rd/inih"
    "core/3rd/os"
    "core/3rd/uart"
    "core/3rd/threadpool/src"
    "core/3rd/jsoncpp"
    "core/3rd/my_string"
)

# 添加c/c++ 头文件目录
set(inc_dirs
    "./"
    "./core"
    "core/3rd/minilog"
    "core/3rd/inih"
    "core/3rd/os"
    "core/3rd/uart"
    "core/3rd/threadpool/src"
    "core/3rd/jsoncpp"
    "core/3rd/my_string"
)

# 添加 库目录
set(lib_dirs
)

# 添加库名词
set(libs_name
    "pthread"
)



###################### 请勿轻易修改 ######################
message(STATUS "-------------------------")
# 添加LD选项
set(ALL_LIB_NAME "")
if (ADD_LDFLAGS)
    message(STATUS "[+] ld-cfgs : ${ADD_LDFLAGS}")
    set(ALL_LIB_NAME ${ADD_LDFLAGS})
    message(STATUS "-------------------------")
endif()

# 添加C编译选项
if (ADD_C_FLAGS)
foreach(c_flags ${ADD_C_FLAGS})
    message(STATUS "[+]  c-cfgs : ${c_flags}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${c_flags}")
endforeach()
    message(STATUS "-------------------------")
endif()
# 添加CXX编译选项
if (ADD_CXX_FLAGS)
foreach(cxx_flags ${ADD_CXX_FLAGS})
    message(STATUS "[+] cx-cfgs : ${cxx_flags}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${cxx_flags}")
endforeach()
    message(STATUS "-------------------------")
endif()

# 遍历指定的源目录
if (src_dirs)
set(ALL_SRC_FILES "")
foreach(sdir ${src_dirs})
    #set(ROOT_PROJ ${CMAKE_CURRENT_SOURCE_DIR})
    #exec_program("cd ${ROOT_PROJ}/; find ${sdir}/ -maxdepth 1 -mindepth 1 -type d 2>/dev/null | grep -v '.git' | xargs"  WORKING_DIRECTORY ${sdir} OUTPUT_VARIABLE DIR_STRING)
    set(DIR_STRING ${sdir})
    aux_source_directory(${sdir}/. ALL_SRC_FILES)
    message(STATUS "[+] src-dir ${DIR_STRING}")
    # 如果允许INLUCDE_CODE_DIR，那么源文件目录也加入头文件跟踪
    if (INLUCDE_CODE_DIR)
        include_directories( ${DIR_STRING})
        message(STATUS "[+] inc-dir ${DIR_STRING}")
    endif()
endforeach()
#message(STATUS "[ ] is ${ALL_SRC_FILES}")
message(STATUS "-------------------------")
endif()

# 遍历指定的头目录
if (inc_dirs)
foreach(sdir ${inc_dirs})
    set(DIR_STRING ${sdir})
    include_directories( ${DIR_STRING})
    message(STATUS "[+] inc-dir ${DIR_STRING}")
endforeach()
message(STATUS "-------------------------")
endif()

# 遍历指定的库目录
if(lib_dirs)
foreach(sdir ${lib_dirs})
    set(DIR_STRING ${sdir})
    link_directories( ${DIR_STRING})
    message(STATUS "[+] lib-dir ${DIR_STRING}")
endforeach()
message(STATUS "-------------------------")
endif()

# 添加指定的库名
if(libs_name)
foreach(sdir ${libs_name})
    set(DIR_STRING "-l${sdir}")
    list(APPEND ALL_LIB_NAME ${DIR_STRING})
    message(STATUS "[+] lib     ${sdir}")
endforeach()
#message(STATUS "[ ] lib     ${ALL_LIB_NAME}")
message(STATUS "-------------------------")
endif()

# 指定工具链
if (CROSS_COMPILE)
    set(CMAKE_C_COMPILER   "${CROSS_COMPILE}gcc")
    set(CMAKE_CXX_COMPILER "${CROSS_COMPILE}g++")
    message(STATUS "using CMAKE_C_COMPILER: ${CMAKE_C_COMPILER}")
    message(STATUS "using CMAKE_CXX_COMPILER: ${CMAKE_CXX_COMPILER}")
    message(STATUS "-------------------------")
endif()


add_executable(program ${ALL_SRC_FILES})

###   if(NOT CMAKE_BUILD_TYPE)
###       set(CMAKE_BUILD_TYPE MinSizeRel)
###   endif()
###
###   message(STATUS "CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")


#string(TIMESTAMP COMPILE_TIME %Y%m%d_%H%M%S)
#string(TIMESTAMP COMPILE_TIME %Y%m%d_%H)

# 指定输出文件名
if (NOT OUTPUT_APPNAME)
    set(OUTPUT_APPNAME "quick-cpp")
endif()
# 指定输出目录(默认在out目录)
if (NOT OUTPUT_DIRNAME)
    SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/out)
else()
    SET(EXECUTABLE_OUTPUT_PATH ${OUTPUT_DIRNAME})
endif()

set(target_name "${OUTPUT_APPNAME}")
SET_TARGET_PROPERTIES (program PROPERTIES OUTPUT_NAME "${target_name}")
target_link_libraries(program ${ALL_LIB_NAME})

#  message(STATUS "CMAKE_CURRENT_SOURCE_DIR: ${TOP}")
#  ##可以使用配置文件进行加载不同的编译链
#  ## 例如:
#  ##      toolchain.make
#  ##      文件内容：
#  ##           set(CMAKE_C_COMPILER gcc)
#  ##           set(CMAKE_CXX_COMPILER g++)
#  ##
#  include(${TOP}/tools/toolchain.cmake)
