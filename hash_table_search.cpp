// *********************************************************
// Program: hash_table_search.cpp
// Course: CCP6214 Algorithm Design and Analysis
// Lecture Class: TC2L
// Tutorial Class: TT8L
// Trimester: 2610
// Member_1: 242UC244KB | KOK HUEY HUEY | KOK.HUEY.HUEY@student.mmu.edu.my | 0162011560
// Member_2: 242UC244KD | LIM JOEY | LIM.JOEY@student.mmu.edu.my | 0192270150
// Member_3: 242UC242LB | YAP SHEN YEE | YAP.SHEN.YEE@student.mmu.edu.my | 0162897881
// Member_4: 242UC244KC | YAP YU NING | YAP.YU.NING@student.mmu.edu.my | 0122293817
// *********************************************************
// Task Distribution
// Member_1: Radix sort algorithm
// Member_2: Heap sort algorithm
// Member_3: Hash table search algorithm
// Member_4: Dataset generator
// *********************************************************
// Collision resolution: SEPARATE CHAINING with a singly linked list per
// bucket (adapted from the lecturer's HashTable.cpp / LinkedList.cpp).
// Each bucket is a LinkedList; insert() puts a record at the front of its
// bucket's list, and retrieve() scans that list comparing the integer key.
// *********************************************************

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <chrono>
#include <climits>
#include <cmath>
#include <direct.h>

using namespace std;
using namespace chrono;

#define MAKE_DIR(d) _mkdir(d)

struct Record {
    long long key;   // integer field (the search key)
    string str;      // string field that travels with the key
};

// ─── Linked list (separate-chaining bucket) ──────────────
// Derived from the lecturer's LinkedList.cpp: a singly linked list with
// insertFront() and a linear find(). Specialised to store Record and to
// compare on the integer key.
struct Node {
    Record info;
    Node* next;
};

class LinkedList {
private:
    Node* start;
public:
    LinkedList() { start = nullptr; }
    ~LinkedList() { makeEmpty(); }

    // insert at the beginning of the linked list
    void insertFront(const Record& element) {
        Node* newNode = new Node;
        newNode->info = element;
        newNode->next = start;
        start = newNode;
    }

    // Linear search along the chain. Returns true if the key is found, and
    // reports the number of key comparisons performed via cmp (this is the
    // search-path length used for the best/average/worst analysis).
    bool find(long long target, int& cmp) const {
        cmp = 0;
        Node* ptr = start;
        while (ptr != nullptr) {
            cmp++;
            if (ptr->info.key == target)
                return true;
            ptr = ptr->next;
        }
        return false;
    }

    void makeEmpty() {
        while (start != nullptr) {
            Node* ptr = start;
            start = start->next;
            delete ptr;
        }
    }
};

// ─── Hash table with chaining ────────────────────────────
// Derived from the lecturer's HashTable.cpp: a vector of LinkedList buckets
// with hashfunction(key) = key % table.size().
const int TABLE_SIZE = 1000003; // prime

class HashTable {
private:
    vector<LinkedList> table;
public:
    HashTable(int size) { table.resize(size); }

    int hashfunction(long long key) const {
        return (int)(key % (long long)table.size());
    }

    int size() const { return (int)table.size(); }

    void insert(const Record& newItem) {
        int location = hashfunction(newItem.key);
        table[location].insertFront(newItem);
    }

    // Returns the number of key comparisons a search for target performs
    // (0 if the bucket is empty). Used both to classify each key's path
    // length and as the timed workload (the count is accumulated into a
    // volatile sink so the optimizer cannot delete the search calls).
    int searchCost(long long target) const {
        int location = hashfunction(target);
        int cmp = 0;
        table[location].find(target, cmp);
        return cmp;
    }
};

// Load CSV
vector<Record> loadCSV(const string& fn) {
    vector<Record> d;
    ifstream fin(fn);
    if (!fin.is_open()) {
        cerr << "Error: cannot open " << fn << endl;
        return d;
    }

    string line;
    while (getline(fin, line)) {
        if (line.empty())
            continue;

        size_t c = line.find(',');
        if (c == string::npos)
            continue;

        Record r;
        r.key = stoll(line.substr(0, c));
        r.str = line.substr(c + 1);
        if (!r.str.empty() && r.str.back() == '\r')
            r.str.pop_back();
        d.push_back(r);
    }
    return d;
}

