add_library(common
include/external/md5.hpp
include/ClientBase.h
include/Configuration.h
include/Date.h
include/FileBuffer.h
include/FileReader.h
include/FileWriter.h
include/FileData.h
include/JSONObject.h
include/Log.h
include/MD5.h
include/UpdateThread.h
include/URI.h
include/Value.h
include/Version.h
src/Configuration.cpp
src/FileReader.cpp
src/FileWriter.cpp
src/Value.cpp)

target_include_directories(common PRIVATE include)

set_target_properties(common PROPERTIES OUTPUT_NAME "common")