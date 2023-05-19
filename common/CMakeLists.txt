file(GLOB_RECURSE INCLUDE "include/*.h")
file(GLOB_RECURSE SOURCE "src/*.cpp")

add_library(common ${INCLUDE} ${SOURCE})

target_include_directories(common PRIVATE include)

set_target_properties(common PROPERTIES OUTPUT_NAME "common")