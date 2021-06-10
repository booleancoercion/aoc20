files=$(find . -type f -name '*.c')
flags=$(cat ./compile_flags.txt)

clang $files $flags
