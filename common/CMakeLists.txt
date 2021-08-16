add_library(common
include/FileReader.h
include/Log.h
include/Value.h
include/Version.h
include/JSONObject.h
src/FileReader.cpp
src/Value.cpp)

target_include_directories(common PRIVATE include)

set_target_properties(common PROPERTIES OUTPUT_NAME "common")