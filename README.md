# Distributed Vector Clocks with Singhal-Kshemkalyani Optimization

A C++ implementation of vector clocks and the Singhal-Kshemkalyani optimization for distributed systems using MPI. This project demonstrates causal ordering of events and achieves ~22% average reduction in communication overhead.

## Overview

Vector clocks maintain partial ordering of events in distributed systems. The Singhal-Kshemkalyani (SK) optimization reduces message size by sending only updated vector clock entries since the last message to each destination.

## Features

- Standard Vector Clock: Full vector timestamp with every message
- SK Optimization: Differential updates for reduced bandwidth
- Automated Testing: Scripts for running experiments across multiple process counts
- Verification Tools: Python scripts to verify causality, concurrency, and monotonicity
- Visualization: Graphs comparing both implementations

## How to Run

1. Clone this repository on your local machine.
2. Open `README.txt` to see the instructions to set up the environment.
3. Run the files to observe the results.
