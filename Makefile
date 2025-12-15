CXX = mpic++
CXXFLAGS = -std=c++11 -O2 -Wall
TARGETS = vector_clock sk_vector_clock
all: $(TARGETS)
vector_clock: vector_clock.cpp
	$(CXX) $(CXXFLAGS) -o vector_clock vector_clock.cpp
sk_vector_clock: sk_vector_clock.cpp
	$(CXX) $(CXXFLAGS) -o sk_vector_clock sk_vector_clock.cpp
clean:
	rm -f $(TARGETS)
	rm -f *.txt
	rm -rf experiment_results
run: all
	mpirun --oversubscribe -np 3 ./vector_clock
	@echo "\nStandard Vector Clock Results:"
	@cat vector_clock_stats.txt
	@echo "\nRunning SK Optimization:"
	mpirun --oversubscribe -np 3 ./sk_vector_clock
	@echo "\nSK Vector Clock Results:"
	@cat sk_vector_clock_stats.txt
experiments: all
	@chmod +x run_experiments.sh
	@./run_experiments.sh
verify:
	@python3 verify_consistency.py
plot:
	@python3 plot_results.py
full: experiments verify plot
.PHONY: all clean run experiments verify plot full
