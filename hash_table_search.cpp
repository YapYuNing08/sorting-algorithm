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


#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <vector>
#include <chrono>
#include <algorithm>
#include <cstdlib>
#include <sstream>
#include <direct.h>

using namespace std;
using namespace chrono;

#define MAKE_DIR(d) _mkdir(d)

struct Record { long long key; string str; };

// ─── AVL Tree ────────────────────────────────────────────
struct AVLNode {
    Record data; AVLNode* left; AVLNode* right; int height;
    AVLNode(Record r) : data(r), left(nullptr), right(nullptr), height(1) {}
};

int avlH(AVLNode* n) { return n ? n->height : 0; }
void avlUpd(AVLNode* n) { if(n) n->height=1+max(avlH(n->left),avlH(n->right)); }

AVLNode* rotR(AVLNode* y) {
    AVLNode* x=y->left; AVLNode* T=x->right;
    x->right=y; y->left=T; avlUpd(y); avlUpd(x); return x;
}
AVLNode* rotL(AVLNode* x) {
    AVLNode* y=x->right; AVLNode* T=y->left;
    y->left=x; x->right=T; avlUpd(x); avlUpd(y); return y;
}
AVLNode* avlBal(AVLNode* n) {
    avlUpd(n);
    int bf=avlH(n->left)-avlH(n->right);
    if(bf>1){ if(avlH(n->left->left)<avlH(n->left->right)) n->left=rotL(n->left); return rotR(n); }
    if(bf<-1){ if(avlH(n->right->right)<avlH(n->right->left)) n->right=rotR(n->right); return rotL(n); }
    return n;
}

// Inserts node, and sets 'inserted' to true if it's a new insertion (no duplicates).
AVLNode* avlIns(AVLNode* nd, Record r, bool& inserted) {
    if(!nd) {
        inserted = true;
        return new AVLNode(r);
    }
    if(r.key<nd->data.key) nd->left=avlIns(nd->left,r,inserted);
    else if(r.key>nd->data.key) nd->right=avlIns(nd->right,r,inserted);
    else inserted = false;
    return avlBal(nd);
}

bool avlSearch(AVLNode* nd, long long tgt) {
    while(nd){
        if(tgt==nd->data.key) return true;
        else if(tgt<nd->data.key) nd=nd->left;
        else nd=nd->right;
    }
    return false;
}

bool avlSearchCount(AVLNode* nd, long long tgt, long long& comparisons) {
    while(nd){
        comparisons++;
        if(tgt==nd->data.key) return true;
        else if(tgt<nd->data.key) nd=nd->left;
        else nd=nd->right;
    }
    return false;
}

// Recursively deletes AVL tree nodes to prevent memory leaks.
void freeAVL(AVLNode* nd) {
    if (!nd) return;
    freeAVL(nd->left);
    freeAVL(nd->right);
    delete nd;
}

// ─── Hash Table ──────────────────────────────────────────
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
    int hf(long long k) const { return (int)(k % tableSize); }

    void insert(Record r){
        int i=hf(r.key);
        bool inserted = false;
        table[i].root=avlIns(table[i].root,r,inserted);
        if (inserted) {
            table[i].chainLen++;
        }
    }
    bool search(long long tgt) const {
        int i=hf(tgt);
        if(!table[i].root) return false;
        return avlSearch(table[i].root,tgt);
    }
    long long searchComparisons(long long tgt) const {
        int i=hf(tgt);
        if(!table[i].root) return 1;
        long long comps = 0;
        avlSearchCount(table[i].root, tgt, comps);
        return comps;
    }
};

// Helper function to check if a number is prime.
bool isPrime(int num) {
    if (num <= 1) return false;
    if (num <= 3) return true;
    if (num % 2 == 0 || num % 3 == 0) return false;
    for (int i = 5; i * i <= num; i += 6) {
        if (num % i == 0 || num % (i + 2) == 0) return false;
    }
    return true;
}

// Helper function to find the next prime number.
int nextPrime(int num) {
    if (num <= 2) return 2;
    int p = num;
    if (p % 2 == 0) p++;
    while (true) {
        if (isPrime(p)) return p;
        p += 2;
    }
}

// Find the leaf node at the maximum height of the AVL tree.
long long findDeepestKey(AVLNode* nd) {
    if (!nd) return 0;
    while (nd->left || nd->right) {
        if (avlH(nd->left) > avlH(nd->right)) {
            nd = nd->left;
        } else {
            nd = nd->right;
        }
    }
    return nd->data.key;
}

vector<Record> loadCSV(const string& fn){
    vector<Record> d; ifstream fin(fn);
    if(!fin.is_open()){ cerr<<"Error: cannot open "<<fn<<endl; return d; }
    string line;
    while(getline(fin,line)){
        if(line.empty()) continue;
        size_t c=line.find(','); if(c==string::npos) continue;
        Record r; 
        r.key=atoll(line.substr(0,c).c_str()); 
        r.str=line.substr(c+1);
        if(!r.str.empty()&&r.str.back()=='\r') r.str.pop_back();
        d.push_back(r);
    }
    return d;
}

