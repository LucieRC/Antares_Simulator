OMESSAGE("Antares Solver")

#TODO : see to add bigobj support

if(MSVC)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
endif(MSVC)


add_subdirectory(infeasible-problem-analysis)
add_subdirectory(variable)
add_subdirectory(hydro)
add_subdirectory(simulation)
add_subdirectory(ts-generator)
add_subdirectory(utils)
add_subdirectory(optimisation)
add_subdirectory(main)
add_subdirectory(constraints-builder)

#
# Resource file for Windows
#
if(WIN32)
	file(REMOVE "${CMAKE_CURRENT_SOURCE_DIR}/win32/solver.o")
	FILE(COPY "${CMAKE_CURRENT_SOURCE_DIR}/win32/solver.ico" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}/win32/")
	configure_file("${CMAKE_CURRENT_SOURCE_DIR}/win32/solver.rc.cmake"
		"${CMAKE_CURRENT_SOURCE_DIR}/win32/solver.rc")

	if(MINGW)
		# resource compilation for mingw
		add_custom_command(OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/win32/solver.o"
			COMMAND windres.exe "-I${CMAKE_CURRENT_SOURCE_DIR}"
			"-i${CMAKE_CURRENT_SOURCE_DIR}/win32/solver.rc"
			-o "${CMAKE_CURRENT_BINARY_DIR}/win32/solver.o")
		set(SRCS ${SRCS} "${CMAKE_CURRENT_BINARY_DIR}/win32/solver.o")
	else()
		set(SRCS ${SRCS} "win32/solver.rc")
	endif()
endif()



OMESSAGE("  :: application solver")

set(exec_name "antares-${ANTARES_PRG_VERSION}-solver")

add_executable(antares-solver
	misc/options.h
	misc/options.cpp
	misc/process-priority.cpp
	misc/cholesky.h
	misc/cholesky.hxx
	misc/matrix-dp-make.h
	misc/matrix-dp-make.hxx

	misc/system-memory.h
	misc/system-memory.cpp

    misc/write-command-line.h
    misc/write-command-line.cpp

	${SRCS}
	main.cpp
	application.h application.cpp

    signal-handling/common.h
    signal-handling/public.h
    signal-handling/common.cpp
    signal-handling/linux.cpp
    signal-handling/windows.cpp
    )

set_target_properties(antares-solver PROPERTIES OUTPUT_NAME ${exec_name})

set(ANTARES_SOLVER_LIBS
	Antares::args_helper
	Antares::date
	Antares::benchmarking
	Antares::result_writer
	Antares::sys
	Antares::infoCollection
	Antares::checks
	yuni-static-uuid
	yuni-static-core		
	${CMAKE_THREADS_LIBS_INIT}
)

set(ANTARES_SOLVER_LIBS  ${ANTARES_SOLVER_LIBS}
	antares-solver-main-economy
	antares-solver-main-adequacy
	antares-solver-hydro
	antares-solver-variable
	antares-solver-simulation
	antares-solver-ts-generator
	model_antares
	antares-core)

target_link_libraries(antares-solver
		PRIVATE
			${ANTARES_SOLVER_LIBS}
		)

target_include_directories(antares-solver
		PRIVATE
			${CMAKE_SOURCE_DIR}/solver
)

import_std_libs(antares-solver)
executable_strip(antares-solver)

copy_dependency(sirius_solver antares-solver)



#TODO : not working inside macro
install(TARGETS antares-solver EXPORT antares-solver DESTINATION bin)

INSTALL(EXPORT antares-solver
	FILE antares-solverConfig.cmake
	DESTINATION cmake
)

