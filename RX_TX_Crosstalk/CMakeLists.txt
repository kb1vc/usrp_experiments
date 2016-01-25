cmake_minimum_required(VERSION 2.8)

project(RX_TX_Crosstalk)

find_package(Boost 1.45 REQUIRED COMPONENTS program_options thread date_time system filesystem serialization regex)

find_package(UHD REQUIRED)

include_directories(SYSTEM ${Boost_INCLUDE_DIRS} ${UHD_INCLUDE_DIRS})

set(TEST_LIBS ${Boost_LIBRARIES} ${UHD_LIBRARIES})


set(RX_TX_Crosstalk_SRCS RX_TX_Crosstalk.cxx)

add_executable(RX_TX_Crosstalk ${RX_TX_Crosstalk_SRCS})

target_link_libraries(RX_TX_Crosstalk ${TEST_LIBS})


