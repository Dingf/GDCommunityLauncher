file(GLOB_RECURSE INCLUDE "include/*.h")
file(GLOB_RECURSE SOURCE "src/*.cpp")

add_executable(extractor ${INCLUDE} ${SOURCE})

target_include_directories(extractor PRIVATE include)
target_include_directories(extractor PRIVATE ${lz4_INCLUDE_DIR})
target_link_libraries(extractor PRIVATE common)
target_link_libraries(extractor PRIVATE game)
target_link_libraries(extractor PRIVATE ${lz4_LIBRARY})

set_target_properties(extractor PROPERTIES OUTPUT_NAME "extractor")