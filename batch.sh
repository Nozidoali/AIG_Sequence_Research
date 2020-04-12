echo "" > test.csv
for file in data/epfl/random_control/*
do
    if test -f $file ;
    then
        name=`basename $file`
        filename="${name%%.*}"
        echo ${filename} 
        echo "${filename},\c " >> test.csv
        ./main -i ${file} >> test.csv
        echo "" >> test.csv
    fi
done
for file in data/epfl/arithmetic/*
do
    if test -f $file ;
    then
        name=`basename $file`
        filename="${name%%.*}"
        echo ${filename}
        echo "${filename},\c" >> test.csv
        ./main -i ${file} >> test.csv
        echo  "" >> test.csv
    fi
done
