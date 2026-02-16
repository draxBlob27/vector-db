find_package(GTest REQUIRED)

add_executable(dataset_test)
target_sources(dataset_test
    PRIVATE
        test_vdb.cpp
)

target_link_libraries(dataset_test
    PRIVATE
        VectorStore_lib
        GTest::Main
)