Leaklog
=======

Leaklog is a leakage control system based on Regulation (EC) No 842/2006.
It logs findings and parameters of leakage checks, shows a history of the
development of parameters, compares them with nominal ones and calculates
the percentage of leakage.

### Supported platforms

* Mac OS X
* Windows
* Linux and various UNIX-based platforms

### Languages

* English
* Slovak

Installation
------------

Binaries are available for **Mac OS X** and **Windows** at
[SourceForge][SF].

[SF]: http://sourceforge.net/projects/leaklog/

To build Leaklog from source, you will need Qt 4.6 or a newer,
compatible version.

Use the `lrelease` tool to compile translations as follows:

	lrelease Leaklog.pro
	lrelease rc/i18n/Leaklog-i18n.ts

If you're running Mac OS X, type:

	qmake -spec macx-llvm -config release

Otherwise, the following will do:

	qmake -config release

And finally:

	make

(You can also use any other maketool, such as `mingw32-make`, `gmake`
or `nmake`, depending on your platform.)