cmake_minimum_required(VERSION 3.12)

project(
  libModbusNovusN1500FT
  VERSION 0.0.0
  DESCRIPTION "Communicate with the NOVUS N1500FT using MODBUS-RTU"
  HOMEPAGE_URL "https://github.com/TRIGA-IPR-R1/libModbusNovusN1500FT"
  LANGUAGES CXX)

find_package(PkgConfig REQUIRED)

pkg_search_module(Modbus REQUIRED IMPORTED_TARGET libmodbus)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_C_STANDARD_INCLUDE_DIRECTORIES
    ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES})

set(LIBMODBUSNOVUSN1500FT_SRC src/libModbusNovusN1500FT.cpp)

add_library(modbusNovusN1500FT STATIC ${LIBMODBUSNOVUSN1500FT_SRC})
add_library(modbusNovusN1500FT::modbusNovusN1500FT ALIAS modbusNovusN1500FT)

target_include_directories(modbusNovusN1500FT
                           PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(modbusNovusN1500FT PUBLIC PkgConfig::Modbus)

add_executable(testModbusNovusN1500FT src/test.cpp)
target_link_libraries(testModbusNovusN1500FT PRIVATE modbusNovusN1500FT)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include/libModbusNovusN1500FT/)
install(
  TARGETS modbusNovusN1500FT
  LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
  ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
  RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

set(LibModbusNovusN1500FT_INCLUDE_DIRS
    ${CMAKE_INSTALL_PREFIX}/include/libModbusNovusN1500FT)
set(LibModbusNovusN1500FT_LIBRARIES
    ${CMAKE_INSTALL_PREFIX}/lib/libmodbusNovusN1500FT.a)
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/resources/LibModbusNovusN1500FTConfig.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/resources/LibModbusNovusN1500FTConfig.cmake
  @ONLY)
install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/resources/LibModbusNovusN1500FTConfig.cmake
  DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/cmake/LibModbusNovusN1500FT)
