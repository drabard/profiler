set -e

clang++ -g -O0 test.cpp -o test
./test
