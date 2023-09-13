#!/bin/bash
function diffs() {
    diff "${@:3}" <(cat sample_output/result-"$1"-input_"$2") <(cat output/result-"$1"-input_"$2"); 
}

for alloc_type in {0..1}; do
    for sample_input_num in {1..12}; do
        # Output sample_input_num and alloc_type
        echo "Testing sample input $sample_input_num with allocation type $alloc_type"
        for i in {1..10}; do
            # don't allow proj2 to output to stdout
            ./proj2 "$alloc_type" sample_input/input_"$sample_input_num" > /dev/null
            diffs "$alloc_type" "$sample_input_num"
        done
    done
done