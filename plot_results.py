#!/usr/bin/env python3

import matplotlib.pyplot as plt
import csv
n_values = []
standard_vc_avg = []
sk_vc_avg = []
with open('experiment_results/summary.csv', 'r') as f:
    reader = csv.DictReader(f)
    for row in reader:
        n_values.append(int(row['n']))
        standard_vc_avg.append(float(row['Standard_VC_Avg']))
        sk_vc_avg.append(float(row['SK_VC_Avg']))
plt.figure(figsize=(10, 6))
plt.plot(n_values, standard_vc_avg, marker='o', linewidth=2, 
         markersize=8, label='Standard Vector Clock')
plt.plot(n_values, sk_vc_avg, marker='s', linewidth=2, 
         markersize=8, label='Singhal-Kshemkalyani Optimization')
plt.xlabel('Number of Processes (n)', fontsize=12)
plt.ylabel('Average VC Entries per Message', fontsize=12)
plt.title('Comparison of Vector Clock Implementations', fontsize=14, fontweight='bold')
plt.legend(fontsize=11)
plt.grid(True, alpha=0.3)
plt.xticks(n_values)
for i, (n, std, sk) in enumerate(zip(n_values, standard_vc_avg, sk_vc_avg)):
    plt.text(n, std + 0.2, f'{std:.2f}', ha='center', fontsize=9)
    plt.text(n, sk - 0.3, f'{sk:.2f}', ha='center', fontsize=9)
plt.tight_layout()
plt.savefig('experiment_results/comparison_graph.png', dpi=300)
print("Graph saved as experiment_results/comparison_graph.png")
print("\nSavings Analysis:")
print("\n")
for n, std, sk in zip(n_values, standard_vc_avg, sk_vc_avg):
    savings = ((std - sk) / std) * 100
    print(f"n={n}: Standard={std:.2f}, SK={sk:.2f}, Savings={savings:.2f}%")
plt.show()
