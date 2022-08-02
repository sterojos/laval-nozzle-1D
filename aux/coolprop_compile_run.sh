#!/bin/bash

if [[ -z "$1" ]];
then
	echo "missing file name"
elif [[ -n "$2" ]];
then
	echo "too many args"
else
	g++ -L. -I/home/pepik/CoolProp/include $1 -lCoolProp -ldl
	printf "\nVYSTUP PROGRAMU\n"
	./a.out
fi
