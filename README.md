# LineArcOffsetDemo

A demo program written with C++/Qt to show offsetting of shapes comprised of line and arc segments. It also can perform boolean operations on these shapes.

It currently has three backends that natively support arc segments: CGAL (used by OpenSCAD), and Open CASCADE (used by FreeCAD), and CavalierContours (lesser known, mostly abandoned for a Rust rewrite).

There are five backends that approximate arcs with line segments: Clipper, Clipper2, Boost, and GEOS.

## Screenshots

[![offset shapes](https://i.imgur.com/bPJmiXwm.png "offset shapes")](https://i.imgur.com/bPJmiXw.png)
[![combined shapes](https://i.imgur.com/dlGJi19m.png "combined shapes")](https://i.imgur.com/dlGJi19.png)
[![overlapping shapes](https://i.imgur.com/t0Ig0Jzm.png "overlapping shapes")](https://i.imgur.com/t0Ig0Jz.png)

## Dependencies

Mandatory dependencies:

* [CMake](https://cmake.org/) v3.31 (v3.13+ required)
* [Qt](https://www.qt.io/) v5.15 (widgets and xml modules)

Optional dependencies for different engines:

* [CGAL](https://www.cgal.org/) v6.0.1
* [Open CASCADE Technology](https://dev.opencascade.org/release) v7.8.1
* [CavalierContours](https://github.com/jbuckmccready/CavalierContours) CavalierContours (included as git sub-module)
* [Clipper](http://www.angusj.com/delphi/clipper.php) v6.4.2
* [Clipper2](https://www.angusj.com/clipper2/Docs/Overview.htm) v2.0.1 (included via CMake FetchContent)
* [Boost](https://www.boost.org/) v1.83.0
* [GEOS](https://trac.osgeo.org/geos) v3.13.1

NOTE: the version numbers are for reference, other versions may work too.

The relevant development packages to install on Debian 13 (trixie) are:

* `qtbase5-dev`
* `libcgal-dev`
* `libocct-modeling-algorithms-dev` and `libocct-modeling-data-dev` and `libocct-data-exchange-dev`
* `libpolyclipping-dev`
* `libboost-dev`
* `libgeos++-dev`

## Compiling

Run the following commands in the top-level directory with the `CMakeLists.txt` file:

```
mkdir build
cd build
cmake .. -DUSE_GEOS=ON
cmake --build .
```

You can add one or more `-DUSE_xxx=ON` to enable various engines as backends:

* `-DUSE_CGAL=ON` for CGAL
* `-DUSE_OCCT=ON` for OpenCASCADE
* `-DUSE_CAVC=ON` for CavalierContours
* `-DUSE_CLIPPER=ON` for Clipper
* `-DUSE_CLIPPER2=ON` for Clipper2
* `-DUSE_BOOST=ON` for Boost
* `-DUSE_GEOS=ON` for GEOS

## Running

You can launch the built `LineArcOffsetDemo` executable that will be put in the build directory:

```
./LineArcOffsetDemo
```

An example of manually specifying the engine to use (useful if multiple engines are enabled):

```
./LineArcOffsetDemo --engine geos
```

Here are the various options:

* `--engine cgal` for CGAL
* `--engine occt` for OpenCASCADE
* `--engine cavc` for CavalierContours
* `--engine clipper` for Clipper
* `--engine clipper2` for Clipper2
* `--engine boost` for Boost
* `--engine geos` for GEOS
