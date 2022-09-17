add_library(common
include/md5.hpp
include/ClientBase.h
include/Date.h
include/FileBuffer.h
include/FileReader.h
include/FileWriter.h
include/FileData.h
include/Log.h
include/Value.h
include/Version.h
include/JSONObject.h
include/URI.h
src/FileReader.cpp
src/FileWriter.cpp
src/Value.cpp)

target_include_directories(common PRIVATE include)

set_target_properties(common PROPERTIES OUTPUT_NAME "common")