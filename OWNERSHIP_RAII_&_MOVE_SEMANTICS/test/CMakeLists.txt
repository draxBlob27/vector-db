find_package(GTest REQUIRED)

add_executable(run_tests test_safeVector.cpp)
target_link_libraries(run_tests PRIVATE GTest::Main)
target_compile_options(run_tests PRIVATE -Wall -Wextra -Wpedantic)
target_include_directories(run_tests PUBLIC ${PROJECT_SOURCE_DIR}/include)