tclgdb - helpers for using gdb and tcl together for debugging
===

tclgdb is a Tcl cmdtrace handler in C.  This allows one to set a breakpoint on
tclgdb_cmdstep and step the Tcl interpreter while in gdb.

[How to install](INSTALL.md)

Using with truss or strace
----------------
Add the command trace to your TCL program and start it. This will have the overhead of the Tclx cmdtrace. Your program will be slower.
```
package require tclgdb
::tclgdb::tclgdb
```
Inside your TCL process the trace is writing to file descriptor -1.  That is an intensional error.  But we can get the trace data out of the running
process with ```truss``` on BSD or ```strace``` on Linux.

You only need the ```write(-1,``` trace from the truss.  An example looks like this:

```
0.000078469 write(-1,"15: @@ proc=my_scan,pLn=0,path=unknown @@ incr count",67) ERR#9 'Bad file descriptor'
0.000199850 write(-1,"15: @@ proc=my_scan,pLn=0,path=unknown @@ compile_user $trigger(user_id)",111) ERR#9 'Bad file descriptor'
0.000271614 write(-1,"16: @@ proc=compile_user,pLn=0,path=unknown @@ if {[info exists ::cache($userID)]} {\n\t\treturn $::cache($userID)\n\t}",152) ERR#9 'Bad file descriptor'
```

For truss you should add "-d" for timestamps. Also add "-s <N>" to expand the reported arguments. Note that truss writes on stderr not stdout.
```
truss -d -s 500 -p <pid> 2>truss.out
```
You can control-C the truss when you think that you have collected enough data.

Once you have generated the truss output, then you can convert that file into a viewable heatmap or interactive viewer.
See [viewing with traceevent](traceevent/README.md).  Or you can simply use the output from truss.

gdb debugger Usage
------------------
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

Future Work
--------
One goal will be to incorporate the Unicorn engine, http://www.unicorn-engine.org.  With Unicorn's virtual machine we can run diagnostic Tcl without affecting the Tcl interpreter.
For example, if you call ```print Tcl_GetString(argv[2])``` then one may cause a Tcl alloc or string creation in the representation.
Using the Unicorn interpreter, we can call Tcl interpreter code without changing the debug target.


