cmake_minimum_required(VERSION 3.13)

project(LineArcOffsetDemo LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
# set(CMAKE_AUTORCC ON)
# set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Widgets Xml REQUIRED)

option(USE_CGAL "Enable CGAL Engine" OFF)
option(USE_OCCT "Enable OpenCASCADE Engine" OFF)
option(USE_OCE "Enable OpenCASCADE Engine" OFF)
option(USE_CAVC "Enable CavalierContours Engine" OFF)
option(USE_CLIPPER "Enable Clipper Engine" OFF)
option(USE_BOOST "Enable Boost Engine" OFF)
option(USE_GEOS "Enable GEOS Engine" OFF)

if(USE_CGAL)
    find_package(CGAL REQUIRED)
    add_compile_definitions(USING_CGAL)
    set(CGAL_SOURCES
        src/cgal/CGALWrapper_cpp.cpp
		src/cgal/CGALQt.cpp
		src/cgal/GeometryCGAL.cpp
		src/cgal/GeometryOperationsCGAL.cpp
    )
endif()

if(USE_OCCT)
    find_package(OpenCASCADE REQUIRED)
    add_compile_definitions(USING_OCCT)
    set(OCCT_SOURCES
        src/occt/GeometryOCCT.cpp
		src/occt/GeometryOperationsOCCT.cpp
    )
    set(OCCT_LIBS
        TKBO
		TKBool
		TKBRep
		TKGeomBase
		TKMath
		TKOffset
		TKTopAlgo
		TKDEIGES
		TKernel
		TKG3d
		TKShHealing
    )
endif()

if(USE_CAVC)
    find_package(OpenCASCADE REQUIRED)
    add_compile_definitions(USING_CAVC)
    set(CAVC_SOURCES
        src/cavc/CavCQt.cpp
		src/cavc/GeometryCavC.cpp
		src/cavc/GeometryOperationsCavC.cpp
    )
endif()

if(USE_CLIPPER)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(POLYCLIPPER REQUIRED polyclipping)
    add_compile_definitions(USING_CLIPPER)
    set(CLIPPER_SOURCES
        src/clipper/GeometryClipper.cpp
		src/clipper/GeometryOperationsClipper.cpp
    )
endif()

if(USE_BOOST)
    find_package(Boost REQUIRED)
    add_compile_definitions(USING_BOOST)
    set(BOOST_SOURCES
        src/boost/GeometryBoost.cpp
		src/boost/GeometryOperationsBoost.cpp
    )
endif()

if(USE_GEOS)
    find_package(geos REQUIRED)
    add_compile_definitions(USING_GEOS)
    set(GEOS_SOURCES
        src/geos/GeometryGEOS.cpp
		src/geos/GeometryOperationsGEOS.cpp
    )
endif()

add_compile_definitions(
    QT_DEPRECATED_WARNINGS
    QT_NO_FOREACH
    QT_NO_KEYWORDS
)

include_directories(
    $<$<BOOL:${USE_CAVC}>:deps/CavalierContours/include>
    $<$<BOOL:${USE_CLIPPER}>:${POLYCLIPPER_INCLUDE_DIRS}>
)

add_executable(${PROJECT_NAME}
    src/AleksFile.cpp
	src/Svg.cpp
	src/Geometry.cpp
	src/GeometryOperations.cpp
	src/GeometryQt.cpp
	src/GeometryScene.cpp
	src/GeometryView.cpp
    $<$<BOOL:${USE_CGAL}>:${CGAL_SOURCES}>
    $<$<BOOL:${USE_OCCT}>:${OCCT_SOURCES}>
    $<$<BOOL:${USE_CAVC}>:${CAVC_SOURCES}>
    $<$<BOOL:${USE_CLIPPER}>:${CLIPPER_SOURCES}>
    $<$<BOOL:${USE_BOOST}>:${BOOST_SOURCES}>
    $<$<BOOL:${USE_GEOS}>:${GEOS_SOURCES}>
	src/Actions.cpp
	src/MenuBar.cpp
	src/MainWindow.cpp
	src/main.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Widgets
    Qt5::Xml
    $<$<BOOL:${USE_CGAL}>:CGAL::CGAL>
    $<$<BOOL:${USE_OCCT}>:${OCCT_LIBS}>
    $<$<BOOL:${USE_CLIPPER}>:${POLYCLIPPER_LIBRARIES}>
    $<$<BOOL:${USE_GEOS}>:GEOS::geos>
)