string fmtTime(double t){
    ostringstream oss; oss<<fixed<<setprecision(9)<<t; return oss.str();
}

// Stress test: deliberately small table size to force real collisions,
// proving worst-case AVL chain traversal is genuinely slower than best case.
// This is a SEPARATE diagnostic from the main dynamic results
// and is NOT representative of the actual hash table used for submission.
void runStressTest(const vector<Record>& data) {
    const int STRESS_TABLE_SIZE = 97; // small prime -> forces deep chains
    cout << "\n[STRESS TEST] Using a deliberately small table size ("
         << STRESS_TABLE_SIZE << ") to force real AVL chain depth.\n"
         << "This block is NOT the actual hash table used for submission.\n";

    HashTable sht(STRESS_TABLE_SIZE);
    for (const Record& r : data) sht.insert(r);

    int maxChain = 0, maxIdx = 0;
    for (int i = 0; i < STRESS_TABLE_SIZE; i++) {
        if (sht.table[i].chainLen > maxChain) {
            maxChain = sht.table[i].chainLen;
            maxIdx = i;
        }
    }
    long long worstKey = 0;
    { AVLNode* cur = sht.table[maxIdx].root; while (cur) { worstKey = cur->data.key; cur = cur->right; } }

    long long bestKey = data[0].key;
    for (const Record& r : data) {
        if (sht.table[sht.hf(r.key)].chainLen == 1) { bestKey = r.key; break; }
    }

    int worstHeight = sht.table[maxIdx].root ? sht.table[maxIdx].root->height : 0;
    cout << "[STRESS TEST] longest chain = " << maxChain
         << ", AVL height = " << worstHeight << "\n";

    int n = (int)data.size();
    volatile bool dummy = false;

    auto t0 = high_resolution_clock::now();
    for (int i = 0; i < n; i++) dummy = dummy ^ sht.search(bestKey);
    auto t1 = high_resolution_clock::now();
    double bestT = duration<double>(t1 - t0).count();

    auto t2 = high_resolution_clock::now();
    for (int i = 0; i < n; i++) dummy = dummy ^ sht.search(worstKey);
    auto t3 = high_resolution_clock::now();
    double worstT = duration<double>(t3 - t2).count();

    cout << "[STRESS TEST] Best case (chain=1) time:  " << fmtTime(bestT)  << " seconds\n";
    cout << "[STRESS TEST] Worst case (chain=" << maxChain << ") time: " << fmtTime(worstT) << " seconds\n";
    cout << "[STRESS TEST] Best case comparisons:  " << sht.searchComparisons(bestKey)  << "\n";
    cout << "[STRESS TEST] Worst case comparisons: " << sht.searchComparisons(worstKey) << "\n";
}

void fisherYatesShuffle(vector<long long>& arr){
    for(int i=(int)arr.size()-1;i>0;i--){
        int j=rand()%(i+1);
        long long tmp=arr[i]; arr[i]=arr[j]; arr[j]=tmp;
    }
}

