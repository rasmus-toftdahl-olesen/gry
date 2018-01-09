cmake_minimum_required (VERSION 2.6)
project (gry)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")
endif()

add_executable(gry_server src/valuebuffer.cxx src/utils.cxx src/source.cxx src/repository.cxx src/web.cxx src/udp.cxx src/main.cxx src/webvaluebuffer.cxx)
include_directories(include)

find_package(Boost COMPONENTS filesystem thread system chrono unit_test_framework)
if(Boost_FOUND)
  include_directories(${Boost_INCLUDE_DIRS})
  link_directories(${Boost_LIBRARY_DIRS})
  target_link_libraries(gry_server ${Boost_LIBRARIES})
else()
  error(BOOST NOT FOUND)
endif()

find_package(PkgConfig)
pkg_check_modules(PION REQUIRED pion)
include_directories(${PION_INCLUDE_DIRS})
link_directories(${PION_LIBRARY_DIRS})
target_link_libraries(gry_server ${PION_LIBRARIES})
if(PION_VERSION VERSION_LESS "5.0.7")
  message("Using pion version ${PION_VERSION} with non-const request_ptr and connection_ptr.")
  set(GRY_NON_CONST_PION_PTRS 1)
endif()

set(GRY_WEB_PORT 8123 CACHE STRING "Port to listen for HTTP traffic on.")
set(GRY_UDP_PORT 8124 CACHE STRING "Port to listen for UDP traffic on.")
option(GRY_INCLUDE_STATIC_WEB "Include the static web files in the gry_server executable." ON)
set(GRY_STATIC_WEB_DIR "${CMAKE_SOURCE_DIR}/web/")
configure_file(src/config.h.in ${CMAKE_BINARY_DIR}/config.h)

file(WRITE ${CMAKE_BINARY_DIR}/generated_web.cxx "")
function(gry_generate_web FILENAME)
  file(READ web/${FILENAME} CONTENT)
  string(REPLACE "\\" "\\\\" CONTENT "${CONTENT}")
  string(REPLACE "\"" "\\\"" CONTENT "${CONTENT}")
  string(REPLACE "\n" "\\n" CONTENT "${CONTENT}")
  string(REPLACE "." "_" IDENTIFIER "${FILENAME}")
  file(APPEND ${CMAKE_BINARY_DIR}/generated_web.cxx "static const char * s_${IDENTIFIER} = \"${CONTENT}\";\n\n")
endfunction()

gry_generate_web(index.html)
gry_generate_web(jquery.js)
gry_generate_web(jquery.flot.min.js)

include_directories(${CMAKE_BINARY_DIR})

add_executable(gry_unit_tests src/valuebuffer.cxx src/source.cxx src/utils.cxx src/webvaluebuffer.cxx
  tests/main.cxx tests/SourceTests.cxx tests/ValueBufferTests.cxx tests/CircularBufferTests.cxx
  tests/ValueBufferRealWorldTests.cxx)
target_link_libraries(gry_unit_tests ${Boost_LIBRARIES} ${PION_LIBRARIES})
add_custom_target(check COMMAND ./gry_unit_tests DEPENDS gry_unit_tests)
add_custom_target(run COMMAND ./gry_server DEPENDS gry_server)
