# Builds a SpatialOS worker using the C++ API

# Replace project name below with your own worker name
# !!! This needs to match the artifact_name in spatialos.<name>.worker.json
# !!! because ${PROJECT_NAME} is used below when creating the zip target
project(Managed)
cmake_minimum_required(VERSION 3.10)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(APPLICATION_ROOT "${PROJECT_SOURCE_DIR}/../..")
set(SCHEMA_SOURCE_DIR "${APPLICATION_ROOT}/schema")
set(WORKER_SDK_DIR "${APPLICATION_ROOT}/dependencies")

set(CODE_DIR "${APPLICATION_ROOT}/../Code")
set(ENGINE_DIR "${APPLICATION_ROOT}/../Code/Submodule/Engine/Code")

# Strict warnings.
if(MSVC)
  add_definitions(/W3)
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -lm")
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
#  add_definitions(-Wall -Wextra -Werror -pedantic)
endif()

add_subdirectory(${WORKER_SDK_DIR} "${CMAKE_CURRENT_BINARY_DIR}/WorkerSdk")
add_subdirectory(${SCHEMA_SOURCE_DIR} "${CMAKE_CURRENT_BINARY_DIR}/Schema")


file(GLOB_RECURSE CODE_FILES
    "${ENGINE_DIR}/Engine/Physics/*.cpp"
    "${ENGINE_DIR}/Engine/Math/*.cpp"
    "${ENGINE_DIR}/Engine/Core/*.cpp"
    "${ENGINE_DIR}/ThirdParty/TinyXML2/*.cpp" 
    "${CODE_DIR}/Shared/*.cpp"

    "${ENGINE_DIR}/Engine/Physics/*.hpp"
    "${ENGINE_DIR}/Engine/Math/*.hpp"
    "${ENGINE_DIR}/Engine/Core/*.hpp"
    "${ENGINE_DIR}/ThirdParty/TinyXML2/*.h"   
    "${CODE_DIR}/Shared/*.hpp")

add_library(Code STATIC ${CODE_FILES})
target_include_directories(Code SYSTEM PUBLIC "${CODE_DIR}")
target_include_directories(Code SYSTEM PUBLIC "${ENGINE_DIR}")

# Set the default Visual Studio startup project to the worker itself. This only has an effect from
# CMake 3.6 onwards.
set(VS_STARTUP_PROJECT ${PROJECT_NAME})

# The worker binary.
file(GLOB_RECURSE SOURCE_FILES
    "${CODE_DIR}/Server/*.cc"
    "${CODE_DIR}/Server/*.hpp"
    "${CODE_DIR}/Server/*.cpp"
    )


add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} WorkerSdk Schema Code)

# Set artifact subdirectories.
# WORKER_ASSEMBLY_DIR should not be changed so that spatial local launch
# and spatial upload can find the worker assemblies
set(WORKER_ASSEMBLY_DIR "${APPLICATION_ROOT}/build/assembly/worker")

# The worker in a zip.
add_custom_target(
  ${PROJECT_NAME}Zip ALL
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  COMMAND ${CMAKE_COMMAND} -E make_directory "${WORKER_ASSEMBLY_DIR}"
  COMMAND spatial file zip -b "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
          -o "${WORKER_ASSEMBLY_DIR}/${PROJECT_NAME}"
          --worker_platform=current
          "$<TARGET_FILE_NAME:${PROJECT_NAME}>"
  DEPENDS ${PROJECT_NAME})
