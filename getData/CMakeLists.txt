cmake_minimum_required(VERSION 3.5)

project(getData LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(AMReX_BUILD_SHARED_LIBS OFF)
set(AMReX_MPI FALSE)

add_subdirectory(amrex/)

add_executable(getData main.cpp)

target_link_libraries(getData AMReX::amrex_3d)

include(GNUInstallDirs)
install(TARGETS getData
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
