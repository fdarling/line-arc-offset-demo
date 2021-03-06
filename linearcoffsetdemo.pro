QT += widgets xml
CONFIG += object_parallel_to_source
# CONFIG += debug

OBJECTS_DIR=build/obj
MOC_DIR=build/moc

# WARNING: precompiling the header seems to be slower!
# CONFIG += precompile_header
# PRECOMPILED_HEADER = src/cgal/CGALWrapper.h

# GEOMETRY_ENGINES += cgal
# GEOMETRY_ENGINES += occt
GEOMETRY_ENGINES += oce
# GEOMETRY_ENGINES += cavc
# GEOMETRY_ENGINES += clipper
# GEOMETRY_ENGINES += boost
# GEOMETRY_ENGINES += geos

contains(GEOMETRY_ENGINES, cgal) {
	HEADERS += \
		src/cgal/CGALWrapper.h \
		src/cgal/CGALQt.h \
		src/cgal/GeometryCGAL.h \
		src/cgal/GeometryOperationsCGAL.h
	SOURCES += \
		src/cgal/CGALWrapper_cpp.cpp \
		src/cgal/CGALQt.cpp \
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
contains(GEOMETRY_ENGINES, occt)|contains(GEOMETRY_ENGINES, oce) {
	HEADERS += \
		src/occt/GeometryOCCT.h \
		src/occt/GeometryOperationsOCCT.h
	SOURCES += \
		src/occt/GeometryOCCT.cpp \
		src/occt/GeometryOperationsOCCT.cpp
	contains(GEOMETRY_ENGINES, occt) {
		LIBS += -L/usr/local/lib
		INCLUDEPATH += /usr/local/include/opencascade
	}
	contains(GEOMETRY_ENGINES, oce) {
		# LIBS += -L/usr/lib/x86_64-linux-gnu/
		INCLUDEPATH += /usr/include/oce
	}
	LIBS += \
		-lTKBO \
		-lTKBool \
		-lTKBRep \
		-lTKGeomBase \
		-lTKMath \
		-lTKOffset \
		-lTKTopAlgo \
		-lTKIGES \
		-lTKernel \
		-lTKG3d \
		-lTKShHealing
	DEFINES += USING_OCCT
}
contains(GEOMETRY_ENGINES, cavc) {
	HEADERS += \
		src/cavc/CavCQt.h \
		src/cavc/GeometryCavC.h \
		src/cavc/GeometryOperationsCavC.h
	SOURCES += \
		src/cavc/CavCQt.cpp \
		src/cavc/GeometryCavC.cpp \
		src/cavc/GeometryOperationsCavC.cpp
	INCLUDEPATH += deps/CavalierContours/include
	DEFINES += USING_CAVC
}
contains(GEOMETRY_ENGINES, clipper) {
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
contains(GEOMETRY_ENGINES, boost) {
	HEADERS += \
		src/boost/GeometryBoost.h \
		src/boost/GeometryOperationsBoost.h
	SOURCES += \
		src/boost/GeometryBoost.cpp \
		src/boost/GeometryOperationsBoost.cpp
	DEFINES += USING_BOOST
}
contains(GEOMETRY_ENGINES, geos) {
	HEADERS += \
		src/geos/GeometryGEOS.h \
		src/geos/GeometryOperationsGEOS.h
	SOURCES += \
		src/geos/GeometryGEOS.cpp \
		src/geos/GeometryOperationsGEOS.cpp
	LIBS += `geos-config --libs`
	QMAKE_CXXFLAGS += `geos-config --cflags`
	DEFINES += USING_GEOS
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
	src/Actions.h \
	src/MenuBar.h \
	src/MainWindow.h

SOURCES += \
	src/AleksFile.cpp \
	src/Svg.cpp \
	src/Geometry.cpp \
	src/GeometryOperations.cpp \
	src/GeometryQt.cpp \
	src/GeometryScene.cpp \
	src/GeometryView.cpp \
	src/Actions.cpp \
	src/MenuBar.cpp \
	src/MainWindow.cpp \
	src/main.cpp
