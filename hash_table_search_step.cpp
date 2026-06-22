// *********************************************************
// Program: hash_table_search_step.cpp
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
// This program traces the search path: it hashes the target to a bucket
// then walks that bucket's linked list, comparing the integer key of each
// node until the target is found or the chain ends.
// *********************************************************

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <direct.h>

using namespace std;

#define MAKE_DIR(d) _mkdir(d)

struct Record {
    long long key;   // integer field (the search key)
    string str;      // string field that travels with the key
};

// ─── Linked list (separate-chaining bucket) ──────────────
// Derived from the lecturer's LinkedList.cpp: a singly linked list with
// insertFront() and a linear search. Here the search logs every comparison
// so the search path can be shown step by step.
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

    // Walk the chain comparing the integer field of each node. Each step is
    // appended to log. Returns true if the target is found.
    //   found:     "target = key/str"
    //   mismatch:  "key/str != target"   (one line per node compared)
    bool findStep(long long target, vector<string>& log) const {
        Node* ptr = start;
        while (ptr != nullptr) {
            if (ptr->info.key == target) {
                log.push_back(to_string(target) + " = " +
                              to_string(ptr->info.key) + "/" + ptr->info.str);
                return true;
            }
            log.push_back(to_string(ptr->info.key) + "/" + ptr->info.str +
                          " != " + to_string(target));
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
const int TABLE_SIZE = 1000003; // prime

class HashTable {
private:
    vector<LinkedList> table;
public:
    HashTable(int size) { table.resize(size); }

    int hashfunction(long long key) const {
        return (int)(key % (long long)table.size());
    }

    void insert(const Record& newItem) {
        int location = hashfunction(newItem.key);
        table[location].insertFront(newItem);
    }

    // Trace the search for target: log the bucket index, then the walk along
    // that bucket's chain. Returns true if found.
    bool searchStep(long long target, vector<string>& log) const {
        int idx = hashfunction(target);
        log.push_back("hash(" + to_string(target) + ") = " + to_string(idx));
        return table[idx].findStep(target, log);
    }
};

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

int main(int argc, char* argv[]) {
    // Inputs
    string csvFile = "datasets/dataset_2000.csv";
    long long target = 1000000038LL; // change to found / not-found key

    if (argc >= 2)
        csvFile = argv[1];
    if (argc >= 3)
        target = stoll(argv[2]);

    vector<Record> data = loadCSV(csvFile);
    if (data.empty()) {
        cerr << "No data loaded." << endl;
        return 1;
    }

    HashTable ht(TABLE_SIZE);
    for (const Record& r : data)
        ht.insert(r);

    vector<string> steps;
    bool found = ht.searchStep(target, steps);

    // Build output filename from csv base name
    string base = csvFile;
    size_t sl = base.find_last_of("/\\");
    if (sl != string::npos)
        base = base.substr(sl + 1);
    size_t dt = base.find_last_of('.');
    if (dt != string::npos)
        base = base.substr(0, dt);

    MAKE_DIR("outputs");
    string outFile = "outputs/" + base + "_hash_table_search_step_" + to_string(target) + ".txt";

    ofstream fout(outFile);
    for (const string& s : steps) {
        cout << s << "\n";
        fout << s << "\n";
    }

    // Not found: target absent from the bucket (empty chain or end reached).
    if (!found) {
        string nf = "-1 != " + to_string(target);
        cout << nf << "\n";
        fout << nf << "\n";
    }

    fout.close();
    cout << "\nOutput written to: " << outFile << endl;
    return 0;
}
