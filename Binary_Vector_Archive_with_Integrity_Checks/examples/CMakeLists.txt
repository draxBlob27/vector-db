add_executable(myCLI)
target_sources(myCLI
    PRIVATE
        cli_demo.cpp
)


target_link_libraries(myCLI
    PRIVATE
        archive_lib
)