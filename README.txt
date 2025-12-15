Steps for setting up and running the programs:

1. Set up the environment - 
Open the Linux Terminal and run:


# Update system
sudo apt-get update

# Install MPI
sudo apt-get install -y openmpi-bin openmpi-common libopenmpi-dev

# Install Python and dependencies
sudo apt-get install -y python3 python3-pip
pip3 install matplotlib


This sets up MPI and Python.

2. Set up the directory with all the required files.
3. Open the Linux Terminal at the directory and run:


make all


This 'makes' and sets up all the programs.

4. Now run all these files one-by-one:


# Quick test (n=3)
./quick_test.sh

# Verification
python3 verify_consistency.py

# Running the full experiments (n=10 to n=15)
./run_experiments.sh

# Plotting the results
python3 plot_results.py


This generates and saves all the required results, thus completing the execution workflow.
