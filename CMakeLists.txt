cmake_minimum_required(VERSION 3.15)
project(MICROMEM VERSION 0.1 LANGUAGES C)

# Build options
option(BUILD_SHARED_LIBS "Build MICROMEM as shared library" OFF)
option(MICROMEM_BUILD_TESTS "Build tests (if tests exist)" OFF)

# Collect sources
file(GLOB_RECURSE MICROMEM_SOURCES
	"${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

# Library target
add_library(MICROMEM ${MICROMEM_SOURCES})

target_include_directories(MICROMEM
	PUBLIC
		$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
		$<INSTALL_INTERFACE:include>
)

set_target_properties(MICROMEM PROPERTIES
	C_STANDARD 99
	C_STANDARD_REQUIRED ON
	POSITION_INDEPENDENT_CODE OFF
)

# Compiler warnings (adjust for your toolchain)
if(MSVC)
	target_compile_options(MICROMEM PRIVATE /W4 /wd4996)
else()
	target_compile_options(MICROMEM PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Optional example: build if examples/main.c exists
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples/main.c")
	add_executable(MICROMEM_example examples/main.c)
	target_link_libraries(MICROMEM_example PRIVATE MICROMEM)
	target_include_directories(MICROMEM_example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
endif()

# Optional tests: user can put tests under tests/ and enable MICROMEM_BUILD_TESTS
if(MICROMEM_BUILD_TESTS)
	enable_testing()
	if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/tests")
		# Example test targets (uncomment and adapt to your tests):
		# add_executable(test_basic tests/test_basic.c)
		# target_link_libraries(test_basic PRIVATE MICROMEM)
		# add_test(NAME basic COMMAND test_basic)
		message(STATUS "MICROMEM_BUILD_TESTS enabled — add test targets under 'tests/' in this CMakeLists.txt")
	else()
		message(STATUS "MICROMEM_BUILD_TESTS enabled but no 'tests/' folder found")
	endif()
endif()

# Install rules (simple)
include(GNUInstallDirs)
install(TARGETS MICROMEM
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Helpful configure-time messages
message(STATUS "Project: ${PROJECT_NAME} ${PROJECT_VERSION}")
message(STATUS "Source files: ${MICROMEM_SOURCES}")
message(STATUS "Build shared libs: ${BUILD_SHARED_LIBS}")
message(STATUS "Build tests: ${MICROMEM_BUILD_TESTS}")