int main(int argc, char* argv[]){
    // Seed matches group leader ID: 242UC242LB -> 2421324222
    srand((unsigned int)2421324222U);

    string csvFile = "datasets/dataset_1000.csv";
    if(argc>=2) csvFile=argv[1];

    // Load (not timed)
    vector<Record> data=loadCSV(csvFile);
    if(data.empty()){ cerr<<"No data loaded."<<endl; return 1; }
    int n=(int)data.size();
    cout<<"Dataset loaded: "<<n<<" elements"<<endl;

    // Build hash table (not timed)
    // Dynamic size: next prime >= n to keep average chainLen <= 1
    int dynamicTableSize = nextPrime(n);
    HashTable ht(dynamicTableSize);
    for(const Record& r:data) ht.insert(r);

    int maxChain=0, maxIdx=0;
    for(int i=0;i<dynamicTableSize;i++){
        if(ht.table[i].chainLen>maxChain){ maxChain=ht.table[i].chainLen; maxIdx=i; }
    }
    long long worstKey=0;
    { AVLNode* cur=ht.table[maxIdx].root; while(cur){ worstKey=cur->data.key; cur=cur->right; } }

    // Diagnostic: report how deep the worst real chain actually is
    int worstAVLHeight = ht.table[maxIdx].root ? ht.table[maxIdx].root->height : 0;
    cout << "[diagnostic] Table size = " << dynamicTableSize
         << ", longest bucket chain length = " << maxChain
         << ", AVL height = " << worstAVLHeight << endl;

    // ── Identify case keys & generate cache-representative search arrays ──
    
    // Best case keys: All keys in buckets of exactly length 1
    vector<long long> bestCasePool;
    for(const Record& r : data) {
        if(ht.table[ht.hf(r.key)].chainLen == 1) {
            bestCasePool.push_back(r.key);
        }
    }
    if(bestCasePool.empty()) {
        // Fallback
        bestCasePool.push_back(data[0].key);
    }
    vector<long long> bestCaseSearchKeys(n);
    for(int i=0; i<n; i++) {
        bestCaseSearchKeys[i] = bestCasePool[i % bestCasePool.size()];
    }
    fisherYatesShuffle(bestCaseSearchKeys);

    // Worst case keys: Deepest leaf node from each bucket that meets/exceeds a threshold depth.
    // Setting threshold = max(2, maxChain) ensures we pull leaf nodes from the deepest chains.
    int threshold = max(2, maxChain);
    vector<long long> worstCasePool;
    for (int i = 0; i < dynamicTableSize; i++) {
        if (ht.table[i].chainLen >= threshold && ht.table[i].root) {
            worstCasePool.push_back(findDeepestKey(ht.table[i].root));
        }
    }
    if(worstCasePool.empty()) {
        worstCasePool.push_back(worstKey);
    }
    vector<long long> worstCaseSearchKeys(n);
    for(int i=0; i<n; i++) {
        worstCaseSearchKeys[i] = worstCasePool[i % worstCasePool.size()];
    }
    fisherYatesShuffle(worstCaseSearchKeys);

    // Average case keys: Shuffle all existing keys
    vector<long long> searchKeys(n);
    for(int i=0;i<n;i++) searchKeys[i]=data[i].key;
    fisherYatesShuffle(searchKeys);

    volatile bool dummy = false;

    // ── BEST CASE: n searches on single-element bucket keys (shuffled to emulate cache misses) ──
    auto t0=high_resolution_clock::now();
    for(int i=0;i<n;i++) dummy = dummy ^ ht.search(bestCaseSearchKeys[i]);
    auto t1=high_resolution_clock::now();
    double bestTime=duration<double>(t1-t0).count();

    // ── AVERAGE CASE: n searches on shuffled existing keys ──
    auto t2=high_resolution_clock::now();
    for(int i=0;i<n;i++) dummy = dummy ^ ht.search(searchKeys[i]);
    auto t3=high_resolution_clock::now();
    double avgTime=duration<double>(t3-t2).count();

    // ── WORST CASE: n searches on deepest-chain keys (shuffled to emulate cache misses) ──
    auto t4=high_resolution_clock::now();
    for(int i=0;i<n;i++) dummy = dummy ^ ht.search(worstCaseSearchKeys[i]);
    auto t5=high_resolution_clock::now();
    double worstTime=duration<double>(t5-t4).count();

    // ── Comparison counts (avg per search) ──
    long long bestCompsSum = 0;
    for (int i = 0; i < n; i++) bestCompsSum += ht.searchComparisons(bestCaseSearchKeys[i]);
    double bestComps = (double)bestCompsSum / n;

    long long avgCompsSum = 0;
    for (int i = 0; i < n; i++) avgCompsSum += ht.searchComparisons(searchKeys[i]);
    double avgComps = (double)avgCompsSum / n;

    long long worstCompsSum = 0;
    for (int i = 0; i < n; i++) worstCompsSum += ht.searchComparisons(worstCaseSearchKeys[i]);
    double worstComps = (double)worstCompsSum / n;

    // ── Output ──────────────────────────────────────────
    string base=csvFile;
    size_t sl=base.find_last_of("/\\"); if(sl!=string::npos) base=base.substr(sl+1);
    size_t dt=base.find_last_of('.'); if(dt!=string::npos) base=base.substr(0,dt);
    
    MAKE_DIR("outputs");
    string outFile="outputs/hash_table_search_"+base+".txt";

    string bestLine  = "Best case time: "    + fmtTime(bestTime)  + " seconds";
    string avgLine   = "Average case time: " + fmtTime(avgTime)   + " seconds";
    string worstLine = "Worst case time: "   + fmtTime(worstTime) + " seconds";

    string bestCompLine  = "Best case comparisons: "    + fmtTime(bestComps)  + " (avg per search)";
    string avgCompLine   = "Average case comparisons: " + fmtTime(avgComps)   + " (avg per search)";
    string worstCompLine = "Worst case comparisons: "   + fmtTime(worstComps) + " (avg per search)";

    cout<<bestLine<<endl;
    cout<<avgLine<<endl;
    cout<<worstLine<<endl;
    cout<<bestCompLine<<endl;
    cout<<avgCompLine<<endl;
    cout<<worstCompLine<<endl;

    ofstream fout(outFile);
    fout<<bestLine<<"\n"<<avgLine<<"\n"<<worstLine<<"\n";
    fout<<bestCompLine<<"\n"<<avgCompLine<<"\n"<<worstCompLine<<"\n";
    fout.close();

    cout<<"\nOutput written to: "<<outFile<<endl;

    // Optional: run stress test to prove worst-case AVL depth scaling
    // Pass a second argument "stress" to enable, e.g.:
    //   ./hash_table_search datasets/dataset_1000.csv stress
    if (argc >= 3 && string(argv[2]) == "stress") {
        runStressTest(data);
    }

    return 0;
}