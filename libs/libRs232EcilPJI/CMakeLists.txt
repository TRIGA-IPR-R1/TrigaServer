cmake_minimum_required(VERSION 3.12)

project(
  libRs232EcilPJI
  VERSION 0.0.0
  DESCRIPTION "Communicate with the ECIL PJ-I over RS-232C (Jotta protocol)"
  HOMEPAGE_URL "https://github.com/TRIGA-IPR-R1/libRs232EcilPJI"
  LANGUAGES CXX)

find_package(PkgConfig REQUIRED)

pkg_search_module(SerialPort REQUIRED IMPORTED_TARGET libserialport)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_C_STANDARD_INCLUDE_DIRECTORIES
    ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES})

set(LIBR232ECILPJI_SRC src/libRs232EcilPJI.cpp)

add_library(rs232EcilPJI STATIC ${LIBR232ECILPJI_SRC})
add_library(rs232EcilPJI::rs232EcilPJI ALIAS rs232EcilPJI)

target_include_directories(rs232EcilPJI
                           PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(rs232EcilPJI PUBLIC PkgConfig::SerialPort)

add_executable(testRs232EcilPJI src/test.cpp)
target_link_libraries(testRs232EcilPJI PRIVATE rs232EcilPJI)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/libRs232EcilPJI/)
install(
  TARGETS rs232EcilPJI
  LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
  ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

set(LibRs232EcilPJI_INCLUDE_DIRS
    ${CMAKE_INSTALL_PREFIX}/include/libRs232EcilPJI)
set(LibRs232EcilPJI_LIBRARIES
    ${CMAKE_INSTALL_PREFIX}/lib/librs232EcilPJI.a)
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/resources/LibRs232EcilPJIConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/resources/LibRs232EcilPJIConfig.cmake
  @ONLY)
install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/resources/LibRs232EcilPJIConfig.cmake
  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/LibRs232EcilPJI)
