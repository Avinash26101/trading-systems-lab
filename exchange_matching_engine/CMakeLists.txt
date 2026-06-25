find_package(Threads REQUIRED)

add_library(exchange_engine
  src/order_book.cpp
)

target_include_directories(exchange_engine PUBLIC include)
target_link_libraries(exchange_engine PUBLIC Threads::Threads)

add_executable(exchange_demo src/demo.cpp)
target_link_libraries(exchange_demo PRIVATE exchange_engine)

add_executable(exchange_benchmark bench/benchmark.cpp)
target_link_libraries(exchange_benchmark PRIVATE exchange_engine)

add_executable(exchange_tests tests/order_book_tests.cpp)
target_link_libraries(exchange_tests PRIVATE exchange_engine)
add_test(NAME exchange_tests COMMAND exchange_tests)
set_tests_properties(exchange_tests PROPERTIES TIMEOUT 10)
