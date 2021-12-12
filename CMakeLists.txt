cmake_minimum_required(VERSION 3.20)


add_compile_options(-g -Wall)

project(smap)

file(GLOB PUBLIC_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/include/*.h)
file(GLOB smap_sources ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)


add_executable(smap_e ${smap_sources})


list(FILTER smap_sources EXCLUDE REGEX "main.c")

add_library(smap SHARED ${smap_sources})
add_library(smap_static STATIC ${smap_sources})

target_compile_options(smap PUBLIC -fPIC)
target_compile_options(smap_static PUBLIC -fPIC)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

target_include_directories(
  smap
    PUBLIC
      $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      ${CMAKE_CURRENT_BINARY_DIR}/include
      ${CMAKE_CURRENT_SOURCE_DIR}/include)


target_include_directories(
  smap_static
    PUBLIC
      $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      ${CMAKE_CURRENT_BINARY_DIR}/include
      ${CMAKE_CURRENT_SOURCE_DIR}/include)


target_include_directories(
  smap_e
    PUBLIC
      $<INSTALL_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
      $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
      ${CMAKE_CURRENT_BINARY_DIR}/include
      ${CMAKE_CURRENT_SOURCE_DIR}/include)


include(FetchContent)
FetchContent_Declare(
  VLQ_static
  URL /home/ianertson/workspace/VLQ
  #GIT_REPOSITORY https://github.com/sebbekarlsson/VLQ.git
  )
FetchContent_MakeAvailable(VLQ_static)


FetchContent_Declare(
  json_static
  GIT_REPOSITORY https://github.com/sebbekarlsson/libjson.git
  )
FetchContent_MakeAvailable(json_static)


FetchContent_Declare(
  hashmap_static
  GIT_REPOSITORY https://github.com/sebbekarlsson/hashmap.git
  )

if (NOT TARGET hashmap_static)
FetchContent_MakeAvailable(hashmap_static)
endif()
target_link_libraries(smap PRIVATE m VLQ_static json_static)
target_link_libraries(smap_e PRIVATE m VLQ_static json_static)
target_link_libraries(smap_static PRIVATE m VLQ_static json_static)
