for size in "1gb.txt" "5gb.txt" "10gb.txt"
do 
    for i in {1..10}
    do
        ts=$(date +%s%N) ; ./counter $size ; tt=$((($(date +%s%N) - $ts)/1000000)) ; echo ""
        res_file="res_serial_{$1}_{$size}_{$i}.txt"
        echo "$size,$tt ms" >> $res_file
        ./b2-linux upload_file maciektr-public $res_file $res_file
    done;
done;