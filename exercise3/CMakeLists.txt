include_directories( ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/include )

add_executable(Exercise3 MACOSX_BUNDLE
	src/main.cpp
	src/Viewer.cpp include/Viewer.h
	src/Primitives.cpp include/Primitives.h
	src/SurfaceArea.cpp include/SurfaceArea.h
	src/Volume.cpp include/Volume.h
	src/ShellExtraction.cpp include/ShellExtraction.h
	src/Smoothing.cpp include/Smoothing.h
	src/Stripification.cpp include/Stripification.h
	include/sample_set.h)

target_link_libraries(Exercise3 CG1Common ${LIBS})