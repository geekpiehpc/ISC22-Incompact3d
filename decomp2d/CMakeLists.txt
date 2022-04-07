file(GLOB files_decomp decomp_2d.f90
        glassman.f90
        io.f90)
include_directories(${CMAKE_SOURCE_DIR}/decomp2d)
if (${FFT_Choice} MATCHES "generic")
    file(GLOB files_fft fft_generic.f90)
elseif (${FFT_Choice} MATCHES "fftw3")
    file(GLOB files_fft fft_fftw3.f90)
elseif (${FFT_Choice} MATCHES "mkl")
    file(GLOB files_fft fft_mkl.f90)
    list(APPEND files_decomp mkl_dfti.f90)
else ()
    message(FATAL_ERROR "Unknown FFT_CHOICE ${FFT_Choice}")
endif ()

set(SRCFILES ${files_decomp} ${files_fft})

add_library(decomp2d STATIC ${SRCFILES})

if (${FFT_Choice} MATCHES "fftw3")
    find_package(FFTW3)
    target_include_directories(decomp2d PRIVATE "${FFTW3_INCLUDE_DIRS}")
    target_link_directories(decomp2d PRIVATE "${FFTW3_LIBRARY_DIRS}")
    target_link_libraries(decomp2d PRIVATE fftw3 fftw3f)
elseif (${FFT_Choice} MATCHES "mkl")
    target_include_directories(decomp2d PUBLIC "$ENV{MKLROOT}/include")
    target_link_libraries(decomp2d PUBLIC
            -Wl,--start-group
            "$ENV{MKLROOT}/lib/intel64/libmkl_intel_lp64.a"
            "$ENV{MKLROOT}/lib/intel64/libmkl_sequential.a"
            "$ENV{MKLROOT}/lib/intel64/libmkl_core.a"
            -Wl,--end-group
            ${CMAKE_DL_LIBS}
            )
endif ()

if (MPI_FOUND)
    target_link_libraries(decomp2d PRIVATE MPI::MPI_Fortran)
endif (MPI_FOUND)
if (ADIOS2_FOUND)
    target_link_libraries(decomp2d PRIVATE adios2::fortran_mpi)
endif ()

install(TARGETS decomp2d
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        )