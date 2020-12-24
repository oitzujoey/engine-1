#!/bin/bash

SERVER=sengine-1
CLIENT=cengine-1

CC=gcc
CFLAGS='-std=c99 -pedantic-errors -Werror'
CLIBS='-lSDL2 -lSDL2_net -llua'

# Not a command line argument
DEBUG='-debug'
DIDSTUFF=false


FORCECOMPILE=false

# build clean
if [ "$1" == "clean" ]
then
    echo 'Running clean'
    find . -name "*.o" -type f -delete
    rm $SERVER $SERVER$DEBUG $CLIENT $CLIENT$DEBUG
    exit
fi

# build debug
if [ "$1" == "debug" ]
then
    CFLAGS+=' -DDEBUG -g -O0';
else
    DEBUG=''
fi

# @TODO: When a .h file is modified, only rebuild .o files that include that file.
function compile_directory {
    
    for CFILE in $1/*.c
    do
        OFILE=`echo $CFILE | sed 's/\.c/.o/'`
        
        for HFILE in $1/*.h
        do
            if [ $HFILE -nt $OFILE ]
            then
                FORCECOMPILE=true;
            fi
        done
    
        if [ $CFILE -nt $OFILE ] || $FORCECOMPILE
        then
            echo "$CFILE -> $OFILE"
            $CC $CFLAGS -c -o $OFILE $CFILE
            DIDSTUFF=true
        fi
    done
    
    FORCECOMPILE=false
}

function final_compile {
    BINARY=$1
    OFILESTRING=${@:2}
    IFS=' ' read -r -a OFILEARRAY <<< "$OFILESTRING"
    
    for OFILE in ${OFILEARRAY[@]}
    do
        if [ $OFILE -nt $BINARY$DEBUG ]
        then
            echo "{ $OFILESTRING } -> $BINARY$DEBUG"
            $CC $CFLAGS -o $BINARY$DEBUG $OFILESTRING $CLIBS
            DIDSTUFF=true
            break
        fi
    done
}

compile_directory src/common

# TEMPCFLAGS=$CFLAGS
# CFLAGS+=' -DCLIENT'
compile_directory src/client
final_compile cengine-1 src/client/*.o src/common/*.o

# CFLAGS=$TEMPCFLAGS
# CFLAGS+=' -DSERVER'
compile_directory src/server
final_compile sengine-1 src/server/*.o src/common/*.o

if ! $DIDSTUFF
then
    echo 'Nothing to do'
fi
