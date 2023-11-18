#!/bin/bash






visit()
{
	if [ -d "$1" ]
	then
	
		for i in "$1"/*
		do
			visit "$i"
		done
	
	elif [ -f "$1" ]
	then
		
        basename=$(basename "$i")

        extension="${basename##*.}"

        basename_wo_extension="${basename%.*}"

        if [ "$extension" == "zip" ];then
            unzip -q "$i" -d "$path2/$basename"
        else
            mv "$i" "$path2/$basename"
        fi
	fi
}






inputs="$1"
outputs="$2"

if [ -d "$outputs" ];then
    rm -r outputs
fi



mkdir $outputs

cd $outputs
path2=$(pwd)

cd ..

#take the path where A2 folder and shell.sh exiss
path=$(pwd)

cd A2

path1=$(pwd)

for i in $path1/*
do
    visit "$i"
done