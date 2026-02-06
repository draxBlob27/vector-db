find_package(GTest REQUIRED)

add_executable(myArchive)
target_sources(myArchive PRIVATE test_archive.cpp)

target_link_libraries(myArchive
    PRIVATE 
        archive_lib
        GTest::Main
)

target_include_directories(archive_lib 
    PUBLIC 
        ../include/vectorDB 
)

# target_compile_options(myArchive)
target_include_directories(myArchive PRIVATE ${GTEST_INCLUDE_DIRS}/include)

