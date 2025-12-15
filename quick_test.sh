#!/bin/bash

echo "Vector Clock Implementation - Quick Test"
echo ""
if [ ! -f "vector_clock" ] || [ ! -f "sk_vector_clock" ]; then
    echo "Compiling programs..."
    make all
    echo ""
fi
if [ ! -f "inp-params.txt" ]; then
    echo "Creating inp-params.txt..."
    cat > inp-params.txt << EOF
3 5 1.5 40
1 2 3
2 1 3
3 1 2
EOF
    echo "Created inp-params.txt with n=3, λ=5, α=1.5, m=40"
    echo ""
fi
echo "Running Standard Vector Clock (n=3)..."
mpirun --oversubscribe -np 3 ./vector_clock
if [ $? -eq 0 ]; then
    echo ""
    echo "Standard Vector Clock completed successfully."
    echo ""
    echo "Statistics:"
    cat vector_clock_stats.txt
    echo ""
    echo "First 10 events from log:"
    head -10 vector_clock_log.txt
    echo "... (see vector_clock_log.txt for full log)"
else
    echo "Standard Vector Clock failed."
    exit 1
fi
echo ""
echo "Running SK Vector Clock Optimization (n=3)..."
mpirun --oversubscribe -np 3 ./sk_vector_clock
if [ $? -eq 0 ]; then
    echo ""
    echo "SK Vector Clock completed successfully."
    echo ""
    echo "Statistics:"
    cat sk_vector_clock_stats.txt
    echo ""
    echo "First 10 events from log:"
    head -10 sk_vector_clock_log.txt
    echo "... (see sk_vector_clock_log.txt for full log)"
else
    echo "SK Vector Clock failed."
    exit 1
fi
echo ""
echo "Comparison Summary:"
std_avg=$(grep "Average" vector_clock_stats.txt | awk '{print $NF}')
sk_avg=$(grep "Average" sk_vector_clock_stats.txt | awk '{print $NF}')
echo "Standard VC - Average entries per message: $std_avg"
echo "SK Optimization - Average entries per message: $sk_avg"
if [ ! -z "$std_avg" ] && [ ! -z "$sk_avg" ]; then
    savings=$(echo "scale=2; (($std_avg - $sk_avg) / $std_avg) * 100" | bc)
    echo "Savings: $savings%"
fi
echo ""
echo "Files generated:"
echo "  - vector_clock_log.txt"
echo "  - vector_clock_stats.txt"
echo "  - sk_vector_clock_log.txt"
echo "  - sk_vector_clock_stats.txt"
echo ""
echo "Run 'python3 verify_consistency.py' to verify correctness"
