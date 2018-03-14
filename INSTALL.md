Building
--------
In order to see the cmdline variable in tclgdb_cmdstep you should set CFLAGS when
configuring with autoconf. 
```
CFLAGS="-g" ./configure --with-tcl=/usr/local/lib/tcl8.6
```

Building with access to TCL internals
-------------------------------------
If you have the source build of TCL you can get more information by hacking into the TCL
internals.
THIS MIGHT CRASH YOUR PROGRAM.

```
# if you git pull the TCL source
CFLAGS="-DHAVE_TCLINT_H -I$HOME/git/tcl/generic -I$HOME/git/tcl/unix" ./configure && make clean && make && sudo make install
```

```
# if you build TCL from FreeBSD ports
CFLAGS="-DHAVE_TCLINT_H -I/usr/ports/lang/tcl86/work/tcl8.6.7/generic -I/usr/ports/lang/tcl86/work/tcl8.6.7/unix" ./configure && make clean && make && sudo make install
```

Installing
----------
Installation is a typical TEA autoconf style.
```
mkdir -p config
autoreconf -vi
CFLAGS="-g" ./configure
make 
sudo make install
```
