# MICROSOFT-SIGNALR_FOUND
# MICROSOFT-SIGNALR_INCLUDE_DIR
# MICROSOFT-SIGNALR_LIBRARIES
# MICROSOFT-SIGNALR_LIBRARY
# MICROSOFT-SIGNALR_LIBRARY_DEBUG

include(FindPackageHandleStandardArgs)

find_path(MICROSOFT-SIGNALR_INCLUDE_DIR
	NAMES
		signalrclient/http_client.h
	HINTS
		${microsoft-signalr_DIR}/include
		
)
find_library(MICROSOFT-SIGNALR_LIBRARY
	NAMES
		microsoft-signalr
	HINTS
		${microsoft-signalr_DIR}/lib
)
find_library(MICROSOFT-SIGNALR_LIBRARY_DEBUG
	NAMES
		microsoft-signalrd
	HINTS
		${microsoft-signalr_DIR}/lib
)

if(MICROSOFT-SIGNALR_LIBRARY AND MICROSOFT-SIGNALR_LIBRARY_DEBUG)
	set(MICROSOFT-SIGNALR_LIBRARIES optimized ${MICROSOFT-SIGNALR_LIBRARY} debug ${MICROSOFT-SIGNALR_LIBRARY_DEBUG})
else()
	set(MICROSOFT-SIGNALR_LIBRARIES ${MICROSOFT-SIGNALR_LIBRARY})
endif()

set(MICROSOFT-SIGNALR_INCLUDE_DIRS
	${MICROSOFT-SIGNALR_INCLUDE_DIR}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
	microsoft-signalr DEFAULT_MSG
	MICROSOFT-SIGNALR_LIBRARY MICROSOFT-SIGNALR_INCLUDE_DIR
)

if(MICROSOFT-SIGNALR_FOUND)
	if(NOT TARGET microsoft-signalr::microsoft-signalr)
		add_library(microsoft-signalr::microsoft-signalr STATIC IMPORTED)
		set_target_properties(microsoft-signalr::microsoft-signalr PROPERTIES INTERFACE_INCLUDE_DIRECTORIES "${MICROSOFT-SIGNALR_INCLUDE_DIRS}")
		if(MICROSOFT-SIGNALR_LIBRARY)
			set_property(TARGET microsoft-signalr::microsoft-signalr APPEND PROPERTY IMPORTED_CONFIGURATIONS "RELEASE")
			set_target_properties(microsoft-signalr::microsoft-signalr PROPERTIES IMPORTED_LINK_INTERFACE_LANGUAGES_RELEASE "CXX")
			set_target_properties(microsoft-signalr::microsoft-signalr PROPERTIES IMPORTED_LOCATION_RELEASE "${MICROSOFT-SIGNALR_LIBRARY}")
		endif()
		if(MICROSOFT-SIGNALR_LIBRARY_DEBUG)
			set_property(TARGET microsoft-signalr::microsoft-signalr APPEND PROPERTY IMPORTED_CONFIGURATIONS "DEBUG")
			set_target_properties(microsoft-signalr::microsoft-signalr PROPERTIES IMPORTED_LINK_INTERFACE_LANGUAGES_DEBUG "CXX")
			set_target_properties(microsoft-signalr::microsoft-signalr PROPERTIES IMPORTED_LOCATION_DEBUG "${MICROSOFT-SIGNALR_LIBRARY_DEBUG}")
		endif()
	endif()
endif()
