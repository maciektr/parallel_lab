for type in 0 1
do 
    for bins in 10 50 100 500 1000
    do 
        for i in 1 2 3 4 5 6 7 8 9 10
        do 
            ./histo.out $type $bins >> result.txt
        done;
    done;
done;