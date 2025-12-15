#include <mpi.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <random>
#include <chrono>
#include <thread>
#include <algorithm>
#include <iomanip>
#include <ctime>
#include <map>
using namespace std;
struct Event {
    int process_id;
    string type;
    int event_num;
    vector<int> vc;
    string timestamp;
    int dest_process = -1;
    int msg_id = -1;};
class SKVectorClock {
private:
    int n;
    int my_rank;
    vector<int> clock;
    vector<vector<int>> LS;
public:
    SKVectorClock(int num_processes, int rank) : n(num_processes), my_rank(rank) {
        clock.resize(n, 0);
        LS.resize(n, vector<int>(n, 0));}
    void increment() {
        clock[my_rank]++;}
    vector<int> get_clock() {
        return clock;}
    map<int, int> get_diff_clock(int dest) {
        map<int, int> diff;
        for (int k = 0; k < n; k++) {
            if (clock[k] > LS[dest][k]) {
                diff[k] = clock[k];
                LS[dest][k] = clock[k];}}
        return diff;}
    void update(const map<int, int>& received_clock, int sender) {
        for (const auto& entry : received_clock) {
            int idx = entry.first;
            int val = entry.second;
            clock[idx] = max(clock[idx], val);}
        clock[my_rank]++;}
    int get_size_sent(int dest) {
        int count = 0;
        for (int k = 0; k < n; k++) {
            if (clock[k] > LS[dest][k]) {
                count++;}}
        return count;}};
string get_current_time() {
    auto now = chrono::system_clock::now();
    auto ms = chrono::duration_cast<chrono::milliseconds>(now.time_since_epoch()) % 1000;
    auto timer = chrono::system_clock::to_time_t(now);
    tm bt = *localtime(&timer);
    ostringstream oss;
    oss << put_time(&bt, "%H:%M:%S");
    oss << '.' << setfill('0') << setw(3) << ms.count();
    return oss.str();}
string vc_to_string(const vector<int>& vc) {
    ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < vc.size(); i++) {
        oss << vc[i];
        if (i < vc.size() - 1) oss << " ";}
    oss << "]";
    return oss.str();}
