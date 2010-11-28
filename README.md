AsbestOS Loader
===============

This is a simple alternative to Hermes' AsbestOS loader, without any
dependency on his PSGroove payload. The default stage2 used was modified by
him.

Only use this with 3.41 retail, and any payload that supports peek/poke.


Installation
------------

Place "kernel.bin" in the data/ folder of the project.

kernel.bin can be downloaded
[here](http://marcansoft.com/transf/dtbImage-20101020.bin)
([Instructions](http://marcansoft.com/blog/2010/10/asbestos-running-linux-as-gameos/))

Just run *make* to build Hermes' stage2 and the self. Then *make run* it
straight to ps3load.
