FUNCTION(PREPEND var prefix)
   SET(listVar "")
   FOREACH(f ${ARGN})
      LIST(APPEND listVar "${prefix}/${f}")
   ENDFOREACH(f)
   SET(${var} "${listVar}" PARENT_SCOPE)
ENDFUNCTION(PREPEND)

FUNCTION(JOIN VALUES GLUE OUTPUT)
  string (REGEX REPLACE "([^\\]|^);" "\\1${GLUE}" _TMP_STR "${VALUES}")
  string (REGEX REPLACE "[\\](.)" "\\1" _TMP_STR "${_TMP_STR}") #fixes escaping
  set (${OUTPUT} "${_TMP_STR}" PARENT_SCOPE)
ENDFUNCTION()

FUNCTION(ProcessGLSLFiles GLSL_FILES_VAR)
	set(GLSL_FILES ${${GLSL_FILES_VAR}})
	PREPEND(GLSL_FILES "${CMAKE_CURRENT_SOURCE_DIR}/glsl/" ${GLSL_FILES})	
	set(${GLSL_FILES_VAR} ${GLSL_FILES} PARENT_SCOPE)
	
	source_group(glsl FILES ${GLSL_FILES} glsl.cpp glsl.h)

	JOIN("${GLSL_FILES}" "," glsl_string)
	set(bin2c_cmdline
		-DOUTPUT_C=glsl.cpp
		-DOUTPUT_H=glsl.h
		"-DINPUT_FILES=${glsl_string}"
		-P "${NANOGUI_DIR}/resources/bin2c.cmake")

	add_custom_command(
	  OUTPUT glsl.cpp glsl.h
	  COMMAND ${CMAKE_COMMAND} ARGS ${bin2c_cmdline}
	  DEPENDS ${GLSL_FILES}
	  COMMENT "Running bin2c"
	  PRE_BUILD VERBATIM)
ENDFUNCTION()

FUNCTION(SetupBuildEnvironment)
	# Enable folders for projects in Visual Studio
	if (CMAKE_GENERATOR MATCHES "Visual Studio")
	  set_property(GLOBAL PROPERTY USE_FOLDERS ON)
	endif()

	# Sanitize build environment for static build with C++11
	if (MSVC)
	  add_definitions (/D "_CRT_SECURE_NO_WARNINGS")
	  add_definitions (/D "__TBB_NO_IMPLICIT_LINKAGE")

	  add_definitions (-DNOMINMAX )
	  add_definitions(/D_USE_MATH_DEFINES)

	  # Parallel build on MSVC (all targets)
	  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")

	  if (NOT CMAKE_SIZEOF_VOID_P EQUAL 8)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:SSE2")

		# Disable Eigen vectorization for Windows 32 bit builds (issues with unaligned access segfaults)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DEIGEN_DONT_ALIGN")
	  endif()

	  # Static build
	  set(CompilerFlags
			CMAKE_CXX_FLAGS CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE
			CMAKE_CXX_FLAGS_MINSIZEREL CMAKE_CXX_FLAGS_RELWITHDEBINFO
			CMAKE_C_FLAGS CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE
			CMAKE_C_FLAGS_MINSIZEREL CMAKE_C_FLAGS_RELWITHDEBINFO)
	  foreach(CompilerFlag ${CompilerFlags})
		string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
	  endforeach()
	elseif(APPLE)
	  # Try to auto-detect a suitable SDK
	  execute_process(COMMAND bash -c "xcodebuild -version -sdk | grep MacOSX | grep Path | head -n 1 | cut -f 2 -d ' '" OUTPUT_VARIABLE CMAKE_OSX_SYSROOT)
	  string(REGEX REPLACE "(\r?\n)+$" "" CMAKE_OSX_SYSROOT "${CMAKE_OSX_SYSROOT}")
	  string(REGEX REPLACE "^.*X([0-9.]*).sdk$" "\\1" CMAKE_OSX_DEPLOYMENT_TARGET "${CMAKE_OSX_SYSROOT}")
	endif()

	if(CMAKE_COMPILER_IS_GNUCXX OR CLANG) 
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-declarations -Wno-unused-result -Wno-misleading-indentation")
	endif() 

	set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} PARENT_SCOPE) 
	set (CMAKE_CXX_STANDARD 14 PARENT_SCOPE)
ENDFUNCTION()
