# CMakeList.txt : CMake project for NetMQ, include source and define
# project specific logic here.
#

# Add source to this project's executable.
add_executable(NetMQ)

if(CMAKE_VERSION VERSION_GREATER 3.25)
	set_property(TARGET NetMQ PROPERTY CXX_STANDARD 23)
endif()

# Check the Operating System and include the appropriate file for execution
if(WIN32)
	target_sources(NetMQ PRIVATE
		"src/sys/win32/Main.cpp"
		"src/sys/win32/net/WinSockAPI.h"
		"src/sys/win32/net/WinSockAPI.cpp"
	)
elseif(LINUX OR APPLE)
	target_sources(NetMQ PRIVATE
		"src/sys/posix/Main.cpp"
	)
endif()

# Specify the common OS independent source files for the NetMQ project.
target_sources(NetMQ PRIVATE
	"src/framework/Log.h"
	"src/framework/Log.cpp"
	"src/framework/CmdSystem.h"
	"src/framework/CmdSystem.cpp"
 "src/framework/cmd/Cmd.h" "src/framework/cmd/PublishCmd.cpp"   "src/framework/cmd/SubscribeCmd.cpp"  "src/framework/cmd/ConnectCmd.cpp" "src/framework/cmd/UnsubscribeCmd.cpp"   "src/framework/cmd/DisconnectCmd.cpp")

include_directories(src)

# Set up linker options
if(WIN32)
	target_compile_definitions(NetMQ PRIVATE UNICODE _UNICODE)
	target_link_libraries(NetMQ PUBLIC ws2_32)
	target_link_libraries(NetMQ PUBLIC mswsock)
endif()

# Set up all the compiler options here
if(MSVC)
	target_compile_options(NetMQ PRIVATE "/W4" "/WX" "/permissive-" "/analyze" "/fp:fast" "/FAs")				# Common options for all build types
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")																		# Debug build compiler options
		target_compile_options(NetMQ PRIVATE
			"/Zi"
			"/fsanitize=address"
			"/Od"
			"/MDd"
			"/JMC"
		)
		target_link_options(NetMQ PRIVATE "/DEBUG")																# Ensure PDB file is generated
	elseif(CMAKE_BUILD_TYPE STREQUAL "Release")																	# Release build compiler options
		target_compile_options(NetMQ PRIVATE "/O2" "/MD" "/GL" "/Gw")
	endif()

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")							# CLANG and GCC compiler options
	target_compile_options(NetMQ PRIVATE "-Wall" "-Werror" "-Wpedantic" "-ffast-math" "-fanalyzer")				# Common options for all build types
	if(CMAKE_BUILD_TYPE STREQUAL "Debug")																		# Debug build compiler options
		target_compile_options(NetMQ PRIVATE
			"-g"
			#"-fsanitize=leak"
			#"-fsanitize=undefined"
			#"-fsanitize=address"
			#"-fsanitize=thread"
			"-O0"
		)
		target_link_libraries(NetMQ PRIVATE
			#"-fsanitize=leak"
			#"-fsanitize=undefined"
			#"-fsanitize=address"
			#"-fsanitize=thread"
		)
	elseif(CMAKE_BUILD_TYPE STREQUAL "Release")																	# Release build compiler options
		target_compile_options(NetMQ PRIVATE "-O3" "-flto")
	endif()
endif()

# TODO: Add tests and install targets if needed.
