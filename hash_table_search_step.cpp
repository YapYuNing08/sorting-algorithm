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
// Collision resolution: SEPARATE CHAINING using a singly linked list
// per bucket. This step program traces the search path: the hash index,
// then each comparison made while walking the bucket's linked list until
// the key is found or the chain ends (-1).
// (The array-based vs linked-list-based AVL BST comparison is discussed
//  THEORETICALLY in the report's conclusion only.)
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

// ─── Singly Linked List Node ─────────────────────────────
struct Node {
    Record data;
    Node* next;
    Node(Record r) : data(r), next(nullptr) {}
};

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

// Walk the bucket's linked list, logging every comparison.
bool listStep(Node* curr, long long tgt, vector<string>& log) {
    while (curr) {
        if (tgt == curr->data.key) {
            log.push_back(to_string(tgt) + " = " + to_string(curr->data.key) + "/" + curr->data.str);
            return true;
        }
        log.push_back(to_string(curr->data.key) + "/" + curr->data.str + " != " + to_string(tgt));
        curr = curr->next;
    }
    return false;
}

// ─── Hash Table (linked-list chaining) ───────────────────
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

    bool search(long long tgt, vector<string>& log, bool& notFoundLogged) const {
        int idx = hf(tgt);
        log.push_back("hash(" + to_string(tgt) + ") = " + to_string(idx));

        if (!table[idx].head) {
            log.push_back("-1 != " + to_string(tgt));   // empty bucket
            notFoundLogged = true;
            return false;
        }
        notFoundLogged = false;
        return listStep(table[idx].head, tgt, log);
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

    // Walking off the end of the chain also means "not found".
    if (!found && !notFoundLogged) {
        string nf = "-1 != " + to_string(target);
        cout << nf << "\n";
        fout << nf << "\n";
    }

    fout.close();
    cout << "\nOutput written to: " << outFile << endl;
    return 0;
}
