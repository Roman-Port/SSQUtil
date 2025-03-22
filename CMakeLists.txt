cmake_minimum_required (VERSION 3.8)

project ("SSQUtil")

# Add library
add_subdirectory ("lib/libssq")

# Add main
add_executable (SSQUtil "src/main.c")

# Link
target_link_libraries(SSQUtil ssq)
target_include_directories(SSQUtil PRIVATE "lib/libssq/include/")
