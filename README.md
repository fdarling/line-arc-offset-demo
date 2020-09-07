# LineArcOffsetDemo

A demo program written with C++/Qt to show offsetting of shapes comprised of line and arc segments. It also can do boolean operations on these shapes.

It currently has two backends supporting arc segments: CGAL (used by OpenSCAD), and Open CASCADE (used by FreeCAD).

And three backends that approximate arcs with line segments: Clipper, Boost, and GEOS.

## Screenshots

[![offset shapes](https://i.imgur.com/bPJmiXwm.png "offset shapes")](https://i.imgur.com/bPJmiXw.png)
[![combined shapes](https://i.imgur.com/dlGJi19m.png "combined shapes")](https://i.imgur.com/dlGJi19.png)
[![overlapping shapes](https://i.imgur.com/t0Ig0Jzm.png "overlapping shapes")](https://i.imgur.com/t0Ig0Jz.png)

## Dependencies

Mandatory dependencies:

* [Qt](https://www.qt.io/) 5 (widgets and xml modules)

Optional dependencies for different engines:

* [CGAL](https://www.cgal.org/) 4.11.2
* [Open CASCADE Technology](https://www.opencascade.com/content/latest-release) 7.4.0
* [Open CASCADE Community Edition](https://github.com/tpaviot/oce) 0.18.2 (forked from Open CASCADE Technology 6.9.0)
* [Clipper](http://www.angusj.com/delphi/clipper.php) 6.4.2
* [Boost](https://www.boost.org/) 1.65.1
* [GEOS](https://trac.osgeo.org/geos) 3.6.2

NOTE: the version numbers are for reference, other versions may work too.

The relevant development packages to install on Ubuntu 18.04 (bionic) are:

* qt5-default
* libcgal-dev
* libocct-modeling-algorithms-dev
* liboce-modeling-dev
* libpolyclipping-dev
* libboost-dev
* libgeos++-dev

NOTE: libocct is only available on newer Debian/Ubuntu distributions, so you'll either have to use liboce instead, or compile/install OCCT manually (see the instructions further down).

## Open CACADE Technology (OCCT) vs. Open CASCADE Community Edition (OCE)

Open CASCADE Technology was forked at version 6.9.0 into a "Community Edition" that has a different version number scheme. The motivation was to more actively integrate user submitted patches. FreeCAD uses OCE and not official OCCT, and Debian/Ubuntu provide packages for OCE but not OCCT, making it more convenient to install.

https://en.wikipedia.org/wiki/Open_Cascade_Technology#Community_fork

## Building Open CASCADE Technology

Official Open CASCADE Technology (not the Community Edition) packages are not available in Ubuntu's package repositories. You must instead compile it from the source.

For Debian 9 (stretch) and likely Ubuntu 18.04 (bionic), the following dependencies need to be installed to build Open CACADE 7.4.0:

```
sudo apt-get install cmake tk-dev libxmu-dev libxi-dev
```

The official website (linked earlier) wants you to log in with an account before allowing you to download any files. There is however a GitHub mirror with the sourcecode packages: https://github.com/tpaviot/oce/releases

Here are example commands for downloading, unpacking, compiling, and installing Open CASCADE 7.4.0:

```
cd ~/Downloads
wget -c https://github.com/tpaviot/oce/releases/download/official-upstream-packages/opencascade-7.4.0.tgz
tar xf opencascade-7.4.0.tgz
cd opencascade-7.4.0
mkdir build
cd build
cmake ..
make
make install
```

## Configuring

You will want to edit the `linearcoffsetdemo.pro` file and choose which goemetry backends to use by commenting or uncommenting the appropriate lines. It is fine to have multiple engines enabled at the same time, the active engine can be chosen by passing the `--engine` command line option with the appropriate engine name.

This example shows using only OCE:

```
# GEOMETRY_ENGINES += cgal
# GEOMETRY_ENGINES += occt
GEOMETRY_ENGINES += oce
# GEOMETRY_ENGINES += clipper
# GEOMETRY_ENGINES += boost
# GEOMETRY_ENGINES += geos
```

This example shows using both CGAL and GEOS:

```
GEOMETRY_ENGINES += cgal
# GEOMETRY_ENGINES += occt
# GEOMETRY_ENGINES += oce
# GEOMETRY_ENGINES += clipper
# GEOMETRY_ENGINES += boost
GEOMETRY_ENGINES += geos
```

NOTE: both OCCT and OCE can be enabled at the same time, but only OCCT will actually be used (it overrides the other).

You may also need to modify the appropriate `LIBS` and `QXX_CFLAGS` lines for CGAL and OCCT/OCE depending on your platform.

## Compiling

On Linux, run the following commands in the top-level directory with the `linearcoffsetdemo.pro` file:

```
qmake
make
```

## Running

On Linux, you can launch the built `LineArcOffsetDemo` executable that will be put in the same directory as the `linearcoffsetdemo.pro` file:

```
./LineArcOffsetDemo
```

An example of manually specifying the engine to use (useful if multiple engines are enabled):

```
./LineArcOffsetDemo --engine geos
```
