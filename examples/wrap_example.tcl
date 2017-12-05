package require tclgdb

proc addem {x y} {
    expr {$x + $y}
}

proc wrap_addem {x y} {
    ::tclgdb::wrap addem $x $y 1
}

proc multiply {x y} {
    if {$x == 0} {
	return 0;
    } else {
       if {$x == 1} {
           return $y
       }
       wrap_addem $y [wrap_multiply [wrap_addem $x -1] $y]
    }
}

proc wrap_multiply {x y} {
    ::tclgdb::wrap multiply $x $y 2
}

for {set i 0} {$i < 1000000} {incr i} {
   set v [wrap_addem 42 532]
}
puts $v
