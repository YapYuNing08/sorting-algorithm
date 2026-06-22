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
// Collision resolution: SEPARATE CHAINING where each bucket is a
// pointer-based AVL balanced binary search tree (a "linked-list based AVL
// BST"). insert() places a record into its bucket's AVL tree keyed on the
// integer field; a search is therefore either a DIRECT hit (key found at the
// root of the bucket) or a TREE traversal that compares the integer key and
// goes left/right until the key is found or a null link is reached. Because
// every bucket tree is height-balanced, the longest search path in a bucket
// holding m keys is O(log m) instead of O(m) as it would be for a plain list.
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

// ─── AVL balanced BST (separate-chaining bucket) ─────────
// A pointer-based ("linked") AVL tree specialised to store Record and to
// order/compare on the integer key. Insertion rebalances with single/double
// rotations so the tree stays height-balanced. find() walks from the root
// comparing keys and reports the number of key comparisons performed via
// cmp (this is the search-path length used for the best/average/worst study).
struct Node {
    Record info;
    Node* left;
    Node* right;
    int height;
};

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

        // Left-Left
        if (bf > 1 && element.key < node->left->info.key)
            return rotateRight(node);
        // Right-Right
        if (bf < -1 && element.key > node->right->info.key)
            return rotateLeft(node);
        // Left-Right
        if (bf > 1 && element.key > node->left->info.key) {
            node->left = rotateLeft(node->left);
            return rotateRight(node);
        }
        // Right-Left
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

    // Tree search. Returns true if the key is found, and reports the number of
    // key comparisons performed via cmp (1 = direct hit at the root).
    bool find(long long target, int& cmp) const {
        cmp = 0;
        Node* ptr = root;
        while (ptr != nullptr) {
            cmp++;
            if (target == ptr->info.key)
                return true;
            else if (target < ptr->info.key)
                ptr = ptr->left;
            else
                ptr = ptr->right;
        }
        return false;
    }
};

// ─── Hash table with AVL-tree buckets ────────────────────
// A vector of AVLTree buckets with hashfunction(key) = key % table.size().
// The table size is the next prime >= n so the average load factor stays ~1.
class HashTable {
private:
    vector<AVLTree> table;
public:
    HashTable(int size) { table.resize(size); }

    int hashfunction(long long key) const {
        return (int)(key % (long long)table.size());
    }

    int size() const { return (int)table.size(); }

    void insert(const Record& newItem) {
        int location = hashfunction(newItem.key);
        table[location].insert(newItem);
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

// Helper: primality test, used to size the table to the next prime >= n.
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

    // Build hash table (not timed). Table size = next prime >= n keeps the
    // average load factor ~1 so most buckets are small balanced trees.
    int tableSize = nextPrime(n);
    HashTable ht(tableSize);
    for (const Record& r : data)
        ht.insert(r);

    // ─── Classify each key by its actual search-path length (not timed) ──
    // For every existing key we compute how many comparisons a successful
    // search performs. With AVL-tree buckets the cost is the key's depth in
    // its bucket tree (1 = direct hit at the root). We then pick one
    // representative key for each case:
    //   best    -> shortest path  (minimum comparisons, root of a bucket)
    //   worst   -> longest path   (maximum comparisons, deepest tree node)
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
    cout << "Table size = " << tableSize
         << "  |  path length (comparisons)  best=" << minCost
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