string fmtTime(double t) {
    ostringstream oss;
    oss << fixed << setprecision(9) << t;
    return oss.str();
}

int main(int argc, char* argv[]) {
    string csvFile = "datasets/dataset_2000.csv";
    if (argc >= 2)
        csvFile = argv[1];

    // Load (not timed)
    vector<Record> data = loadCSV(csvFile);
    if (data.empty()) {
        cerr << "No data loaded." << endl;
        return 1;
    }
    int n = (int)data.size();
    cout << "Dataset loaded: " << n << " elements" << endl;

    // Build hash table (not timed)
    HashTable ht(TABLE_SIZE);
    for (const Record& r : data)
        ht.insert(r);

    // ─── Classify each key by its actual search-path length (not timed) ──
    // For every existing key we compute how many comparisons a successful
    // search performs. With chaining the cost is the key's position in its
    // bucket's linked list (1 = at the front of the chain). We then pick one
    // representative key for each case:
    //   best    -> shortest path  (minimum comparisons, front of a chain)
    //   worst   -> longest path   (maximum comparisons, end of longest chain)
    //   average -> path closest to the mean over all keys
    // Repeating ONE representative key per case keeps every case equally
    // cache-hot, so the measured times differ only by path length and the
    // ordering best <= average <= worst holds by construction.
    long long bestKey = data[0].key, worstKey = data[0].key, avgKey = data[0].key;
    int minCost = INT_MAX, maxCost = 0;
    long long totalCost = 0;
    for (const Record& r : data) {
        int c = ht.searchCost(r.key);
        totalCost += c;
        if (c < minCost) { minCost = c; bestKey = r.key; }
        if (c > maxCost) { maxCost = c; worstKey = r.key; }
    }
    double meanCost = (double)totalCost / n;
    double bestDelta = 1e18;
    for (const Record& r : data) {
        double d = fabs((double)ht.searchCost(r.key) - meanCost);
        if (d < bestDelta) { bestDelta = d; avgKey = r.key; }
    }
    cout << "Path length (comparisons)  best=" << minCost
         << "  avg~=" << meanCost << "  worst=" << maxCost << endl;

    // volatile sink: forces the optimizer to actually execute every search
    // (the comparison count is consumed) instead of deleting the loop.
    volatile long long sink = 0;

    // BEST CASE: n searches on the shortest-path key
    auto t0 = high_resolution_clock::now();
    for (int i = 0; i < n; i++)
        sink += ht.searchCost(bestKey);
    auto t1 = high_resolution_clock::now();
    double bestTime = duration<double>(t1 - t0).count() / n;

    // AVERAGE CASE: n searches on the mean-path key
    auto t2 = high_resolution_clock::now();
    for (int i = 0; i < n; i++)
        sink += ht.searchCost(avgKey);
    auto t3 = high_resolution_clock::now();
    double avgTime = duration<double>(t3 - t2).count() / n;

    // WORST CASE: n searches on the longest-path key
    auto t4 = high_resolution_clock::now();
    for (int i = 0; i < n; i++)
        sink += ht.searchCost(worstKey);
    auto t5 = high_resolution_clock::now();
    double worstTime = duration<double>(t5 - t4).count() / n;

    // Output
    string base = csvFile;
    size_t sl = base.find_last_of("/\\");
    if (sl != string::npos)
        base = base.substr(sl + 1);
    size_t dt = base.find_last_of('.');
    if (dt != string::npos)
        base = base.substr(0, dt);

    MAKE_DIR("outputs");
    string outFile = "outputs/hash_table_search_" + base + ".txt";

    string bestLine  = "Best case time: "    + fmtTime(bestTime)  + " seconds";
    string avgLine   = "Average case time: " + fmtTime(avgTime)   + " seconds";
    string worstLine = "Worst case time: "   + fmtTime(worstTime) + " seconds";

    cout << bestLine << endl;
    cout << avgLine << endl;
    cout << worstLine << endl;

    ofstream fout(outFile);
    fout << bestLine << "\n" << avgLine << "\n" << worstLine << "\n";
    fout.close();

    cout << "\nOutput written to: " << outFile << endl;
    return 0;
}
