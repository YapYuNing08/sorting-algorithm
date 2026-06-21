// *********************************************************
// Program: hash_table_search.cpp
// Course: CCP6214 Algorithm Design and Analysis
// Lecture Class: TC4L
// Tutorial Class: T13L
// Trimester: 2610
// Member_1: ID | NAME | EMAIL | PHONE
// Member_2: ID | NAME | EMAIL | PHONE
// Member_3: ID | NAME | EMAIL | PHONE
// Member_4: ID | NAME | EMAIL | PHONE
// *********************************************************
// Task Distribution
// Member_1:
// Member_2:
// Member_3:
// Member_4:
// *********************************************************

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <chrono>
#include <algorithm>
#include <cstdlib>
#include <direct.h>

using namespace std;
using namespace chrono;

#define MAKE_DIR(d) _mkdir(d)

struct Record {
    long long key;
    string str;
};

// ─── AVL Tree ────────────────────────────────────────────
struct AVLNode {
    Record data;
    AVLNode* left;
    AVLNode* right;
    int height;
    AVLNode(Record r) : data(r), left(nullptr), right(nullptr), height(1) {}
};

int avlH(AVLNode* n) {
    return n ? n->height : 0;
}

void avlUpd(AVLNode* n) {
    if (n)
        n->height = 1 + max(avlH(n->left), avlH(n->right));
}

AVLNode* rotR(AVLNode* y) {
    AVLNode* x = y->left;
    AVLNode* T = x->right;
    x->right = y;
    y->left = T;
    avlUpd(y);
    avlUpd(x);
    return x;
}

AVLNode* rotL(AVLNode* x) {
    AVLNode* y = x->right;
    AVLNode* T = y->left;
    y->left = x;
    x->right = T;
    avlUpd(x);
    avlUpd(y);
    return y;
}

AVLNode* avlBal(AVLNode* n) {
    avlUpd(n);
    int bf = avlH(n->left) - avlH(n->right);
    if (bf > 1) {
        if (avlH(n->left->left) < avlH(n->left->right))
            n->left = rotL(n->left);
        return rotR(n);
    }
    if (bf < -1) {
        if (avlH(n->right->right) < avlH(n->right->left))
            n->right = rotR(n->right);
        return rotL(n);
    }
    return n;
}

AVLNode* avlIns(AVLNode* nd, Record r) {
    if (!nd)
        return new AVLNode(r);
    if (r.key < nd->data.key)
        nd->left = avlIns(nd->left, r);
    else if (r.key > nd->data.key)
        nd->right = avlIns(nd->right, r);
    return avlBal(nd);
}

bool avlSearch(AVLNode* nd, long long tgt) {
    while (nd) {
        if (tgt == nd->data.key)
            return true;
        else if (tgt < nd->data.key)
            nd = nd->left;
        else
            nd = nd->right;
    }
    return false;
}

// Hash Table
const int TABLE_SIZE = 1000003; // prime

struct HashSlot {
    AVLNode* root;
    int chainLen;
    HashSlot() : root(nullptr), chainLen(0) {}
};

class HashTable {
public:
    HashSlot* table;

    HashTable() {
        table = new HashSlot[TABLE_SIZE];
    }

    ~HashTable() {
        delete[] table;
    }

    int hf(long long k) const {
        return (int)(k % TABLE_SIZE);
    }

    void insert(Record r) {
        int i = hf(r.key);
        table[i].root = avlIns(table[i].root, r);
        table[i].chainLen++;
    }

    bool search(long long tgt) const {
        int i = hf(tgt);
        if (!table[i].root)
            return false;
        return avlSearch(table[i].root, tgt);
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

// Fisher-Yates shuffle using rand() (matches dataset_generator seed style)
void fisherYatesShuffle(vector<long long>& arr) {
    for (int i = (int)arr.size() - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        long long tmp = arr[i];
        arr[i] = arr[j];
        arr[j] = tmp;
    }
}

int main(int argc, char* argv[]) {
    // Leader ID: 242UC244KD -> 2421324414
    srand((unsigned int)2421324414U);

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
    HashTable ht;
    for (const Record& r : data)
        ht.insert(r);

    // Identify case keys
    // Best case : bucket with exactly 1 element (single root hit, O(1))
    long long bestKey = data[0].key;
    for (const Record& r : data) {
        if (ht.table[ht.hf(r.key)].chainLen == 1) {
            bestKey = r.key;
            break;
        }
    }

    // Worst case: deepest node in the bucket with the longest chain
    int maxChain = 0, maxIdx = 0;
    for (int i = 0; i < TABLE_SIZE; i++) {
        if (ht.table[i].chainLen > maxChain) {
            maxChain = ht.table[i].chainLen;
            maxIdx = i;
        }
    }
    long long worstKey = 0;
    {
        AVLNode* cur = ht.table[maxIdx].root;
        while (cur) {
            worstKey = cur->data.key;
            cur = cur->right;
        }
    }

    // Average: shuffle all keys for random search order
    vector<long long> searchKeys(n);
    for (int i = 0; i < n; i++)
        searchKeys[i] = data[i].key;
    fisherYatesShuffle(searchKeys);

    // BEST CASE: n searches on single-element bucket key
    auto t0 = high_resolution_clock::now();
    for (int i = 0; i < n; i++)
        ht.search(bestKey);
    auto t1 = high_resolution_clock::now();
    double bestTime = duration<double>(t1 - t0).count();

    // AVERAGE CASE: n searches on shuffled existing keys
    auto t2 = high_resolution_clock::now();
    for (int i = 0; i < n; i++)
        ht.search(searchKeys[i]);
    auto t3 = high_resolution_clock::now();
    double avgTime = duration<double>(t3 - t2).count();

    // WORST CASE: n searches on deepest-chain key
    auto t4 = high_resolution_clock::now();
    for (int i = 0; i < n; i++)
        ht.search(worstKey);
    auto t5 = high_resolution_clock::now();
    double worstTime = duration<double>(t5 - t4).count();

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
