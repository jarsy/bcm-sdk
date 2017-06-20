# $Id: makeDepSymTbl.tcl,v 1.0 2013/10/10 10:10:00 yunfengw Exp $
# $Copyright: (c) 2013 Broadcom Corp.
# All Rights Reserved.$
#
# dependent symbol table creation utility
#

namespace eval makeDepSymTbl {
    set nmCmd		"nm"
    set nmFlags		"-u"
    set symPrefixToIgnore ""
    set symPrefixToAdd	""
    set cpuType		""
    set fdOut           ""
}

##############################################################################
#
# depTblCreate - create an array of symbol for an object module
#
# This procedure creates an array of symbols (C code) based on a file symbol
# table.
#
##############################################################################

proc makeDepSymTbl::depTblCreate {modName} {
    variable symPrefixToIgnore
    variable symPrefixToAdd
    variable cpuType
    variable nmCmd
    variable nmFlags
    variable fdOut

    set symbolInfo ""
    set symList {}
    set nsyms 0

    # get all the global/local symbols from the object module

    set symbolInfo [eval exec $nmCmd $nmFlags $modName]

    # replace consecutive whitespaces with single whitespace

    regsub -all { +} $symbolInfo " " symbolInfo

    set symbolInfoListRaw [split $symbolInfo \n]
    set symbolTblEntryList {}
    

    # Parse the symbol list to create the symbol table

    foreach symbolEntry $symbolInfoListRaw {
	# Store output fields
    
    if {![regexp {^ (.*) (.*)} $symbolEntry dummy type name]} {
	    continue
	}

	regsub $symPrefixToIgnore $name "" cName
            
	switch -glob $type {
		[U] {
	    puts $fdOut "extern int ${cName} ();"
	    lappend symbolTblEntryList "(char*) $cName,"
	    }
	    default {
		puts "Warning: makeDepSymTbl.tcl - invalid symbol information \
			($symbolEntry)"
	    }
	}
    }

    set nsyms [llength $symbolTblEntryList]

    # convert nm output to symbol entries in array

    puts $fdOut ""
    puts $fdOut "char * ${symPrefixToAdd}depTbl \[$nsyms\] ="
    puts $fdOut "    {"

    foreach symbolTblEntry $symbolTblEntryList {
        puts $fdOut "        $symbolTblEntry"
    }

    puts $fdOut "    };"
    puts $fdOut ""

    return $nsyms
}

##############################################################################
#
# main - entry point of utility
#
##############################################################################

# check for correct number of args

if {$argc < 3} {
    puts stderr "Usage: makeDepSymTbl.tcl <cpu type> <objMod> <file>"
    exit 1
}

set outFile [lindex $argv 2]

# set appropriate version of the nm command for the processor

if [catch {set makeDepSymTbl::cpuType [lindex $argv 0]}] {
    set makeDepSymTbl::cpuType ""
}

set makeDepSymTbl::nmCmd "nm$makeDepSymTbl::cpuType"

# For sh/68k, the toolchain prepended underscore is stripped from all symbols
# names. 

if {($makeDepSymTbl::cpuType == "sh") || ($makeDepSymTbl::cpuType == "68k")} {
    set makeDepSymTbl::symPrefixToIgnore "_"
}

# Diab special symbols have no underscore prepended. The solution to correctly
# include them in is to declare/define all symbols with no underscore
# prepended and to build as such with a Diab pragma.

if {($makeDepSymTbl::cpuType == "cf")} {
    set makeDepSymTbl::symPrefixToAdd "_"
}

# parse the arguments

set modName [lindex $argv 1]

# Create the module's dependent symbol table

set makeDepSymTbl::fdOut [open $outFile w+]

# The following (C code) goes into the wrapper file depSymTbl.c

puts $makeDepSymTbl::fdOut "/* depSymTbl.c - dependent symbol tables wrapper */"
puts $makeDepSymTbl::fdOut ""
puts $makeDepSymTbl::fdOut "/* CREATED BY $argv0"
puts $makeDepSymTbl::fdOut " *  WITH ARGS $argv"
puts $makeDepSymTbl::fdOut " *         ON [clock format [clock seconds]]"
puts $makeDepSymTbl::fdOut " */"
puts $makeDepSymTbl::fdOut ""
puts $makeDepSymTbl::fdOut ""


# Create the symbol table

set nsyms [makeDepSymTbl::depTblCreate $modName]

puts $makeDepSymTbl::fdOut ""

close $makeDepSymTbl::fdOut
