tclgdb - helpers for using gdb and tcl together for debugging
===

tclgdb is a Tcl cmdtrace handler in C.  This allows one to set a breakpoint on
tclgdb_cmdstep and step the Tcl interpreter while in gdb.

Usage
-----
```
package require tclgdb

::tclgdb::tclgdb
```
Then in gdb set a breakpoint on tclgdb_cmdstep.

Example
-------
```
$ gdb /usr/local/bin/tclsh8.6
GNU gdb 6.1.1 [FreeBSD]
Copyright 2004 Free Software Foundation, Inc.
GDB is free software, covered by the GNU General Public License, and you are
welcome to change it and/or distribute copies of it under certain conditions.
Type "show copying" to see the conditions.
There is absolutely no warranty for GDB.  Type "show warranty" for details.
This GDB was configured as "amd64-marcel-freebsd"...
(gdb) break tclgdb_cmdstep
Function "tclgdb_cmdstep" not defined.
Make breakpoint pending on future shared library load? (y or [n]) y
Breakpoint 1 (tclgdb_cmdstep) pending.
(gdb) run tests/simple.tcl
Starting program: /usr/local/bin/tclsh8.6 tests/simple.tcl
[New LWP 100889]
[New Thread 802006400 (LWP 100889/tclsh8.6)]
Breakpoint 2 at 0x8018856b0: file ./generic/tclgdb.c, line 39.
Pending breakpoint "tclgdb_cmdstep" resolved
[Switching to Thread 802006400 (LWP 100889/tclsh8.6)]

Breakpoint 2, tclgdb_cmdstep (clientData=0x801a85f80, interp=0x802031010, level=1, command=0x80203e460 "while {1} {\n\tafter 500\n    set val1 0\n\tset done $val1\n}", 
    cmdProc=0x800a4ab20 <TclInvokeObjectCommand>, cmdClientData=0x802031a10, argc=3, argv=0x80203e4a0) at ./generic/tclgdb.c:39
39		strncpy(buffer, s, sizeof(buffer)-1);
(gdb) p command
$1 = 0x80203e460 "while {1} {\n\tafter 500\n    set val1 0\n\tset done $val1\n}"
(gdb) cont
Continuing.

Breakpoint 2, tclgdb_cmdstep (clientData=0x801a85f80, interp=0x802031010, level=2, command=0x80203e4f0 "after 500", cmdProc=0x800a4ab20 <TclInvokeObjectCommand>, 
    cmdClientData=0x802031d10, argc=2, argv=0x80203e510) at ./generic/tclgdb.c:39
39		strncpy(buffer, s, sizeof(buffer)-1);
(gdb) p command
$2 = 0x80203e4f0 "after 500"
(gdb) cont
Continuing.

Breakpoint 2, tclgdb_cmdstep (clientData=0x801a85f80, interp=0x802031010, level=2, command=0x80203e4f0 "set val1 0", cmdProc=0x800a4ab20 <TclInvokeObjectCommand>, 
    cmdClientData=0x802026710, argc=3, argv=0x80203e510) at ./generic/tclgdb.c:39
39		strncpy(buffer, s, sizeof(buffer)-1);
(gdb) p command
$3 = 0x80203e4f0 "set val1 0"
(gdb) cont
Continuing.

Breakpoint 2, tclgdb_cmdstep (clientData=0x801a85f80, interp=0x802031010, level=2, command=0x80203e4f0 "set done $val1", cmdProc=0x800a4ab20 <TclInvokeObjectCommand>, 
    cmdClientData=0x802026710, argc=3, argv=0x80203e510) at ./generic/tclgdb.c:39
39		strncpy(buffer, s, sizeof(buffer)-1);
(gdb) p command
$4 = 0x80203e4f0 "set done $val1"
```

Building
--------
In order to see the cmdline variable in tclgdb_cmdstep you should set CFLAGS when
configuring with autoconf. 
```
CFLAGS="-g" ./configure --with-tcl=/usr/local/lib/tcl8.6
```

Future Work
--------
One goal will be to incorporate the Unicorn engine, http://www.unicorn-engine.org.  With Unicorn's virtual machine we can run diagnostic Tcl without affecting the Tcl interpreter.
For example, if you call ```print Tcl_GetString(argv[2])``` then one may cause a Tcl alloc or string creation in the representation.
Using the Unicorn interpreter, we can call Tcl interpreter code without changing the debug target.


