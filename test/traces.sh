#!/bin/bash

# Initial hexadecimal key
initial_key="FFFF00000000FFFF0F0F0F0FF0F0F0F0"
echo "Initial key: $initial_key"

# Split the initial_key into bytes and store in a tuple (array)
initial_key_tuple=()
for ((i=0; i<${#initial_key}; i+=2)); do
    byte="0x${initial_key:i:2}"
    initial_key_tuple+=("$byte")
done

# Print the tuple containing each byte of the initial key
echo "Initial key bytes as tuple: (${initial_key_tuple[*]})"

# Run aes with 17th argument as 1000, 10'000, 100'000, 1'000'000
for i in 1000 10000 100000 1000000; do
    echo "Running aes with inner loop size as $i"

    for ((j=0; j<2; j+=1)); do
        sudo taskset -c 1 ./aes "${initial_key_tuple[@]}" $i $j
    done

    out=$(python3 comparison.py results/traces_"$i"_0.csv results/traces_"$i"_1.csv)
    echo "Correlation for inner loop size $i: $out"
done

