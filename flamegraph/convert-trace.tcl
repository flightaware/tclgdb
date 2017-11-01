#!/usr/bin/env tclsh
#
namespace import ::tcl::mathop::*

proc getprocname {line} {
	set procindex [string first "@@ proc=" $line]
	if {$procindex > 0} {
		set procindex [+ $procindex 8]
		set commaindex [string first "," $line $procindex]
		if {$commaindex == -1} {
			set commaindex [+ $procindex 32]
			set commaindex [- $commandindex 1]
			set procname [string range $line $procindex $commaindex]
			return $procname
		}
		set w [split $line " "]
		if {[llength $w] >= 3} {
			return [lindex $w 2]
		}
	}
	return "unknown"
}

proc parseline {line} {
	set w [split $line " "]
	set clock [lindex $w 0]
	set level [lindex $w 1]
	set level [string range $level 10 11]
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
	set name [lindex $l 2]
	if {$name == "$::entryname"} {
		append name $::gennum
		incr ::gennum
	}
	if {[llength $path] == 0 || $lastlevel <= 0} {
		# start empty path
		return [list $name]
	}
	set level [lindex $l 1]
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
