// *********************************************************
// Program: hash_table_search_step.cpp
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
#include <string>
#include <vector>
#include <cstdint>

using namespace std;

struct Record { long long key; string str; };

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
AVLNode* avlIns(AVLNode* nd, Record r) {
    if(!nd) return new AVLNode(r);
    if(r.key<nd->data.key) nd->left=avlIns(nd->left,r);
    else if(r.key>nd->data.key) nd->right=avlIns(nd->right,r);
    return avlBal(nd);
}

// Search with step logging. Returns true if found.
bool avlStep(AVLNode* nd, long long tgt, vector<string>& log) {
    if(!nd) return false;
    if(tgt==nd->data.key){
        log.push_back(to_string(tgt)+" = "+to_string(nd->data.key)+"/"+nd->data.str);
        return true;
    }
    log.push_back(to_string(nd->data.key)+"/"+nd->data.str+" != "+to_string(tgt));
    if(tgt<nd->data.key) return avlStep(nd->left, tgt, log);
    else                 return avlStep(nd->right,tgt, log);
}

const int TABLE_SIZE = 1000003; // prime
struct HashSlot { AVLNode* root; HashSlot():root(nullptr){} };

class HashTable {
public:
    HashSlot* table;
    HashTable(){ table=new HashSlot[TABLE_SIZE]; }
    ~HashTable(){ delete[] table; }
    int hf(long long k) const { return (int)(k % TABLE_SIZE); }
    void insert(Record r){ int i=hf(r.key); table[i].root=avlIns(table[i].root,r); }

    bool search(long long tgt, vector<string>& log, bool& notFoundLogged) const {
        int idx=hf(tgt);
        log.push_back("hash("+to_string(tgt)+") = "+to_string(idx));
        if(!table[idx].root){
            log.push_back("-1 != "+to_string(tgt));
            notFoundLogged=true;
            return false;
        }
        notFoundLogged=false;
        return avlStep(table[idx].root, tgt, log);
    }
};

vector<Record> loadCSV(const string& fn){
    vector<Record> d; ifstream fin(fn);
    if(!fin.is_open()){ cerr<<"Error: cannot open "<<fn<<endl; return d; }
    string line;
    while(getline(fin,line)){
        if(line.empty()) continue;
        size_t c=line.find(','); if(c==string::npos) continue;
        Record r; r.key=stoll(line.substr(0,c)); r.str=line.substr(c+1);
        if(!r.str.empty()&&r.str.back()=='\r') r.str.pop_back();
        d.push_back(r);
    }
    return d;
}

int main(int argc, char* argv[]){
    //Inputs
    string  csvFile = "datasets/dataset_1000.csv";
    long long target = 1000000038LL; // change to found / not-found key

    if(argc>=2) csvFile=argv[1];
    if(argc>=3) target=stoll(argv[2]);

    vector<Record> data=loadCSV(csvFile);
    if(data.empty()){ cerr<<"No data loaded."<<endl; return 1; }

    HashTable ht;
    for(const Record& r:data) ht.insert(r);

    vector<string> steps;
    bool nfl=false;
    bool found=ht.search(target,steps,nfl);

    // Build output filename from csv base name
    string base=csvFile;
    size_t sl=base.find_last_of("/\\"); if(sl!=string::npos) base=base.substr(sl+1);
    size_t dt=base.find_last_of('.'); if(dt!=string::npos) base=base.substr(0,dt);
    string outFile=base+"_hash_table_search_step_"+to_string(target)+".txt";

    ofstream fout(outFile);
    for(const string& s:steps){ cout<<s<<"\n"; fout<<s<<"\n"; }
    if(!found && !nfl){
        string nf="-1 != "+to_string(target);
        cout<<nf<<"\n"; fout<<nf<<"\n";
    }
    fout.close();
    cout<<"\nOutput written to: "<<outFile<<endl;
    return 0;
}
