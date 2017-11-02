#!/usr/bin/env tclsh
#
namespace import ::tcl::mathop::*

package require Tclx

#cmdtrace on

proc getprocname {line} {
	set procindex [string first "@@ proc=" $line]
	if {$procindex > 0} {
		set procindex [+ $procindex 8]
		set c [string first "," $line $procindex]
		if {$c != -1} {
			set c [- $c 1]
			set procname [string range $line $procindex $c]
			return $procname
		}
	}
	return "unknown"
}

proc parseline {line} {
	set w [split $line " "]
	set clock [lindex $w 0]
	set level [lindex $w 1]
	if {$level == "write(-1,"} {
	    set level [lindex $w 2]
	    set level [string range $level 1 2]
	} else {
	    set level [string range $level 10 11]
        }
	set level [string trim $level ":"]
	set procname [getprocname $line]
	set r [list $clock $level $procname]
	return $r
}

proc readfile {filename} {
	set l {}
	set fp [open $filename "r"]
	while {![eof $fp]} {
		set s [gets $fp]
		if {[string first "write(-1," $s] != -1} { 
			set v [parseline $s]
			if {[llength $v] > 0} {
				lappend l $v
			}
		}
	}
	close $fp
	return $l
}

# By changing the name of the function
# we can get individual displays of each
# iteration

set gennum 0
set entryname "no-entry-name"

proc makepath {path l lastlevel} {
	set name [string trim [lindex $l 2]]
	if {[string length $name] == 0} {
		set name "{noname}"
	}
	if {$name == "$::entryname"} {
		append name $::gennum
		incr ::gennum
	}
	set level [lindex $l 1]
	if {[llength $path] == 0 || $lastlevel <= 0} {
		set p {}
		while {$level >= 0} {
			lappend p "{lvl}"
			incr level -1
		}
		# start empty path
		lappend p $name
		return $p
	}
	if {$lastlevel < $level} {
		# new frame push on to list
		return [lappend path $name]
	}	
	if {$lastlevel > $level} {
		# pop the frame
		set n [- [llength $path] [+ 1 [- $lastlevel $level]]]
		set np [lrange $path 0 $n]
		if {[llength $np] == 1} {
		}
		return $np
	}
	set n [- [llength $path] 2]
	set t [lrange $path 0 $n]
	return [lappend t $name]
}

lassign $argv filename entryname
set points [readfile $filename]

set n [llength $points]
set path [list]
set lastlevel -1
for {set ii 0} {$ii < $n} {incr ii} {
	set l [lindex $points $ii]
	if {$ii < $n - 1} {
		set path [makepath $path $l $lastlevel]
		set lastlevel [lindex $l 1]
		# there is a next point
		set now [lindex $l 0]
		set later [lindex [lindex $points [+ $ii 1]] 0]
		if {$now != "" && $later != ""} {
			set delta [- [* $later 1000000000] [* $now 1000000000]]
			puts "[join $path ";"] [expr {round($delta)}]"
		} else {
			set ii $n
		}
	}
}
