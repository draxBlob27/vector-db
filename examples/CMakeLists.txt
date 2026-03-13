add_executable(cli_demo "cli_demo.cpp")
target_link_libraries(cli_demo
    PRIVATE
        Archive_lib
)