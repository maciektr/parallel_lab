for type in "shared" "naive"
do 
    for n in 256 512 1024 2048 4096 8192 16384 32768 65536 131072
    do 
        for blocks in 8 16 32 64 128 256 512 1024 2048
        do  
            for grid in "1 1" "2 1" "4 1" "1 2" "1 4"
            do 
                ./matrix_transpose_1.out $n $type $blocks $grid >> result.txt
                ./matrix_transpose_2.out $n $type $blocks $grid >> result.txt
            done;
        done;
    done;
done;