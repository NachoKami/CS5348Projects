# Build
gcc prog1.c -o prog1 -w
gcc prog2.c -o prog2 -lpthread -std=c99 -w

# Run
date
./prog1 a.txt b.txt c1.txt
date
./prog2
date
