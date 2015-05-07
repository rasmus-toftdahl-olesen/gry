cmake_minimum_required (VERSION 2.6)
project (gry)

if(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-long-long -pedantic")
endif()

add_executable(gry_server src/main.cxx src/web.cxx src/source.cxx src/repository.cxx)
include_directories(include)

find_package(Boost COMPONENTS filesystem thread)
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
