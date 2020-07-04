# LineArcOffsetDemo

A demo program written with C++/Qt to show offsetting of shapes comprised of line and arc segments. It also can do boolean operations on these shapes.

It currently has two backends: CGAL (used by OpenSCAD) and Open CASCADE Technology (used by FreeCAD).

## Dependencies

* [Qt](https://www.qt.io/) 5 (widgets and xml modules)
* [CGAL](https://www.cgal.org/) 4.11.2 (older versions may work too)
* [Open CASCADE Technology](https://www.opencascade.com/content/latest-release) 7.4.0 (older versions may work too, not sure about the "Community Edition"...)

The relevant development packages to install on Ubuntu 18.04 are:

* qt5-default
* libcgal-dev

## Compiling

On Linux, run the following commands in the top-level directory with the `linearcoffsetdemo.pro` file:

```
qmake
make
```

## Running

On Linux, you can launch the built `Servoterm` executable that will be put in the same directory as the `servoterm.pro` file:

```
./LineArcOffsetDemo
```
