include_directories( ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/include )

add_executable(Exercise5 MACOSX_BUNDLE
	src/main.cpp
	src/Viewer.cpp include/Viewer.h
	src/Parametrization.cpp include/Parametrization.h
	src/Registration.cpp include/Registration.h
	)

target_link_libraries(Exercise5 CG1Common ${LIBS})