Visualize the data as a flamegraph
==================================

If you have a folded stack trace you can make a FlameGraph with several tools.
Here are instructions on a d3 solution for the browser. It is nice because it's interactive.

d3-flame-graph
==============
The index.html contains the cdn URLs for https://github.com/spiermar/d3-flame-graph.
You need to provide the data.json locally and a web server.

Make data.json
==============
First you need to convert the raw truss or strace output into a flamegraph folded stack.
Then you can convert the folded stack into a visualization. I am converting the folded stack
to JSON with burn, https://github.com/spiermar/burn.git

```
convert-trace.tcl <raw-trace-file> >folded-trace
~/git/burn/burn convert --type=folded foo >data.json
python -m SimpleHTTPServer 8078
open http://localhost:8078
```
