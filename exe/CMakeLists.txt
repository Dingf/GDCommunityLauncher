include_directories(include)

add_executable(exe WIN32 src/main.cpp)

set_target_properties(exe PROPERTIES OUTPUT_NAME "GDCommunityLauncher")