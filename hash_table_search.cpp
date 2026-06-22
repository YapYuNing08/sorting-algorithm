// *********************************************************
// Program: hash_table_search.cpp
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
// Collision resolution: SEPARATE CHAINING using a singly linked list
// per bucket. Each table slot holds the head of a linked list of records
// whose integer keys hashed to that slot.
// (The array-based vs linked-list-based AVL BST comparison is discussed
//  THEORETICALLY in the report's conclusion only; the implemented table
//  uses linked-list chaining.)
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

// ─── Singly Linked List Node ─────────────────────────────
struct Node {
    Record data;
    Node* next;
    Node(Record r) : data(r), next(nullptr) {}
};

// ─── Hash Table (linked-list chaining) ───────────────────
struct HashSlot {
    Node* head;
    int chainLen;
    HashSlot() : head(nullptr), chainLen(0) {}
    ~HashSlot() {
        Node* curr = head;
        while (curr) {
            Node* temp = curr->next;
            delete curr;
            curr = temp;
        }
    }
};

class HashTable {
public:
    HashSlot* table;
    int tableSize;

    HashTable(int size) {
        tableSize = size;
        table = new HashSlot[tableSize];
    }
    ~HashTable() { delete[] table; }

    int hf(long long k) const { return (int)(k % tableSize); }

    void insert(Record r) {
        int i = hf(r.key);
        Node* curr = table[i].head;
        while (curr) {                      // skip duplicate keys
            if (curr->data.key == r.key) return;
            curr = curr->next;
        }
        Node* newNode = new Node(r);
        newNode->next = table[i].head;      // prepend
        table[i].head = newNode;
        table[i].chainLen++;
    }

    bool search(long long tgt) const {
        Node* curr = table[hf(tgt)].head;
        while (curr) {
            if (tgt == curr->data.key) return true;
            curr = curr->next;
        }
        return false;
    }
};

// ─── Prime helpers (table size = next prime >= n) ────────
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

// Last node in a chain (the deepest element = worst-case search position).
long long tailKey(Node* head) {
    while (head->next) head = head->next;
    return head->data.key;
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
    // Table size = next prime >= n keeps the average chain length near 1.
    int dynamicTableSize = nextPrime(n);
    HashTable ht(dynamicTableSize);
    for (const Record& r : data) ht.insert(r);

    // ── Build per-case search key sets (one key per non-empty bucket) ──
    // All three sets are spread across the whole table, so they share the
    // SAME cache behaviour; the only difference is the search DEPTH:
    //   Best  -> chain HEAD key  (1 comparison,            O(1))
    //   Worst -> chain TAIL key  (chainLen comparisons,    O(chainLen))
    //   Avg   -> every dataset key once
    // (If we instead searched a single key repeatedly, that key would stay
    //  hot in cache and read FASTER than the cache-cold average sweep — a
    //  misleading ordering. Spreading every case across all buckets removes
    //  that artifact so the timing reflects chain depth, not cache locality.)
    vector<long long> bestPool, worstPool;
    for (int i = 0; i < dynamicTableSize; i++) {
        Node* head = ht.table[i].head;
        if (!head) continue;
        bestPool.push_back(head->data.key);   // first node in chain
        worstPool.push_back(tailKey(head));   // last node in chain
    }

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

    // ── BEST CASE: n searches over chain-head keys ──
    auto t0 = high_resolution_clock::now();
    for (int i = 0; i < n; i++) dummy = dummy ^ ht.search(bestKeys[i]);
    auto t1 = high_resolution_clock::now();
    double bestTime = duration<double>(t1 - t0).count();

    // ── AVERAGE CASE: n searches over all shuffled keys ──
    auto t2 = high_resolution_clock::now();
    for (int i = 0; i < n; i++) dummy = dummy ^ ht.search(avgKeys[i]);
    auto t3 = high_resolution_clock::now();
    double avgTime = duration<double>(t3 - t2).count();

    // ── WORST CASE: n searches over chain-tail keys ──
    auto t4 = high_resolution_clock::now();
    for (int i = 0; i < n; i++) dummy = dummy ^ ht.search(worstKeys[i]);
    auto t5 = high_resolution_clock::now();
    double worstTime = duration<double>(t5 - t4).count();

    // ── Output ──────────────────────────────────────────
    string base = csvFile;
    size_t sl = base.find_last_of("/\\"); if (sl != string::npos) base = base.substr(sl + 1);
    size_t dt = base.find_last_of('.');   if (dt != string::npos) base = base.substr(0, dt);

    MAKE_DIR("outputs");
    string outFile = "outputs/hash_table_search_" + base + ".txt";

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
