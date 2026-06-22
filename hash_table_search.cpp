// *********************************************************
// Program: hash_table_search_linear.cpp
// Course: CCP6214 Algorithm Design and Analysis
// Lecture Class: TC2L
// Tutorial Class: TT8L
// Trimester: 2610
// Member_1: 242UC244KB | KOK HUEY HUEY | KOK.HUEY.HUEY@student.mmu.edu.my | 0162011560
// Member_2: 242UC244KD | LIM JOEY | LIM.JOEY@student.mmu.edu.my | 0192270150
// Member_3: 242UC242LB | YAP SHEN YEE | YAP.SHEN.YEE@student.mmu.edu.my | 0162897881
// Member_4: 242UC244KB | YAP YU NING | YAP.YU.NING@student.mmu.edu.my | 0122293817
// *********************************************************
// Task Distribution
// Member_1: Radix sort algorithm
// Member_2: Heap sort algorithm
// Member_3: Hash table search algorithm
// Member_4: Dataset generator
// *********************************************************
//
// Collision resolution: LINEAR PROBING (open addressing). Every record is
// stored directly in the table array (no linked lists). On collision we walk
// forward one slot at a time -- (home + 1), (home + 2), ... wrapping around --
// until an empty slot (insert) or the matching key (search) is found.
// Probe distance is measured in slots examined: 1 = found at the home slot.
//
// Because open addressing stores n records inside the array itself, the table
// MUST be larger than n (load factor < 1) or insertion cannot terminate. We
// size it as nextPrime(2n), giving a load factor near 0.5 to keep probe
// sequences short.
// *********************************************************

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <sstream>
#include <direct.h>

using namespace std;
using namespace chrono;

#define MAKE_DIR(d) _mkdir(d)

struct Record { long long key; string str; };

// ─── Hash Table (linear probing / open addressing) ───────
struct Slot {
    Record data;
    bool occupied;
    Slot() : occupied(false) {}
};

class HashTable {
public:
    Slot* table;
    int tableSize;

    HashTable(int size) {
        tableSize = size;
        table = new Slot[tableSize];
    }
    ~HashTable() { delete[] table; }

    int hf(long long k) const { return (int)(k % tableSize); }

    void insert(Record r) {
        int i = hf(r.key);
        while (table[i].occupied) {
            if (table[i].data.key == r.key) return;   // skip duplicate keys
            i++;
            if (i == tableSize) i = 0;                 // wrap around
        }
        table[i].data = r;
        table[i].occupied = true;
    }

    bool search(long long tgt) const {
        int i = hf(tgt);
        while (table[i].occupied) {
            if (table[i].data.key == tgt) return true;
            i++;
            if (i == tableSize) i = 0;
        }
        return false;                                  // hit an empty slot
    }

    // Number of slots examined to FIND tgt (>=1), or 0 if not present.
    int probeLength(long long tgt) const {
        int i = hf(tgt);
        int probes = 1;
        while (table[i].occupied) {
            if (table[i].data.key == tgt) return probes;
            i++;
            if (i == tableSize) i = 0;
            probes++;
        }
        return 0;
    }
};

// ─── Prime helpers (table size = next prime >= 2n) ───────
bool isPrime(int num) {
    if (num <= 1) return false;
    if (num <= 3) return true;
    if (num % 2 == 0 || num % 3 == 0) return false;
    for (int i = 5; i * i <= num; i += 6)
        if (num % i == 0 || num % (i + 2) == 0) return false;
    return true;
}

int nextPrime(int num) {
    if (num <= 2) return 2;
    int p = num;
    if (p % 2 == 0) p++;
    while (true) {
        if (isPrime(p)) return p;
        p += 2;
    }
}

vector<Record> loadCSV(const string& fn) {
    vector<Record> d; ifstream fin(fn);
    if (!fin.is_open()) { cerr << "Error: cannot open " << fn << endl; return d; }
    string line;
    while (getline(fin, line)) {
        if (line.empty()) continue;
        size_t c = line.find(','); if (c == string::npos) continue;
        Record r;
        r.key = atoll(line.substr(0, c).c_str());
        r.str = line.substr(c + 1);
        if (!r.str.empty() && r.str.back() == '\r') r.str.pop_back();
        d.push_back(r);
    }
    return d;
}

string fmtTime(double t) {
    ostringstream oss; oss << fixed << setprecision(9) << t; return oss.str();
}

