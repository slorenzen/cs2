project(cs2)
cmake_minimum_required(VERSION 2.6)
add_definitions("-std=c++0x")
add_definitions("-pedantic -Wall -Wextra -Wdouble-promotion -Winit-self -Wmissing-include-dirs -Wswitch-default -Wswitch-enum -Wfloat-equal -Wundef -Wshadow -Wcast-qual -Wcast-align -Wconversion -Wlogical-op -Wmissing-declarations -Wpacked -Wredundant-decls -Weffc++ -Wnon-virtual-dtor")
find_package(Qt4 REQUIRED)

include_directories(${QT_INCLUDES} ${CMAKE_CURRENT_BINARY_DIR})

set(CMAKE_AUTOMOC TRUE)

set(cs2_SRCS delayedspinbox.cpp minpack.cpp foupack.cpp nopack.cpp polypack.cpp scaledraw24.cpp datapicker.cpp tslist.cpp lumierror.cpp lumifile.cpp timeseries.cpp cs2.cpp main.cpp)

FIND_LIBRARY (GSL_LIB  gsl REQUIRED)
FIND_LIBRARY (GSLCBLAS_LIB  gslcblas REQUIRED)
FIND_LIBRARY (QWT_LIB qwt REQUIRED)

#ADD_EXECUTABLE (cs2 WIN32 ${cs2_SRCS} )
ADD_EXECUTABLE (cs2 ${cs2_SRCS} )

TARGET_LINK_LIBRARIES (cs2 ${QWT_LIB} ${QT_QTGUI_LIBRARY} ${QT_QTCORE_LIBRARY} ${QT_QTSVG_LIBRARY}  ${GSL_LIB} ${GSLCBLAS_LIB})

## for WINDOWS: cmake -DCMAKE_TOOLCHAIN_FILE=../../win.cmake ~/projects/cs2