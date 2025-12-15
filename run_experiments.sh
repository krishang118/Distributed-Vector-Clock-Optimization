#!/bin/bash

echo "Running Vector Clock Experiments..."
mkdir -p experiment_results
for n in {10..15}
do
    echo "Running experiment with n=$n processes..."
    echo "$n 5 1.5 50" > inp-params.txt
    for i in $(seq 1 $n)
    do
        echo -n "$i " >> inp-params.txt
        for j in $(seq 1 $n)
        do
            if [ $i -ne $j ]; then
                echo -n "$j " >> inp-params.txt
            fi
        done
        echo "" >> inp-params.txt
    done
    echo "  Running standard vector clock..."
    mpirun --oversubscribe -np $n ./vector_clock
    mv vector_clock_log.txt experiment_results/vc_log_n${n}.txt
    mv vector_clock_stats.txt experiment_results/vc_stats_n${n}.txt
    echo "  Running SK optimization..."
    mpirun --oversubscribe -np $n ./sk_vector_clock
    mv sk_vector_clock_log.txt experiment_results/sk_log_n${n}.txt
    mv sk_vector_clock_stats.txt experiment_results/sk_stats_n${n}.txt
    echo "  Completed n=$n"
    echo ""
done
echo "All experiments completed."
echo "Results saved in experiment_results/ directory"
echo "Generating summary..."
echo "n,Standard_VC_Avg,SK_VC_Avg" > experiment_results/summary.csv
for n in {10..15}
do
    std_avg=$(grep "Average" experiment_results/vc_stats_n${n}.txt | awk '{print $NF}')  
    sk_avg=$(grep "Average" experiment_results/sk_stats_n${n}.txt | awk '{print $NF}')
    echo "$n,$std_avg,$sk_avg" >> experiment_results/summary.csv
done
echo "Summary created in experiment_results/summary.csv"
