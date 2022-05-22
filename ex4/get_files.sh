echo "Get mapper and reducer"
wget https://raw.githubusercontent.com/maciektr/parallel_lab/main/ex4/reducer.py
wget https://raw.githubusercontent.com/maciektr/parallel_lab/main/ex4/mapper.py

echo "Get gutenberg file"
wget https://www.i3s.unice.fr/\~jplozi/hadooplab_lsds_2015/datasets/gutenberg-500M.txt.gz

gzip -d gutenberg-500M.txt.gz 

file="1gb.txt"
echo "Build $file file"
if [ -f "$file" ]; then
    echo "$file already exists."
else 
    cat gutenberg-500M.txt >> 1gb.txt
    cat gutenberg-500M.txt >> 1gb.txt
fi

file="5gb.txt"
echo "Build $file file"
if [ -f "$file" ]; then
    echo "$file already exists."
else 
    for i in {1..5}
    do
        cat 1gb.txt >> 5gb.txt
    done
fi

file="10gb.txt"
echo "Build $file file"
if [ -f "$file" ]; then
    echo "$file already exists."
else 
    cat 5gb.txt >> 10gb.txt
    cat 5gb.txt >> 10gb.txt
fi

echo "Get b2 client"
wget https://github.com/Backblaze/B2_Command_Line_Tool/releases/latest/download/b2-linux
chmod +x b2-linux
