#!/bin/bash

STRUCTS="
World
GameState
"

THUNKS="
World_init
World_initGL
GameState_init
GameState_initGL
"



MODSDIR="./mods"

function processStruct() {
	hname=$1
	dir=$2
	modname=$3
	
	if [ -f $dir/$hname.mixin.h ]; then
		echo "  struct $hname"
		echo -e "\nstruct {" >> $MODSDIR/$hname.generated_mixin.h
		echo -e "#include \"$dir/$hname.mixin.h\"" | sed s/^/\\t/  >> $MODSDIR/$hname.generated_mixin.h 
		echo -e "} mod_$modname;\n" >> $MODSDIR/$hname.generated_mixin.h
	fi
	
}

function processThunk() {
	hname=$1
	dir=$2
	modname=$3
	
	if [ -f $dir/$hname.thunk.c ]; then
		echo "  thunk $hname"
		echo -e "\n{" >> $MODSDIR/$hname.generated_thunk.c
		echo -e "#include \"$dir/$hname.thunk.c\"" | sed s/^/\\t/ >> $MODSDIR/$hname.generated_thunk.c 
		echo -e "};\n" >> $MODSDIR/$hname.generated_thunk.c
	fi
	
}


function processMod() {
	for s in $STRUCTS; do
		processStruct $s $1 $2
	done
	for s in $THUNKS; do
		processThunk $s $1 $2
	done
}

function checkDir() {
	bname="$1"
	
	if [ -z $2 ]; then
		pname=""
	else 
		pname="${2}_"
	fi
	
	touch $bname/Makefile.am
	
	subdirs="" 
	
	for dname in `find $bname/* -follow -maxdepth 1 -type d | xargs -n 1 -- basename`; do
		
		
		if [ -f $bname/$dname/modpack ]; then
			packname=`cat $bname/$dname/modpack`
			echo "Found mod pack: $packname"
			checkDir "$bname/$dname" "$pname$packname"
		else
			
			if [ -f $bname/$dname/modname ]; then
				modname=`cat $bname/$dname/modname`
				subdirs="$subdirs $dname"
				
				echo "Found mod: $modname"
				
				processMod "$bname/$dname" $modname
			fi
		fi
		
		
	done
	
	
	# write the makefile
	cat > "$bname/Makefile.am" <<EOF
	
SUBDIRS = $subdirs

ACLOCAL_AMFLAGS = -I m4

EOF



}







rm ./mods/Makefile.am

# clear out generated files
for s in $STRUCTS; do
	echo -e "//\n// GENERATED FILE: DO NOT EDIT\n//\n// included inside struct $s\n\n" > $MODSDIR/$s.generated_mixin.h
done
for s in $THUNKS; do
	echo -e "//\n// GENERATED FILE: DO NOT EDIT\n//\n// included inside function $s()\n\n" > $MODSDIR/$s.generated_thunk.c
done


checkDir "./mods"