int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int world_size, world_rank;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    int n, m;
    double lambda, alpha;
    vector<vector<int>> adjacency_list;
    if (world_rank == 0) {
        ifstream infile("inp-params.txt");
        infile >> n >> lambda >> alpha >> m;
        adjacency_list.resize(n);
        string line;
        getline(infile, line);
        for (int i = 0; i < n; i++) {
            getline(infile, line);
            istringstream iss(line);
            int node, neighbor;
            iss >> node;
            while (iss >> neighbor) {
                adjacency_list[i].push_back(neighbor - 1);}}
        infile.close();}
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&lambda, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(&alpha, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Bcast(&m, 1, MPI_INT, 0, MPI_COMM_WORLD);
    for (int i = 0; i < n; i++) {
        int size;
        if (world_rank == 0) {
            size = adjacency_list[i].size();}
        MPI_Bcast(&size, 1, MPI_INT, 0, MPI_COMM_WORLD);
        if (world_rank != 0) {
            adjacency_list.resize(n);
            adjacency_list[i].resize(size);}
        if (size > 0) {
            MPI_Bcast(adjacency_list[i].data(), size, MPI_INT, 0, MPI_COMM_WORLD);}}
    SKVectorClock vc(n, world_rank);
    vector<Event> local_events;
    random_device rd;
    mt19937 gen(rd() + world_rank);
    exponential_distribution<> exp_dist(1.0 / lambda);
    uniform_real_distribution<> uniform(0.0, 1.0);
    int messages_sent = 0;
    int event_counter = 1;
    int total_vc_entries_sent = 0;
    auto start_time = chrono::steady_clock::now();
    while (messages_sent < m) {
        int flag;
        MPI_Status status;
        MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
        if (flag) {
            int count;
            MPI_Get_count(&status, MPI_INT, &count);
            vector<int> received_data(count);
            MPI_Recv(received_data.data(), count, MPI_INT, status.MPI_SOURCE, 
                     status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            int msg_id = received_data[0];
            int num_entries = received_data[1];
            map<int, int> received_vc;
            for (int i = 0; i < num_entries; i++) {
                int idx = received_data[2 + 2*i];
                int val = received_data[2 + 2*i + 1];
                received_vc[idx] = val;}
            vc.update(received_vc, status.MPI_SOURCE);
            Event e;
            e.process_id = world_rank;
            e.type = "receive";
            e.event_num = event_counter++;
            e.vc = vc.get_clock();
            e.timestamp = get_current_time();
            e.dest_process = status.MPI_SOURCE;
            e.msg_id = msg_id;
            local_events.push_back(e);}
        if (messages_sent < m) {
            double prob = uniform(gen);
            double send_prob = 1.0 / (1.0 + alpha);
            if (prob < send_prob && !adjacency_list[world_rank].empty()) {
                vc.increment();
                int neighbor_idx = gen() % adjacency_list[world_rank].size();
                int dest = adjacency_list[world_rank][neighbor_idx];
                Event e;
                e.process_id = world_rank;
                e.type = "send";
                e.event_num = event_counter++;
                e.vc = vc.get_clock();
                e.timestamp = get_current_time();
                e.dest_process = dest;
                e.msg_id = messages_sent + 1;
                local_events.push_back(e);
                map<int, int> diff_vc = vc.get_diff_clock(dest);
                vector<int> send_data;
                send_data.push_back(messages_sent + 1);
                send_data.push_back(diff_vc.size());
                for (const auto& entry : diff_vc) {
                    send_data.push_back(entry.first);
                    send_data.push_back(entry.second);}
                MPI_Send(send_data.data(), send_data.size(), MPI_INT, dest, 0, MPI_COMM_WORLD);
                total_vc_entries_sent += diff_vc.size();
                messages_sent++;
            } else {
                vc.increment();
                Event e;
                e.process_id = world_rank;
                e.type = "internal";
                e.event_num = event_counter++;
                e.vc = vc.get_clock();
                e.timestamp = get_current_time();
                local_events.push_back(e);}
            double sleep_time = exp_dist(gen);
            this_thread::sleep_for(chrono::milliseconds((int)sleep_time));}}
    this_thread::sleep_for(chrono::milliseconds(100));
    int flag = 1;
    while (flag) {
        MPI_Status status;
        MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
        if (flag) {
            int count;
            MPI_Get_count(&status, MPI_INT, &count);
            vector<int> received_data(count);
            MPI_Recv(received_data.data(), count, MPI_INT, status.MPI_SOURCE, 
                     status.MPI_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            int msg_id = received_data[0];
            int num_entries = received_data[1];
            map<int, int> received_vc;
            for (int i = 0; i < num_entries; i++) {
                int idx = received_data[2 + 2*i];
                int val = received_data[2 + 2*i + 1];
                received_vc[idx] = val;}
            vc.update(received_vc, status.MPI_SOURCE);
            Event e;
            e.process_id = world_rank;
            e.type = "receive";
            e.event_num = event_counter++;
            e.vc = vc.get_clock();
            e.timestamp = get_current_time();
            e.dest_process = status.MPI_SOURCE;
            e.msg_id = msg_id;
            local_events.push_back(e);}} 
    MPI_Barrier(MPI_COMM_WORLD);
    int avg_entries = total_vc_entries_sent;
    if (world_rank == 0) {
        vector<int> all_entries(n);
        MPI_Gather(&avg_entries, 1, MPI_INT, all_entries.data(), 1, MPI_INT, 0, MPI_COMM_WORLD);
        int total_entries = 0;
        for (int e : all_entries) total_entries += e;
        double avg = (double)total_entries / (n * m);
        ofstream stats("sk_vector_clock_stats.txt");
        stats << "Total VC entries sent: " << total_entries << endl;
        stats << "Total messages sent: " << (n * m) << endl;
        stats << "Average VC entries per message: " << avg << endl;
        stats.close();
    } else {
        MPI_Gather(&avg_entries, 1, MPI_INT, nullptr, 0, MPI_INT, 0, MPI_COMM_WORLD);}
    for (int i = 0; i < n; i++) {
        if (world_rank == i) {
            ofstream logfile;
            if (i == 0) {
                logfile.open("sk_vector_clock_log.txt", ios::out);
            } else {
                logfile.open("sk_vector_clock_log.txt", ios::app);}
            for (const auto& event : local_events) {
                logfile << "Process" << (event.process_id + 1) << " ";
                if (event.type == "internal") {
                    logfile << "executes internal event e" << (event.process_id + 1) 
                            << event.event_num;
                } else if (event.type == "send") {
                    logfile << "sends message m" << (event.process_id + 1) << event.msg_id 
                            << " to process" << (event.dest_process + 1);
                } else {
                    logfile << "receives m" << (event.dest_process + 1) << event.msg_id 
                            << " from process" << (event.dest_process + 1);}
                logfile << " at " << event.timestamp << ", vc: " << vc_to_string(event.vc) << endl;}
            logfile.close();}
        MPI_Barrier(MPI_COMM_WORLD);}
    MPI_Finalize();
    return 0;}
