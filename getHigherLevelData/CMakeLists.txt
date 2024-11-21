cmake_minimum_required(VERSION 3.5)

project(getHigherLevelData LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(AMReX_BUILD_SHARED_LIBS OFF)
set(AMReX_MPI FALSE)

add_subdirectory(amrex/)

add_executable(getHigherLevelData main.cpp)

target_link_libraries(getHigherLevelData AMReX::amrex_3d)

include(GNUInstallDirs)
install(TARGETS getHigherLevelData
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
