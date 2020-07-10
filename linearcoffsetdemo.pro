QT += widgets xml
# CONFIG += debug

# WARNING: precompiling the header seems to be slower!
# CONFIG += precompile_header
# PRECOMPILED_HEADER = src/cgal/CGALWrapper.h

# GEOMETRY_ENGINES += cgal
GEOMETRY_ENGINES += occt
# GEOMETRY_ENGINES += clipper

contains(GEOMETRY_ENGINES, cgal) {
	HEADERS += \
		src/cgal/CGALWrapper.h \
		src/cgal/GeometryCGAL.h \
		src/cgal/GeometryOperationsCGAL.h
	SOURCES += \
		src/cgal/CGALWrapper_cpp.cpp \
		src/cgal/GeometryCGAL.cpp \
		src/cgal/GeometryOperationsCGAL.cpp
	LIBS += \
		-lCGAL \
		-lCGAL_Core \
		-lmpfr \
		-lgmp
	QMAKE_CXXFLAGS += \
		-std=c++1z
	DEFINES += USING_CGAL
	# to avoid errors when running under valgrind:
	DEFINES += CGAL_DISABLE_ROUNDING_MATH_CHECK=ON
}
contains(GEOMETRY_ENGINES, occt) {
	HEADERS += \
		src/occt/GeometryOCCT.h \
		src/occt/GeometryOperationsOCCT.h
	SOURCES += \
		src/occt/GeometryOCCT.cpp \
		src/occt/GeometryOperationsOCCT.cpp
	LIBS += \
		-std=c++11 \
		-L/usr/local/lib \
		-lTKBO \
		-lTKBRep \
		-lTKGeomBase \
		-lTKGeomAlgo \
		-lTKMath \
		-lTKOffset \
		-lTKPrim \
		-lTKService \
		-lTKTopAlgo \
		-lTKSTL \
		-lTKIGES \
		-lTKHLR \
		-lTKernel \
		-lTKG2d \
		-lTKG3d \
		-lTKBRep \
		-lTKShHealing
	QMAKE_CXXFLAGS += \
		-I/usr/local/include/opencascade \
		-std=c++1z
	DEFINES += USING_OCCT
}
{
	HEADERS += \
		src/clipper/GeometryClipper.h \
		src/clipper/GeometryOperationsClipper.h
	SOURCES += \
		src/clipper/GeometryClipper.cpp \
		src/clipper/GeometryOperationsClipper.cpp
	LIBS += `pkg-config --libs polyclipping`
	QMAKE_CXXFLAGS += `pkg-config --cflags polyclipping`
	DEFINES += USING_CLIPPER
}

TARGET = LineArcOffsetDemo

HEADERS += \
	src/AleksFile.h \
	src/Svg.h \
	src/Geometry.h \
	src/GeometryOperations.h \
	src/GeometryQt.h \
	src/GeometryScene.h \
	src/GeometryView.h \
	src/MainWindow.h

SOURCES += \
	src/AleksFile.cpp \
	src/Svg.cpp \
	src/Geometry.cpp \
	src/GeometryOperations.cpp \
	src/GeometryQt.cpp \
	src/GeometryScene.cpp \
	src/GeometryView.cpp \
	src/MainWindow.cpp \
	src/main.cpp
