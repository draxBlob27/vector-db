find_package(GTest REQUIRED)

add_executable(run_tests test_safeVector.cpp)
target_link_libraries(run_tests PRIVATE GTest::Main)
target_compile_options(run_tests PRIVATE -Wall -Wextra -Wpedantic)

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    target_compile_options(run_tests PRIVATE -fsanitize=address PRIVATE -fsanitize=undefined PRIVATE -fno-omit-frame-pointer)

    target_link_options(run_tests PRIVATE -fsanitize=address PRIVATE -fsanitize=undefined)
endif()

target_include_directories(run_tests PRIVATE ${PROJECT_SOURCE_DIR}/include)