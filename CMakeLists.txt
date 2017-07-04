project(csread)

find_package(HDF5 1.10.1 REQUIRED CXX)
include_directories(${HDF5_INCLUDE_DIRS})

add_library(csread STATIC csread/material.cpp)
target_link_libraries(
	csread
	${HDF5_CXX_LIBRARIES}
)
