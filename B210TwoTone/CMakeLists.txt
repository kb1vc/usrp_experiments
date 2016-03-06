cmake_minimum_required(VERSION 2.8)

project(B210TwoTone)

find_package(Boost 1.45 REQUIRED COMPONENTS program_options thread date_time system filesystem serialization regex)

find_package(UHD REQUIRED)

include_directories(SYSTEM ${Boost_INCLUDE_DIRS} ${UHD_INCLUDE_DIRS})

set(TEST_LIBS ${Boost_LIBRARIES} ${UHD_LIBRARIES})


set(B210TwoTone_SRCS B210TwoTone.cxx)

add_executable(B210TwoTone ${B210TwoTone_SRCS})

target_link_libraries(B210TwoTone ${TEST_LIBS})


