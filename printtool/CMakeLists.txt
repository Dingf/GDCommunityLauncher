file(GLOB_RECURSE INCLUDE "include/*.h")
file(GLOB_RECURSE SOURCE "src/*.cpp")

add_executable(printtool ${INCLUDE} ${RESOURCE} ${SOURCE})

target_include_directories(printtool PRIVATE include)
target_link_libraries(printtool PRIVATE OpenSSL::SSL)
target_link_libraries(printtool PRIVATE game)

set_target_properties(printtool PROPERTIES
    OUTPUT_NAME "printtool"
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)