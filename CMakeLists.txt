cmake_minimum_required(VERSION 3.16)

project("gastar-test")

file(GLOB_RECURSE src_files
    "test/*.cpp"
    "gastar/*.cpp"
    "gastar/*.hpp"
)

add_executable(${PROJECT_NAME}
    ${src_files}
)

target_include_directories(${PROJECT_NAME} PUBLIC
    "./"
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    LINKER_LANGUAGE CXX
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)
