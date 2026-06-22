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

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdint>
#include <direct.h>

using namespace std;

#define MAKE_DIR(d) _mkdir(d)

struct Record {
    long long key;
    string str;
};

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

// Inserts node, and sets 'inserted' to true if it's a new insertion (no duplicates).
AVLNode* avlIns(AVLNode* nd, Record r, bool& inserted) {
    if (!nd) {
        inserted = true;
        return new AVLNode(r);
    }
    if (r.key < nd->data.key)
        nd->left = avlIns(nd->left, r, inserted);
    else if (r.key > nd->data.key)
        nd->right = avlIns(nd->right, r, inserted);
    else
        inserted = false;
    return avlBal(nd);
}

// Recursively deletes AVL tree nodes to prevent memory leaks.
void freeAVL(AVLNode* nd) {
    if (!nd) return;
    freeAVL(nd->left);
    freeAVL(nd->right);
    delete nd;
}

// Helper functions for prime generation
bool isPrime(int num) {
    if (num <= 1) return false;
    if (num <= 3) return true;
    if (num % 2 == 0 || num % 3 == 0) return false;
    for (int i = 5; i * i <= num; i += 6) {
        if (num % i == 0 || num % (i + 2) == 0) return false;
    }
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

// Search with step logging. Returns true if found.
bool avlStep(AVLNode* nd, long long tgt, vector<string>& log) {
    if (!nd)
        return false;

    if (tgt == nd->data.key) {
        log.push_back(to_string(tgt) + " = " + to_string(nd->data.key) + "/" + nd->data.str);
        return true;
    }

    log.push_back(to_string(nd->data.key) + "/" + nd->data.str + " != " + to_string(tgt));

    if (tgt < nd->data.key)
        return avlStep(nd->left, tgt, log);
    else
        return avlStep(nd->right, tgt, log);
}

struct HashSlot {
    AVLNode* root;
    int chainLen;
    HashSlot() : root(nullptr), chainLen(0) {}
    ~HashSlot() {
        freeAVL(root);
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

    ~HashTable() {
        delete[] table;
    }

    int hf(long long k) const {
        return (int)(k % tableSize);
    }

    void insert(Record r) {
        int i = hf(r.key);
        bool inserted = false;
        table[i].root = avlIns(table[i].root, r, inserted);
        if (inserted) {
            table[i].chainLen++;
        }
    }

    bool search(long long tgt, vector<string>& log, bool& notFoundLogged) const {
        int idx = hf(tgt);
        log.push_back("hash(" + to_string(tgt) + ") = " + to_string(idx));

        if (!table[idx].root) {
            log.push_back("-1 != " + to_string(tgt));
            notFoundLogged = true;
            return false;
        }

        notFoundLogged = false;
        return avlStep(table[idx].root, tgt, log);
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
        r.key = atoll(line.substr(0, c).c_str());
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
        target = atoll(argv[2]); // Consistent atoll

    vector<Record> data = loadCSV(csvFile);
    if (data.empty()) {
        cerr << "No data loaded." << endl;
        return 1;
    }

    // Dynamic Hash Table sizing matching the main search file
    int dynamicTableSize = nextPrime((int)data.size());
    HashTable ht(dynamicTableSize);
    for (const Record& r : data)
        ht.insert(r);

    vector<string> steps;
    bool nfl = false;
    bool found = ht.search(target, steps, nfl);

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

    if (!found && !nfl) {
        string nf = "-1 != " + to_string(target);
        cout << nf << "\n";
        fout << nf << "\n";
    }

    fout.close();
    cout << "\nOutput written to: " << outFile << endl;
    return 0;
}
