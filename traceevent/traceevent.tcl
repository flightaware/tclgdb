#!/usr/bin/env tclsh
#
# Convert a ```truss -d -s 1000 -p <pid>``` trace output into
# traceevent format.  You can view traceEvent format in chrome://tracing
#
#
namespace import ::tcl::mathop::*

package require Tclx

set ::last_level 0
set ::B_count 0

proc emitjson {ts level name args} {
	# Are we going up or down levels
	set level_change [- $level $::last_level]
	set ::last_level $level
	set src [join $args " "]
	# We only end as many levels as we started
	# the trace can start at any level, so track level with B_count
	while {$level_change <= 0 && $::B_count > 0} {
	    puts "{\"pid\":\"1\", \"tid\":\"1\", \"cat\":\"tcl\", \"ph\":\"E\", \"ts\":$ts },"
	    incr level_change
	    incr ::B_count -1
        }
	puts -nonewline "{\"pid\":\"1\", \"tid\":\"1\", \"cat\":\"tcl\", \"ph\":\"B\", \"ts\":$ts, \"name\":\"$name\",\"args\":{ \"level\": \"$level\", \"s\":\"$args\"} }"
	incr ::B_count
}

proc getargs {line} {
	set argsindex [+ [string last "@@" $line] 2]
	set commaindex [- [string last "," $line] 2]
	set v [string range $line $argsindex $commaindex]
	# TODO for some reason { and } do not work with backslash
	set v [string map {\" \\\" \\ \\\\ \{ ( \} )} $v]
	return $v
}

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
	# convert times in seconds from truss into nanoseconds
	set clock [* 1000000000 [lindex $w 0]]
	set level [lindex $w 1]
	if {$level == "write(-1,"} {
	    set level [lindex $w 2]
	    set level [string range $level 1 2]
	} else {
	    set level [string range $level 10 11]
        }
	set level [string trim $level ":"]
	set procname [getprocname $line]
	set args [getargs $line]
	set r [list $clock $level $procname $args]
	return $r
}

proc readfile {filename maxRows} {
	set second_line false
	set fp [open $filename "r"]
	while {![eof $fp] && $maxRows >= 0} {
		set s [gets $fp]
		if {[string first "write(-1," $s] != -1} { 
			set v [parseline $s]
			if {[llength $v] > 0} {
				if {$second_line} {
					puts ","
				} else {
					set second_line true
				}
				emitjson {*}$v
			}
		}
		incr maxRows -1
	}
	close $fp
}

lassign $argv filename maxRows
puts "{\"traceEvents\": \["
readfile $filename $maxRows
puts "\],\n\"meta_user\":\"tcl\" }"
