for scale in "0.1" "0.25" "0.5" "0.75" "1" "1.25" "1.5" "1.75" "2" "10"
do 
    for blocks in 8 16 32 64 128 256 512 1024 2048
    do
        ./image_scailing.out "voyager2.pgm" $scale $blocks >> result.txt
        ./image_scailing.out "aerosmith.pgm" $scale $blocks >> result.txt
    done;
done;