cmake_minimum_required(VERSION 3.5)

project(loadData LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(AMReX_BUILD_SHARED_LIBS OFF)
set(AMReX_MPI FALSE)

add_subdirectory(amrex/)

add_executable(loadData main.cpp)

target_link_libraries(loadData AMReX::amrex_3d)

include(GNUInstallDirs)
install(TARGETS loadData
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
