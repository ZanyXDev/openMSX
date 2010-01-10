package provide cheatfinder 0.5

set_help_text findcheat \
{Cheat finder version 0.5

Welcome to the openMSX cheat finder. Please visit
  http://forum.vampier.net/viewtopic.php?t=32
for a quick tutorial

Usage:
  findcheat [-start] [-max n] [expression]
     -start     :  restart search, discard previously found addresses
     -max n     :  show max n results
     expression :  TODO

Examples:
  findcheat 42                 search for specific value
  findcheat bigger             search for values that have become bigger
  findcheat new == (2 * old)   search for values that have doubled
  findcheat new == (old - 1)   search for values that have decreased by 1
  findcheat                    repeat the previous found addresses
  findcheat -start new < 10    restart and search for values less then 10
  findcheat -max 40 smaller    search for smaller values, show max 40 results
}

namespace eval cheat_finder {


set_tabcompletion_proc findcheat [namespace code tab_cheat_type]
proc tab_cheat_type { args } {
	variable monitors
	set result [array names monitors]
	lappend result "-start"
	lappend result "-bigger"
	lappend result "-smaller"
	lappend result "-more"
	lappend result "-less"
	lappend result "-notequal"
	lappend result "-equal"
	lappend result "-loe"	
	lappend result "-moe"	
	lappend result "-max"
}

#set maximum to display cheats
variable max_num_results 15
variable mem
variable translate

# Restart cheat finder.
proc start {} {
	variable mem
	
	set mymem [debug read_block memory 0 0x10000]
	binary scan $mymem c* values
	set addr 0
	foreach val $values {
		set mem($addr) $val
		incr addr
	}
}

# Helper function to do the actual search.
# Returns a list of triplets (addr, old, new)
proc search { expression } {
	variable mem

	set result [list]
	foreach {addr old} [array get mem] {
		set new [debug read memory $addr]
		if [expr $expression] {
			set mem($addr) $new
			lappend result [list $addr $old $new]
		} else {
			unset mem($addr)
		}
	}
	return $result
}

# main routine
proc findcheat { args } {
	variable mem
	variable max_num_results
	variable translate

	# create mem array
	if ![array exists mem] start

	# build translation array for convenience expressions
	if ![array exists translate] {
		# TODO add more here
		set translate()         "true"

		set translate(bigger)   "new > old"
		set translate(smaller)  "new < old"

		set translate(more)     "new > old"
		set translate(less)     "new < old"

		set translate(notequal) "new != old"
		set translate(equal)    "new == old"

		set translate(loe)      "new <= old"
		set translate(moe)      "new >= old"
	}

	# parse options
	while (1) {
		switch -- [lindex $args 0] {
		"-max" {
			  set max_num_results  [lindex $args 1]
			  set args [lrange $args 2 end]
		}
		"-start" {
			start
			set args [lrange $args 1 end]
		}
		"default" break
		}
	}

	# all remaining arguments form the expression
	set expression [join $args]

	if [info exists translate($expression)] {
		# translate a convenience expression into a real expression
		set expression $translate($expression)
	} elseif [string is integer $expression] {
		# search for a specific value
		set expression "new == $expression"
	}

	# prefix 'old', 'new' and 'addr' with '$'
	set expression [string map {old $old new $new addr $addr} $expression]

	# search memory
	set result [search $expression]

	# display the result
	set num [llength $result]
	if {$num == 0} {
		return "No results left"
	} elseif {$num <= $max_num_results} {
		set output ""
		set sorted [lsort -integer -index 0 $result]
		foreach {addr old new} [join $sorted] {
			append output [format "0x%04X : %d -> %d\n" $addr $old $new]
		}
		return $output
	} else {
		return "$num results found -> Maximum result to display set to $max_num_results "
	}
}

namespace export findcheat

} ;# namespace cheat_finder

namespace import cheat_finder::*
