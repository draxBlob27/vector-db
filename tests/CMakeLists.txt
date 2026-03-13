include(FetchContent)
FetchContent_Declare(
  googletest
  # Specify the commit you depend on and update it regularly.
  URL https://github.com/google/googletest/archive/5376968f6948923e2411081fd9372e71a59d8e77.zip
)

FetchContent_MakeAvailable(googletest)

add_executable(test_safeVector "test_safeVector.cpp")
target_link_libraries(test_safeVector
    PUBLIC
        gtest_main
)
target_include_directories(test_safeVector PRIVATE ${PROJECT_SOURCE_DIR}/include)


add_executable(test_archive "test_archive.cpp")
target_link_libraries(test_archive
    PRIVATE
        Archive_lib
    PUBLIC
        gtest_main
)

add_executable(test_unit "test_unit.cpp")
target_link_libraries(test_unit
    PRIVATE
        VectorStore_lib
    PUBLIC
        gtest_main
)

add_executable(test_vdb "test_vdb.cpp")
target_link_libraries(test_vdb
    PRIVATE
        VectorStore_lib
    PUBLIC
        gtest_main
)

add_executable(test_LSH "test_LSH.cpp")
target_link_libraries(test_LSH
    PRIVATE
        LSH_Index
    
    PUBLIC
        gtest_main
)

add_executable(test_NSW "test_NSW.cpp")
target_link_libraries(test_NSW
    PRIVATE
        NSW_Index

    PUBLIC
        gtest_main
)