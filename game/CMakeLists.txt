file(GLOB_RECURSE INCLUDE "include/*.h")
file(GLOB_RECURSE SOURCE "src/*.cpp")

add_library(game ${INCLUDE} ${SOURCE})

target_include_directories(game PUBLIC include)
target_link_libraries(game PRIVATE ${cpprestsdk_LIBRARY})
target_link_libraries(game PRIVATE common)

set_target_properties(game PROPERTIES OUTPUT_NAME "game")