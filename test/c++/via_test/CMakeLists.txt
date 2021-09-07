cmake_minimum_required(VERSION 3.5.1)

SET(LIBNAME extio)
project(${LIBNAME})

set(project_path ${CMAKE_CURRENT_SOURCE_DIR}/../../..)

include(${project_path}/cmake/common.cmake)
add_definitions(-std=c++11)


option(USE_SSL "" OFF)
IF(USE_SSL)
    add_definitions(-DSSL_TYPE=1)
    message(STATUS "[SSL_TYPE]: 1")
ELSEIF(USE_GMSSL)
    add_definitions(-DSSL_TYPE=2)
    message(STATUS "[SSL_TYPE]: 2")
ENDIF()

option(USE_ALONE "" OFF)
IF(USE_ALONE)
    add_definitions(-DUSE_ALONE=1)
ENDIF()

option(ASYNC_SERVER "" OFF)
IF(ASYNC_SERVER)
    add_definitions(-DASYNC_SERVER=1)
ENDIF()

option(ASYNC_CLIENT "" OFF)
IF(ASYNC_CLIENT)
    add_definitions(-DASYNC_CLIENT=1)
ENDIF()

# third party
include_directories(${project_path}/third_party/rapidjson/include)

# threads
find_package(Threads REQUIRED)

SET(ADD_LINK_LIB_FLAGS "-Wl,--rpath=$ORIGIN../:$ORIGIN")
SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} ${ADD_LINK_LIB_FLAGS}")


message(STATUS "_REFLECTION: ${_REFLECTION}")
message(STATUS "_GRPC_GRPCPP: ${_GRPC_GRPCPP}")
message(STATUS "_PROTOBUF_LIBPROTOBUF: ${_PROTOBUF_LIBPROTOBUF}")


# IF(USE_GMSSL)
#     ### use gmssl
#     set(THIRD_PARTY_DIR ${project_path}/third_party)
#     include_directories(${THIRD_PARTY_DIR}/gmssl/include)
#     link_directories(${THIRD_PARTY_DIR}/gmssl/lib)
#     link_libraries(tacrypto tassl)
# ENDIF()

link_libraries(${_REFLECTION} ${_GRPC_GRPCPP} ${_PROTOBUF_LIBPROTOBUF})
# libraries
file(GLOB_RECURSE srcs ${project_path}/core/src/*.cc)
add_library(${LIBNAME} SHARED ${srcs})
target_include_directories(${LIBNAME} PUBLIC ${project_path}/core/include ${project_path}/core/pybind11)
target_link_libraries(${LIBNAME} PUBLIC pthread ${_REFLECTION} ${_GRPC_GRPCPP} ${_PROTOBUF_LIBPROTOBUF})

set_target_properties(${LIBNAME} PROPERTIES FOLDER ${LIBNAME}
APPEND_STRING PROPERTY LINK_FLAGS "${ADD_LINK_LIB_FLAGS}")


foreach(_target
#   p0_cs
#   p1_cs
#   p2_cs
    client 
    server
  )
  add_executable(${_target} ${project_path}/test/c++/via_test/${_target}.cc)
  target_link_libraries(${_target} PRIVATE ${LIBNAME})
endforeach()




