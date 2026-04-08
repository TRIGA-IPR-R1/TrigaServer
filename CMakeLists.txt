cmake_minimum_required(VERSION 3.14)

include(FetchContent)

project(
  TrigaServer
  VERSION 0.0.1
  DESCRIPTION "Get the real-time values of the Nuclear Reator Triga IPR-R1 and share in network."
  HOMEPAGE_URL "https://github.com/TRIGA-IPR-R1/TrigaServer"
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
cmake_policy(SET CMP0079 NEW)
cmake_policy(SET CMP0167 NEW)

find_package(PkgConfig REQUIRED)
find_package(Boost 1.81 REQUIRED COMPONENTS system)

pkg_search_module(SYSTEMD "systemd")
pkg_search_module(cxxopts REQUIRED IMPORTED_TARGET cxxopts)
pkg_search_module(Modbus REQUIRED IMPORTED_TARGET libmodbus)
pkg_search_module(SerialPort REQUIRED IMPORTED_TARGET libserialport)
pkg_search_module(jsoncpp REQUIRED IMPORTED_TARGET jsoncpp)

find_package(LibModbusSystematomSPU QUIET)
if(NOT LibModbusSystematomSPU_FOUND)
  message(STATUS "LibModbusSystematomSPU not found, Downloading it.")
  FetchContent_Declare(
    LibModbusSystematomSPU
    GIT_REPOSITORY https://github.com/TRIGA-IPR-R1/libModbusSystematomSPU
    GIT_TAG main
    EXCLUDE_FROM_ALL)
  FetchContent_MakeAvailable(LibModbusSystematomSPU)
else()
  set(opcTrigaPLC "")
endif()

find_package(LibOpcTrigaPLC QUIET)
if(NOT LibOpcTrigaPLC_FOUND)
  message(STATUS "LibOpcTrigaPLC not found, Downloading it.")
  FetchContent_Declare(
    LibOpcTrigaPLC
    GIT_REPOSITORY https://github.com/TRIGA-IPR-R1/libOpcTrigaPLC
    GIT_TAG main
    EXCLUDE_FROM_ALL)
  FetchContent_MakeAvailable(LibOpcTrigaPLC)
else()
  set(libOpcTrigaPLC "")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_C_STANDARD_INCLUDE_DIRECTORIES
    ${CMAKE_C_IMPLICIT_INCLUDE_DIRECTORIES})

set(TRIGASERVER_SRC ${PROJECT_SOURCE_DIR}/src/main.cpp
                    ${PROJECT_SOURCE_DIR}/src/TrigaServer.cpp
                    ${PROJECT_SOURCE_DIR}/src/libFormatCSV.cpp)

add_executable(trigaserver ${TRIGASERVER_SRC})

target_include_directories(
  trigaserver
  PRIVATE ${PROJECT_SOURCE_DIR}/include
          ${Boost_INCLUDE_DIRS}
          ${LibOpcTrigaPLC_INCLUDE_DIRS}
          ${LibModbusSystematomSPU_INCLUDE_DIRS})

target_link_libraries(
  trigaserver
  PRIVATE ${Boost_LIBRARIES}
          ${LibOpcTrigaPLC_LIBRARIES}
          ${LibModbusSystematomSPU_LIBRARIES}
          opcTrigaPLC
          modbusSystematomSPU
          PkgConfig::cxxopts
          PkgConfig::Modbus
          PkgConfig::SerialPort
          PkgConfig::jsoncpp)

target_compile_options(
  trigaserver
  PRIVATE ${CXXOPTS_CFLAGS_OTHER} ${MODBUS_CFLAGS_OTHER}
          ${SERIALPORT_CFLAGS_OTHER} ${JSONCPP_CFLAGS_OTHER}
          ${LibOpcTrigaPLC_CFLAGS} ${LibModbusSystematomSPU_CFLAGS})

install(TARGETS trigaserver DESTINATION bin)
if("${DONT_INSTALL_CONF}" STREQUAL "")
  install(FILES ${PROJECT_SOURCE_DIR}/resources/trigaserver.conf
          DESTINATION /etc/)
endif()

if(SYSTEMD_FOUND)
  if("${SYSTEMD_SERVICES_INSTALL_DIR}" STREQUAL "")
    execute_process(
      COMMAND ${PKG_CONFIG_EXECUTABLE} --variable=systemdsystemunitdir systemd
      OUTPUT_VARIABLE SYSTEMD_SERVICES_INSTALL_DIR)
    string(REGEX REPLACE "[ \t\n]+" "" SYSTEMD_SERVICES_INSTALL_DIR
                         "${SYSTEMD_SERVICES_INSTALL_DIR}")
  endif()
  configure_file(${PROJECT_SOURCE_DIR}/resources/trigaserver.service
                 ${CMAKE_BINARY_DIR}/trigaserver.service)
  install(FILES ${CMAKE_BINARY_DIR}/trigaserver.service
          DESTINATION ${SYSTEMD_SERVICES_INSTALL_DIR})
endif()

# ==============================================================================
# Configuração do CPack para geração do pacote .deb
# ==============================================================================

# Reaproveitando as variáveis já definidas no project()
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "${PROJECT_HOMEPAGE_URL}")

# Configurações específicas para Debian (.deb)
set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Thalles Campagnani <thalles.campagnani@cdtn.br>")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)   # Descobre as dependências automaticamente!

# Inclui o módulo CPack
include(CPack)