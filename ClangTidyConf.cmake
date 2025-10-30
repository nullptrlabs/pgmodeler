# Configure clang-tidy to run during compilation
# This will show warnings in VS Code/Qt Creator issues panel during build

# Declare USE_CLANG_TIDY as a CMake option if not already set via command line or settings.json (VS Code)
# This allows the user to control when to enable/disable clang-tidy
if(NOT DEFINED USE_CLANG_TIDY)
	set(USE_CLANG_TIDY OFF CACHE BOOL "Enable clang-tidy static analysis during compilation" FORCE)
endif()

# Allow custom clang-tidy path via CLANG_TIDY_PATH variable
# VS Code: This can be set in settings.json: "CLANG_TIDY_PATH": "/path/to/clang-tidy-18"
# Priority:
#   1. Custom path from CLANG_TIDY_PATH variable (if exists)
#   2. System clang-tidy from PATH (fallback)
unset(CLANG_TIDY_EXE CACHE)

if(DEFINED CLANG_TIDY_PATH)
	# Custom path provided - validate it
	if(NOT EXISTS "${CLANG_TIDY_PATH}")
		message(FATAL_ERROR "CLANG_TIDY_PATH is set but file does not exist: ${CLANG_TIDY_PATH}")
	endif()
	
	if(NOT IS_ABSOLUTE "${CLANG_TIDY_PATH}")
		message(FATAL_ERROR "CLANG_TIDY_PATH must be an absolute path: ${CLANG_TIDY_PATH}")
	endif()
	
	set(CLANG_TIDY_EXE "${CLANG_TIDY_PATH}" CACHE FILEPATH "Path to clang-tidy executable")
else()
	# Try to find clang-tidy in system PATH
	find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
endif()

if(CLANG_TIDY_EXE AND USE_CLANG_TIDY)
	# Validate that the executable is actually clang-tidy
	execute_process(
		COMMAND "${CLANG_TIDY_EXE}" --version
		OUTPUT_VARIABLE CLANG_TIDY_VERSION
		RESULT_VARIABLE CLANG_TIDY_VERSION_RESULT
		OUTPUT_STRIP_TRAILING_WHITESPACE
		ERROR_QUIET
	)
	
	# Check if command succeeded and output contains "clang-tidy" or "LLVM"
	# (some versions output "clang-tidy version X", others just "LLVM version X")
	if(NOT CLANG_TIDY_VERSION_RESULT EQUAL 0)
		if(DEFINED CLANG_TIDY_PATH)
			message(FATAL_ERROR "Failed to execute clang-tidy at: ${CLANG_TIDY_EXE}\n"
				"The specified binary could not be executed or is not a valid clang-tidy executable.\n"
				"Make sure the path points to a valid clang-tidy binary.")
		else()
			message(WARNING "Found executable 'clang-tidy' but failed to execute it: ${CLANG_TIDY_EXE}")
			set(USE_CLANG_TIDY OFF CACHE BOOL FORCE)
			unset(CLANG_TIDY_EXE CACHE)
		endif()
	elseif(NOT CLANG_TIDY_VERSION MATCHES "(clang-tidy|LLVM)")
		if(DEFINED CLANG_TIDY_PATH)
			message(FATAL_ERROR "Invalid clang-tidy executable at: ${CLANG_TIDY_EXE}\n"
				"The specified binary is not a valid clang-tidy executable.\n"
				"Output from '${CLANG_TIDY_EXE} --version':\n${CLANG_TIDY_VERSION}")
		else()
			message(WARNING "Found executable 'clang-tidy' but it doesn't appear to be valid: ${CLANG_TIDY_EXE}")
			set(USE_CLANG_TIDY OFF CACHE BOOL "Enable clang-tidy static analysis during compilation" FORCE)
			unset(CLANG_TIDY_EXE CACHE)
		endif()
	else()
		# Extract and display version
		if(CLANG_TIDY_VERSION MATCHES "version ([0-9]+\\.[0-9]+\\.[0-9]+)")
			set(CLANG_TIDY_VERSION_NUM "${CMAKE_MATCH_1}")
			message(STATUS "Using clang-tidy version ${CLANG_TIDY_VERSION_NUM}: ${CLANG_TIDY_EXE}")
		else()
			message(STATUS "Using clang-tidy: ${CLANG_TIDY_EXE}")
		endif()
		
		# clang-tidy reads configuration from .clang-tidy file
		set(CMAKE_CXX_CLANG_TIDY 
			"${CLANG_TIDY_EXE}"
			#"--header-filter=.*/pgmodeler/(apps|libs|plugins|priv-plugins)/.*\\.(h|hpp|cpp)$"
			CACHE STRING "clang-tidy command line" FORCE
		)
	endif()
elseif(NOT CLANG_TIDY_EXE)
	# Disable USE_CLANG_TIDY if clang-tidy executable not found
	set(USE_CLANG_TIDY OFF CACHE BOOL FORCE)
endif()
