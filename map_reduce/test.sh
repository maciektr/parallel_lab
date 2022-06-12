wget https://raw.githubusercontent.com/maciektr/parallel_lab/main/ex4/get_files.sh
chmod +x get_files.sh
./get_files.sh

CMD="hadoop jar /usr/lib/hadoop/hadoop-streaming.jar -files mapper.py,reducer.py -mapper mapper.py -reducer reducer.py -input words -output words-output"

run_hadoop() {
    ts=$(date +%s%N) ; $CMD ; tt=$((($(date +%s%N) - $ts)/1000000)) ; echo ""
}

file_size="1gb.txt"

for size in "1gb.txt" "5gb.txt" "10gb.txt"
do 
    hdfs dfs -mkdir words
    hdfs dfs -put $file_size words

    for i in {1..10}
    do
        res_file="res_{$1}_{$size}_{$i}.txt"
        file_size=$size
        run_hadoop
        echo "$1,$file_size,$i,$tt ms" >> $res_file
        ./b2-linux upload_file maciektr-public $res_file $res_file
        hdfs dfs -rm -r words-output
    done;
done;