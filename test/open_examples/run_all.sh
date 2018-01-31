#!/bin/sh

RUN_MODE=hw_x64

for file in ./* 
do 
if [ -d $file ]; then 
    APP=`echo $file | awk -F '/' '{print $NF}'`
    cd $file
    make clean -f ../Makefile.linux
    make -f ../Makefile.linux 
    echo 
    echo 
    
    echo $APP Running on $RUN_MODE mode ...
    echo 
    echo 

    if test $APP = "common"; then
        echo "common is not a test, skip" 
    elif test $APP = "vcaOpSAD"; then
        ./"$RUN_MODE"."$APP" 4096 8 -random
    elif test $APP = "PrefixSum"; then
        ./"$RUN_MODE"."$APP" 16 
    else
        ./"$RUN_MODE"."$APP"
    fi 

    if test "$?" -eq 0 ; then
        echo
        echo "$APP test passed on $RUN_MODE!"
        echo
        cd ../
    else
        echo  
        echo "$APP test failed on $RUN_MODE!"
        echo "continue... yes or no ? "
        echo  
        read answer
        while test "$answer" != "yes" -a "$answer" != "no"
        do
        read answer
        done
    
        if test "$answer" != "no" ; then
            cd ../
            echo 
            echo 
        else
            exit 1 
        fi 
     fi
 fi
done