void fisherYatesShuffle(vector<long long>& arr) {
    for (int i = (int)arr.size() - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        long long tmp = arr[i]; arr[i] = arr[j]; arr[j] = tmp;
    }
}

int main(int argc, char* argv[]) {
    // Seed matches group leader ID: 242UC242LB -> 2421324222
    srand((unsigned int)2421324222U);

    string csvFile = "datasets/dataset_1000.csv";
    if (argc >= 2) csvFile = argv[1];

    // Load (not timed)
    vector<Record> data = loadCSV(csvFile);
    if (data.empty()) { cerr << "No data loaded." << endl; return 1; }
    int n = (int)data.size();
    cout << "Dataset loaded: " << n << " elements" << endl;

    // Build hash table (not timed).
    // Open addressing needs spare slots; size = next prime >= 2n keeps the
    // load factor near 0.5 so probe sequences stay short.
    int dynamicTableSize = nextPrime(2 * n);
    HashTable ht(dynamicTableSize);
    for (const Record& r : data) ht.insert(r);

    // ── Build per-case search key sets ──
    // Unlike chaining (where depth = chain length), linear-probing depth is
    // the PROBE DISTANCE: slots walked from the home slot until the key is hit.
    //   Best  -> keys found at their home slot       (1 probe,        O(1))
    //   Worst -> keys with the maximum probe distance (O(longest run))
    //   Avg   -> every dataset key once
    // Spreading every case across all home slots keeps cache behaviour the
    // same, so timing reflects probe depth rather than cache locality.
    int maxProbe = 1;
    for (const Record& r : data) {
        int p = ht.probeLength(r.key);
        if (p > maxProbe) maxProbe = p;
    }

    vector<long long> bestPool, worstPool;
    for (const Record& r : data) {
        int p = ht.probeLength(r.key);
        if (p == 1)        bestPool.push_back(r.key);
        if (p == maxProbe) worstPool.push_back(r.key);
    }
    // Safety: a perfectly collision-free table has no "worst" beyond best.
    if (worstPool.empty()) worstPool = bestPool;
    if (bestPool.empty())  bestPool  = worstPool;

    vector<long long> bestKeys(n), avgKeys(n), worstKeys(n);
    for (int i = 0; i < n; i++) {
        bestKeys[i]  = bestPool[i % bestPool.size()];
        avgKeys[i]   = data[i].key;
        worstKeys[i] = worstPool[i % worstPool.size()];
    }
    fisherYatesShuffle(bestKeys);
    fisherYatesShuffle(avgKeys);
    fisherYatesShuffle(worstKeys);

    volatile bool dummy = false;

    // ── BEST CASE: n searches over home-slot keys ──
    auto t0 = high_resolution_clock::now();
    for (int i = 0; i < n; i++) dummy = dummy ^ ht.search(bestKeys[i]);
    auto t1 = high_resolution_clock::now();
    double bestTime = duration<double>(t1 - t0).count();

    // ── AVERAGE CASE: n searches over all shuffled keys ──
    auto t2 = high_resolution_clock::now();
    for (int i = 0; i < n; i++) dummy = dummy ^ ht.search(avgKeys[i]);
    auto t3 = high_resolution_clock::now();
    double avgTime = duration<double>(t3 - t2).count();

    // ── WORST CASE: n searches over longest-probe keys ──
    auto t4 = high_resolution_clock::now();
    for (int i = 0; i < n; i++) dummy = dummy ^ ht.search(worstKeys[i]);
    auto t5 = high_resolution_clock::now();
    double worstTime = duration<double>(t5 - t4).count();

    // ── Output ──────────────────────────────────────────
    string base = csvFile;
    size_t sl = base.find_last_of("/\\"); if (sl != string::npos) base = base.substr(sl + 1);
    size_t dt = base.find_last_of('.');   if (dt != string::npos) base = base.substr(0, dt);

    MAKE_DIR("outputs");
    string outFile = "outputs/hash_table_search_linear_" + base + ".txt";

    string bestLine  = "Best case time: "    + fmtTime(bestTime)  + " seconds";
    string avgLine   = "Average case time: " + fmtTime(avgTime)   + " seconds";
    string worstLine = "Worst case time: "   + fmtTime(worstTime) + " seconds";

    cout << bestLine  << endl;
    cout << avgLine   << endl;
    cout << worstLine << endl;

    ofstream fout(outFile);
    fout << bestLine << "\n" << avgLine << "\n" << worstLine << "\n";
    fout.close();

    cout << "\nOutput written to: " << outFile << endl;
    return 0;
}
