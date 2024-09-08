
# Lua core source files.
set(LUA_CORE_SRC
	lapi.c
	lauxlib.c
	lbaselib.c
	lcode.c
	lcorolib.c
	ldblib.c
	ldebug.c
	ldo.c
	ldump.c
	lfunc.c
	lgc.c
	linit.c
	liolib.c
	llex.c
	lmathlib.c
	lmem.c
	loadlib.c
	lobject.c
	lopcodes.c
	loslib.c
	lparser.c
	lstate.c
	lstring.c
	lstrlib.c
	ltable.c
	ltablib.c
	ltm.c
	lundump.c
	lvm.c
	lzio.c
)
if (LUA_ENABLE_INTERNAL_TESTS)
	list(APPEND LUA_CORE_SRC "ltests.c")
	add_definitions(-DLUA_USER_H=\"ltests.h\")
endif()

# Lua library
add_library(lua STATIC ${LUA_CORE_SRC})
target_link_libraries(lua ${LIBS})
set_target_properties(lua PROPERTIES
	VERSION ${LUA_VERSION}
	CLEAN_DIRECT_OUTPUT 1
)

# Compile code as C++
set_source_files_properties(${LUA_CORE_SRC} PROPERTIES LANGUAGE CXX)

if(LUA_ENABLE_INTERNAL_TESTS)
	target_include_directories(lua PRIVATE src)
	add_executable(lua_binary ${LUA_CORE_SRC} lua.c)
	set_source_files_properties(lua.c PROPERTIES LANGUAGE CXX)
endif()
