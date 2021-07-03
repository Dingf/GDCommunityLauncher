add_executable(extractor
include/DBRecord.h

include/DDSImage.h
include/TEXImage.h
include/ItemDBR.h
src/DBRecord.cpp
src/DDSImage.cpp
src/TEXImage.cpp
src/ItemDBR.cpp
src/main.cpp)

target_include_directories(extractor PRIVATE include)
target_include_directories(extractor PRIVATE ${lz4_INCLUDE_DIR})
target_link_libraries(extractor PRIVATE common)

set_target_properties(exe PROPERTIES OUTPUT_NAME "extractor")