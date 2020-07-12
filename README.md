# LineArcOffsetDemo

A demo program written with C++/Qt to show offsetting of shapes comprised of line and arc segments. It also can do boolean operations on these shapes.

It currently has three backends: CGAL (used by OpenSCAD), Open CASCADE Technology (used by FreeCAD), and Clipper (used by everybody else).

NOTE: Clipper doesn't support true circular arcs, they are approximated with line segments.

## Screenshots

[![offset shapes](https://i.imgur.com/bPJmiXwm.png "offset shapes")](https://i.imgur.com/bPJmiXw.png)
[![combined shapes](https://i.imgur.com/dlGJi19m.png "combined shapes")](https://i.imgur.com/dlGJi19.png)
[![overlapping shapes](https://i.imgur.com/t0Ig0Jzm.png "overlapping shapes")](https://i.imgur.com/t0Ig0Jz.png)

## Dependencies

Mandatory dependencies:

* [Qt](https://www.qt.io/) 5 (widgets and xml modules)

Optional dependencies depending on engines selected:

* [CGAL](https://www.cgal.org/) 4.11.2 (older versions may work too)
* [Open CASCADE Technology](https://www.opencascade.com/content/latest-release) 7.4.0 (older versions may work too, not sure about the "Community Edition"...)
* [Clipper](http://www.angusj.com/delphi/clipper.php) 6.4.2 (older versions may work too)
* [Boost](https://www.boost.org/) 1.65.1 (older versions may work too)

The relevant development packages to install on Ubuntu 18.04 (bionic) are:

* qt5-default
* libcgal-dev
* libpolyclipping-dev
* libboost-dev

For Debian 9 (stretch) and likely Ubuntu 18.04 (bionic), the following dependencies need to be installed to build Open CACADE 7.4.0:

```
sudo apt-get install cmake tk-dev libxmu-dev libxi-dev
```

## Configuring

You will want to edit the `linearoffsetdemo.pro` file and choose which goemetry backends to use by commenting or uncommenting the appropriate lines. ~~It is fine to have multiple engines enabled at the same time.~~ Currently the testbench code only likes having one enabled a time, this will change later.

This example shows using OCCT but not CGAL nor Clipper:

```
# GEOMETRY_ENGINES += cgal
GEOMETRY_ENGINES += occt
# GEOMETRY_ENGINES += clipper
# GEOMETRY_ENGINES += boost
```

You may also need to modify the `LIBS` and `QXX_CFLAGS` for CGAL and OCCT depending on your platform.

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
