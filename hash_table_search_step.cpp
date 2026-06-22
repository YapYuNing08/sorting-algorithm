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
// Collision resolution: SEPARATE CHAINING where each bucket is a
// pointer-based AVL balanced binary search tree. This program traces the
// search path for one target: starting at the root of the target's bucket
// tree it compares the integer key and walks left/right. A DIRECT hit prints
// a single "target = key/string" line; a deeper hit prints one
// "key/string != target" line for each node compared before the match; a
// not-found search prints those comparison lines then "-1 != target".
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

// ─── AVL balanced BST node (separate-chaining bucket) ────
struct Node {
    Record info;
    Node* left;
    Node* right;
    int height;
};

// ─── Prime helpers (size the table to the next prime >= n) ─
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

// ─── AVL tree bucket ─────────────────────────────────────
class AVLTree {
private:
    Node* root;

    int height(Node* n) const { return n ? n->height : 0; }

    int balanceFactor(Node* n) const {
        return n ? height(n->left) - height(n->right) : 0;
    }

    void updateHeight(Node* n) {
        int hl = height(n->left), hr = height(n->right);
        n->height = 1 + (hl > hr ? hl : hr);
    }

    Node* rotateRight(Node* y) {
        Node* x = y->left;
        Node* t2 = x->right;
        x->right = y;
        y->left = t2;
        updateHeight(y);
        updateHeight(x);
        return x;
    }

    Node* rotateLeft(Node* x) {
        Node* y = x->right;
        Node* t2 = y->left;
        y->left = x;
        x->right = t2;
        updateHeight(x);
        updateHeight(y);
        return y;
    }

    Node* insertNode(Node* node, const Record& element) {
        if (node == nullptr) {
            Node* newNode = new Node;
            newNode->info = element;
            newNode->left = newNode->right = nullptr;
            newNode->height = 1;
            return newNode;
        }
        if (element.key < node->info.key)
            node->left = insertNode(node->left, element);
        else if (element.key > node->info.key)
            node->right = insertNode(node->right, element);
        else
            return node; // duplicate key: ignore

        updateHeight(node);
        int bf = balanceFactor(node);

        if (bf > 1 && element.key < node->left->info.key)
            return rotateRight(node);
        if (bf < -1 && element.key > node->right->info.key)
            return rotateLeft(node);
        if (bf > 1 && element.key > node->left->info.key) {
            node->left = rotateLeft(node->left);
            return rotateRight(node);
        }
        if (bf < -1 && element.key < node->right->info.key) {
            node->right = rotateRight(node->right);
            return rotateLeft(node);
        }
        return node;
    }

    void destroy(Node* n) {
        if (n) {
            destroy(n->left);
            destroy(n->right);
            delete n;
        }
    }

public:
    AVLTree() : root(nullptr) {}
    ~AVLTree() { destroy(root); }

    void insert(const Record& element) { root = insertNode(root, element); }

    // Trace the search path through this bucket tree into log. Returns true if
    // the target is found. Each non-matching node compared is logged as
    // "key/string != target"; the match is logged as "target = key/string".
    bool searchPath(long long target, vector<string>& log) const {
        Node* ptr = root;
        while (ptr != nullptr) {
            if (target == ptr->info.key) {
                log.push_back(to_string(target) + " = " +
                              to_string(ptr->info.key) + "/" + ptr->info.str);
                return true;
            }
            log.push_back(to_string(ptr->info.key) + "/" + ptr->info.str +
                          " != " + to_string(target));
            if (target < ptr->info.key)
                ptr = ptr->left;
            else
                ptr = ptr->right;
        }
        return false;
    }
};

// ─── Hash table with AVL-tree buckets ────────────────────
class HashTable {
private:
    vector<AVLTree> table;
public:
    HashTable(int size) { table.resize(size); }

    int hashfunction(long long key) const {
        return (int)(key % (long long)table.size());
    }

    void insert(const Record& newItem) {
        int location = hashfunction(newItem.key);
        table[location].insert(newItem);
    }

    // Trace a search for target. Logs the bucket-tree path; if the key is not
    // found, appends the "-1 != target" verdict.
    bool search(long long target, vector<string>& log) const {
        int location = hashfunction(target);
        bool found = table[location].searchPath(target, log);
        if (!found)
            log.push_back("-1 != " + to_string(target));
        return found;
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
    long long target = 1000000038LL; // change to a found / not-found key

    if (argc >= 2)
        csvFile = argv[1];
    if (argc >= 3)
        target = atoll(argv[2]);

    vector<Record> data = loadCSV(csvFile);
    if (data.empty()) {
        cerr << "No data loaded." << endl;
        return 1;
    }

    // Table size = next prime >= n, matching hash_table_search.cpp.
    int tableSize = nextPrime((int)data.size());
    HashTable ht(tableSize);
    for (const Record& r : data)
        ht.insert(r);

    vector<string> steps;
    ht.search(target, steps);

    // Build output filename from the csv base name:
    //   <dataset>_hash_table_search_step_<target>.txt
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
    fout.close();

    cout << "\nOutput written to: " << outFile << endl;
    return 0;
}
