#!/bin/bash


submissions="$1"
targets="$2"
tests="$3"
answers="$4"


#A message showing how to use the script is printed when the number of arguments is less than the required number.
if [ "$#" -lt 4 ];then
    echo "Please write the following Command:"
    echo "./organize.sh submissions targets tests answers"
    echo "./organize.sh submissions targets tests answers -v"
    echo "./organize.sh submissions targets tests answers -v -noexecute"
    exit 1
fi


#This is used to check if the 5th argument is given(showing the mentioned message) or not(showing no message)
if [ -z "$5" ]; then
    org_ex_msg="no"
    execute="yes"
elif [ "$5" == "-v" ] ; then
    org_ex_msg="yes"
    execute="yes"
elif [ "$5" == "-noexecute" ] ; then
    org_ex_msg="no"
    execute="no"
fi


#This is used to check if the 6th argument is given(Only organize the file) or not(organize the file first then executing the file and create a csv file)
if [ -z "$6" ]; then
    if [ $execute != "no" ];then
        execute="yes"  
    fi
elif [ "$6" == "-noexecute" ] ; then
    execute="no"
fi


#Following variable and function is used to write in the csv file
csv="result.csv"

writeCsv() 
{
    local tmp="$1"
    echo "$tmp" >> "$csv"
}


#recursive function to organize and execute file
#$1 variable has the path of ...Workspace/targets/temporary/folder_name(any name is possible) or file_name folder
#$2 variable has the path of ...Workspace/targets folder
#$3 is the student id
visit()
{
    for i in $1/*
    do
        if [ -f "$i" ];then
            ex="${i##*.}"
            if [ $ex == "c" ];then
                mkdir "$2/C/$3"
                mv "$i" "$2/C/$3/main.c"
                if [ "$org_ex_msg" == "yes" ];then
                    echo Organizing files of "$3"
                fi

                if [ "$execute" == "yes" ];then
                    tmp_path=$(pwd) 
                    cd "$2/C/$3"
                    gcc "main.c" -o "main.out"
                    #./"main.out" < "$path/tests/test1.txt" > "out1.txt"
                    test_files=()
                    for file in "$path/tests/"test*.txt
                    do
                        test_files+=("$file")
                    done

                    for test_file in "${test_files[@]}"
                    do
                        base_name=$(basename "$test_file")
                        output_file="out${base_name:4}"
                        ./"main.out" < "$test_file" > "$output_file"
                    done

                    if [ "$org_ex_msg" == "yes" ];then
                        echo Executing files of "$3"
                    fi     
                    cd "$tmp_path"
                fi

            elif [ $ex == "java" ];then
                mkdir "$2/Java/$3"
                mv "$i" "$2/Java/$3/main.java"
                if [ "$org_ex_msg" == "yes" ];then
                    echo Organizing files of "$3"
                fi

                if [ "$execute" == "yes" ];then
                    tmp_path=$(pwd)
                    cd "$2/Java/$3"
                    javac "main.java" 
                    #java "Main" < "$path/tests/test1.txt" > "out1.txt"
                    test_files=()
                    for file in "$path/tests/"test*.txt
                    do
                        test_files+=("$file")
                    done

                    for test_file in "${test_files[@]}"
                    do
                        base_name=$(basename "$test_file")
                        output_file="out${base_name:4}"
                        java "Main" < "$test_file" > "$output_file"
                    done

                    if [ "$org_ex_msg" == "yes" ];then
                        echo Executing files of "$3"
                    fi 
                    cd "$tmp_path"
                fi

            elif [ $ex == "py" ];then
                mkdir "$2/Python/$3"
                mv "$i" "$2/Python/$3/main.py"
                if [ "$org_ex_msg" == "yes" ];then
                    echo Organizing files of "$3"
                fi

                if [ "$execute" == "yes" ];then

                    tmp_path=$(pwd)
                    cd "$2/Python/$3"
                    #python3 "main.py" < "$path/tests/test1.txt" > "out1.txt"

                    test_files=()
                    for file in "$path/tests/"test*.txt
                    do
                        test_files+=("$file")
                    done

                    for test_file in "${test_files[@]}"
                    do
                        base_name=$(basename "$test_file")
                        output_file="out${base_name:4}"
                        python3 "main.py" < "$test_file" > "$output_file"
                    done

                    if [ "$org_ex_msg" == "yes" ];then
                        echo Executing files of "$3"
                    fi
                    cd "$tmp_path"
                fi
            
            fi

            
        elif [ -d "$i" ];then
            visit "$i" $2 $3
        
        fi

    done
}


#If there is a folder named targets exits then delete this folder recursively.
if [ -d "$targets" ];then
    rm -r targets
fi



mkdir targets
cd targets
mkdir C Java Python
mkdir temporary

cd ..

#path variable has the path of Workspace folder
path=$(pwd)

cd submissions

#dst_folder variable has the path of ...Workspace/targets/temporary folder
dst_folder="$path/targets/temporary"

for f in *.zip; do
    basename=$(basename "$f")

    extension="${basename##*.}"

    basename_wo_extension="${basename%.*}"

    uId="${basename_wo_extension: -7}"

    unzip -q "$f" -d "$dst_folder/$uId"
done

cd ..

cd targets

#path1 variable has the path of ...Workspace/targets folder
path1=$(pwd)

cd temporary

#path2 variable has the path of ...Workspace/targets/temporary folder
path2=$(pwd)

for i in $path2/*
do
    var=$(basename "$i")
    visit "$i" $path1 $var
done

cd ..

rm -r "temporary"


if [ "$execute" == "yes" ];then
    value="student_id,type,matched,not_matched"

    writeCsv "$value"


    ans_files=()
    for file in "$path/answers/"ans*.txt
    do
        ans_files+=("$file")
    done


    for i in "C"/*
    do 
        sid=$(basename "$i")
        type="C"
        match=0
        unmatch=0
        
        out_files=()
        for file in "$i/"out*.txt
        do
            out_files+=("$file")
        done

        for ((i=0; i<${#out_files[@]}; i++))
        do
            out_file=${out_files[i]}
            ans_file=${ans_files[i]}
            diff_output=$(diff "$out_file" "$ans_file")
            if [ -z "$diff_output" ];then
                ((match += 1))
            else
                ((unmatch += 1))
            fi

        done
 
        value="$sid,$type,$match,$unmatch"

        writeCsv "$value"
    done

    for i in "Java"/*
    do 
        sid=$(basename "$i")
        type="Java"
        match=0
        unmatch=0
        

        out_files=()
        for file in "$i/"out*.txt
        do
            out_files+=("$file")
        done

        for ((i=0; i<${#out_files[@]}; i++))
        do
            out_file=${out_files[i]}
            ans_file=${ans_files[i]}
            diff_output=$(diff "$out_file" "$ans_file")
            if [ -z "$diff_output" ];then
                ((match += 1))
            else
                ((unmatch += 1))
            fi

        done
        
        value="$sid,$type,$match,$unmatch"

        writeCsv "$value"
    done

    for i in "Python"/*
    do 
        sid=$(basename "$i")
        type="Python"
        match=0
        unmatch=0
        

        out_files=()
        for file in "$i/"out*.txt
        do
            out_files+=("$file")
        done

        for ((i=0; i<${#out_files[@]}; i++))
        do
            out_file=${out_files[i]}
            ans_file=${ans_files[i]}
            diff_output=$(diff "$out_file" "$ans_file")
            if [ -z "$diff_output" ];then
                ((match += 1))
            else
                ((unmatch += 1))
            fi

        done
        
        value="$sid,$type,$match,$unmatch"

        writeCsv "$value"
    done

fi