sudo yum install -y gcc-c++

wget https://raw.githubusercontent.com/maciektr/parallel_lab/main/ex4/counter.cpp

g++ -O3 -static -o counter counter.cpp

wget https://raw.githubusercontent.com/maciektr/parallel_lab/main/ex4/test_serial.sh
chmod +x test_serial.sh
./test_serial