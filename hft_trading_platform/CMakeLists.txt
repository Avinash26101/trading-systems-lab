add_library(hft_platform
  src/backtester.cpp
  src/exchange_simulator.cpp
  src/order_book.cpp
  src/order_gateway.cpp
  src/risk_engine.cpp
  src/strategy_engine.cpp
  src/udp_feed.cpp
)

target_include_directories(hft_platform PUBLIC include)
if(WIN32)
  target_link_libraries(hft_platform PUBLIC ws2_32)
endif()

add_executable(hft_backtest src/backtest_main.cpp)
target_link_libraries(hft_backtest PRIVATE hft_platform)

add_executable(hft_udp_demo src/udp_demo.cpp)
target_link_libraries(hft_udp_demo PRIVATE hft_platform)

add_executable(hft_tests tests/hft_tests.cpp)
target_link_libraries(hft_tests PRIVATE hft_platform)
add_test(NAME hft_tests COMMAND hft_tests)
set_tests_properties(hft_tests PROPERTIES
  TIMEOUT 10
  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
