cmake_minimum_required(VERSION 3.16)

project(writePlts LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(AMReX_BUILD_SHARED_LIBS OFF)
set(AMReX_MPI FALSE)

add_subdirectory(amrex/)

add_executable(writePlts main.cpp)

target_link_libraries(writePlts AMReX::amrex_3d)

include_directories(${AMReX_INCLUDE_DIR})

include(GNUInstallDirs)
install(TARGETS writePlts
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
