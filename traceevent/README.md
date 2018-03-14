Convert truss data into Googles Trace Event Format
==================================================
Use the trace viewer in Chrome.  Open the URL chrome://tracing and "load" Trace Event formatted
JSON data.

https://docs.google.com/document/d/1CvAClvFfyA5R-PhYUmn5OOQtYMH4h6I0nSsKchNAySU/edit#
https://github.com/catapult-project/catapult/blob/master/tracing/docs/getting-started.md

Inspired by this blog post https://aras-p.info/blog/2017/01/23/Chrome-Tracing-as-Profiler-Frontend/

The TCL script traceevent.tcl converts the truss output from "tclgdb" to Trace Event formatted JSON.

Usage
```
traceevent.tcl <truss output> <line count>
```
The size of the trace can limit how much data the Chrome viewer can load. This will be dependent
on the specific trace.

You can gzip the json for the trace viewer in Chrome. For example,

```
$ ./traceevent.tcl ~/truss.out 250000 | gzip >traceevent.json.gz
```
