add_library(game
include/UID.h
include/GDDataBlock.h
include/Item.h
include/ItemContainer.h
include/Skill.h
include/Faction.h
include/Quest.h
include/Stash.h
include/SharedStash.h
include/Character.h
include/ItemDatabase.h
src/UID.cpp
src/GDDataBlock.cpp
src/Item.cpp
src/ItemContainer.cpp
src/Skill.cpp
src/Faction.cpp
src/Quest.cpp
src/Stash.cpp
src/SharedStash.cpp
src/Character.cpp
src/ItemDatabase.cpp)

target_include_directories(game PUBLIC include)
target_link_libraries(game PRIVATE ${cpprestsdk_LIBRARY})

set_target_properties(game PROPERTIES OUTPUT_NAME "game")