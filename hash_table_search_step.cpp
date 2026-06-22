// *********************************************************
// Program: hash_table_search_step.cpp
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
// Collision resolution: SEPARATE CHAINING using a balanced AVL
// binary search tree per bucket. This step program traces the search
// path: the hash index, then each comparison made while descending the
// bucket's AVL tree (go left / go right) until the key is found or a
// dead end (-1) is reached.
// *********************************************************

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <direct.h>

using namespace std;

#define MAKE_DIR(d) _mkdir(d)

struct Record {
    long long key;
    string str;
};

// ─── AVL Tree Node ───────────────────────────────────────
struct AVLNode {
    Record data;
    AVLNode* left;
    AVLNode* right;
    int height;
    AVLNode(Record r) : data(r), left(nullptr), right(nullptr), height(1) {}
};

// ─── AVL helper routines ─────────────────────────────────
static int height(AVLNode* n) { return n ? n->height : 0; }

static void updateHeight(AVLNode* n) {
    int hl = height(n->left), hr = height(n->right);
    n->height = 1 + (hl > hr ? hl : hr);
}

static int balanceFactor(AVLNode* n) { return n ? height(n->left) - height(n->right) : 0; }

static AVLNode* rotateRight(AVLNode* y) {
    AVLNode* x = y->left;
    AVLNode* t2 = x->right;
    x->right = y;
    y->left = t2;
    updateHeight(y);
    updateHeight(x);
    return x;
}

static AVLNode* rotateLeft(AVLNode* x) {
    AVLNode* y = x->right;
    AVLNode* t2 = y->left;
    y->left = x;
    x->right = t2;
    updateHeight(x);
    updateHeight(y);
    return y;
}

static AVLNode* avlInsert(AVLNode* node, Record r) {
    if (!node) return new AVLNode(r);
    if (r.key < node->data.key)      node->left  = avlInsert(node->left, r);
    else if (r.key > node->data.key) node->right = avlInsert(node->right, r);
    else return node;                          // duplicate key -> ignore

    updateHeight(node);
    int bf = balanceFactor(node);

    if (bf > 1 && r.key < node->left->data.key)  return rotateRight(node);                 // LL
    if (bf < -1 && r.key > node->right->data.key) return rotateLeft(node);                 // RR
    if (bf > 1 && r.key > node->left->data.key) {                                          // LR
        node->left = rotateLeft(node->left);
        return rotateRight(node);
    }
    if (bf < -1 && r.key < node->right->data.key) {                                        // RL
        node->right = rotateRight(node->right);
        return rotateLeft(node);
    }
    return node;
}

static void avlDestroy(AVLNode* n) {
    if (!n) return;
    avlDestroy(n->left);
    avlDestroy(n->right);
    delete n;
}

// Trace the descent through a bucket's AVL tree, logging every comparison.
static bool avlSearchStep(AVLNode* node, long long tgt, vector<string>& log) {
    while (node) {
        if (tgt == node->data.key) {
            log.push_back(to_string(tgt) + " = " + to_string(node->data.key) + "/" + node->data.str);
            return true;
        }
        if (tgt < node->data.key) {
            log.push_back(to_string(tgt) + " < " + to_string(node->data.key) + "/" + node->data.str + "  -> go left");
            node = node->left;
        } else {
            log.push_back(to_string(tgt) + " > " + to_string(node->data.key) + "/" + node->data.str + "  -> go right");
            node = node->right;
        }
    }
    return false;
}

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

// ─── Hash Table (AVL-tree chaining) ──────────────────────
struct HashSlot {
    AVLNode* root;
    HashSlot() : root(nullptr) {}
    ~HashSlot() { avlDestroy(root); }
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
        table[i].root = avlInsert(table[i].root, r);
    }

    bool search(long long tgt, vector<string>& log, bool& notFoundLogged) const {
        int idx = hf(tgt);
        log.push_back("hash(" + to_string(tgt) + ") = " + to_string(idx));

        if (!table[idx].root) {
            log.push_back("-1 != " + to_string(tgt));   // empty bucket
            notFoundLogged = true;
            return false;
        }
        notFoundLogged = false;
        return avlSearchStep(table[idx].root, tgt, log);
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
        if (line.empty()) continue;
        size_t c = line.find(',');
        if (c == string::npos) continue;

        Record r;
        r.key = atoll(line.substr(0, c).c_str());
        r.str = line.substr(c + 1);
        if (!r.str.empty() && r.str.back() == '\r') r.str.pop_back();
        d.push_back(r);
    }
    return d;
}

int main(int argc, char* argv[]) {
    // Inputs (hardcoded constants; override the target via argv[2]).
    string csvFile = "datasets/dataset_2000.csv";
    long long target = 1000000038LL; // change to a found / not-found key

    if (argc >= 2) csvFile = argv[1];
    if (argc >= 3) target = atoll(argv[2]);

    vector<Record> data = loadCSV(csvFile);
    if (data.empty()) {
        cerr << "No data loaded." << endl;
        return 1;
    }

    // Dynamic hash table sizing matching the main search program.
    int dynamicTableSize = nextPrime((int)data.size());
    HashTable ht(dynamicTableSize);
    for (const Record& r : data) ht.insert(r);

    vector<string> steps;
    bool notFoundLogged = false;
    bool found = ht.search(target, steps, notFoundLogged);

    // Build output filename from csv base name.
    string base = csvFile;
    size_t sl = base.find_last_of("/\\");
    if (sl != string::npos) base = base.substr(sl + 1);
    size_t dt = base.find_last_of('.');
    if (dt != string::npos) base = base.substr(0, dt);

    MAKE_DIR("outputs");
    string outFile = "outputs/" + base + "_hash_table_search_step_" + to_string(target) + ".txt";

    ofstream fout(outFile);
    for (const string& s : steps) {
        cout << s << "\n";
        fout << s << "\n";
    }

    // Reaching a null child during descent also means "not found".
    if (!found && !notFoundLogged) {
        string nf = "-1 != " + to_string(target);
        cout << nf << "\n";
        fout << nf << "\n";
    }

    fout.close();
    cout << "\nOutput written to: " << outFile << endl;
    return 0;
}
