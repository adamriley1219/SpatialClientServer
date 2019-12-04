# Builds a SpatialOS worker using the C++ API

# Replace project name below with your own worker name
# !!! This needs to match the artifact_name in spatialos.<name>.worker.json
# !!! because ${PROJECT_NAME} is used below when creating the zip target
project(External)
cmake_minimum_required(VERSION 3.10)


set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(APPLICATION_ROOT "${PROJECT_SOURCE_DIR}/../..")
set(SCHEMA_SOURCE_DIR "${APPLICATION_ROOT}/schema")
set(WORKER_SDK_DIR "${APPLICATION_ROOT}/dependencies")

set(CODE_DIR "${APPLICATION_ROOT}/../Code")
set(ENGINE_DIR "${APPLICATION_ROOT}/../Code/Submodule/Engine/Code")

# Strict warnings.
if(MSVC)
  add_definitions(/W2)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
  add_definitions(-Wall -Wextra -Werror -pedantic)
endif()

add_subdirectory(${WORKER_SDK_DIR} "${CMAKE_CURRENT_BINARY_DIR}/WorkerSdk")
add_subdirectory(${SCHEMA_SOURCE_DIR} "${CMAKE_CURRENT_BINARY_DIR}/Schema")

file(GLOB_RECURSE ENGINE_FILES
    "${ENGINE_DIR}/Engine/*.cpp"
    "${ENGINE_DIR}/Engine/*.hpp")

file(GLOB_RECURSE THIRD_PARTY_FILES
    "${ENGINE_DIR}/ThirdParty/*.cpp" 
    "${ENGINE_DIR}/ThirdParty/*.h")

file(GLOB_RECURSE SHARED_FILES
    "${CODE_DIR}/Shared/*.cpp"  
    "${CODE_DIR}/Shared/*.hpp")

add_library(Engine STATIC ${ENGINE_FILES})
add_library(ThirdParty STATIC ${THIRD_PARTY_FILES})
add_library(Shared STATIC ${SHARED_FILES})

target_link_libraries(Shared Engine)
target_link_libraries(Shared ThirdParty)
target_link_libraries(Engine ThirdParty)

#Great for setting targets of your libs that care included through #pragma comment
target_link_directories(ThirdParty PUBLIC ${ENGINE_DIR})

target_include_directories(Engine SYSTEM PUBLIC "${CODE_DIR}")
target_include_directories(ThirdParty SYSTEM PUBLIC "${CODE_DIR}")
target_include_directories(Shared SYSTEM PUBLIC "${CODE_DIR}")

target_include_directories(Engine SYSTEM PUBLIC "${ENGINE_DIR}")
target_include_directories(ThirdParty SYSTEM PUBLIC "${ENGINE_DIR}")
target_include_directories(Shared SYSTEM PUBLIC "${ENGINE_DIR}")


# Set the default Visual Studio startup project to the worker itself. This only has an effect from
# CMake 3.6 onwards.
set(VS_STARTUP_PROJECT ${PROJECT_NAME})

# The worker binary.
file(GLOB_RECURSE SOURCE_FILES
    "${CODE_DIR}/Game/*.hpp"
    "${CODE_DIR}/Game/*.cpp"
    )

add_executable(${PROJECT_NAME} WIN32 ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} WorkerSdk Schema Engine Shared ThirdParty)


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
