// *********************************************************
// Program: heap_sort_step.cpp
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

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <direct.h>
using namespace std;

#define MAKE_DIR(d) _mkdir(d)

// Data record
struct Record {
    long long key;
    string str;
};

// Read CSV rows [startRow, endRow] (1-indexed)
bool loadRows(const string& filename,
              long long startRow,
              long long endRow,
              vector<Record>& out)
{
    ifstream fin(filename);
    if (!fin.is_open()) return false;

    string line;
    long long row = 0;
    while (getline(fin, line)) {
        ++row;
        if (row < startRow) continue;
        if (row > endRow) break;

        size_t comma = line.find(',');
        if (comma == string::npos) continue;

        Record r;
        r.key = atoll(line.substr(0, comma).c_str());
        r.str = line.substr(comma + 1);

        if (!r.str.empty() && r.str.back() == '\r')
            r.str.pop_back();

        out.push_back(r);
    }
    return true;
}

// Write one snapshot line to the output file
void writeSnapshot(ofstream& fout,
                   const vector<Record>& arr,
                   const string& label)
{
    fout << '[';
    for (size_t i = 0; i < arr.size(); ++i) {
        if (i) fout << ", ";
        fout << arr[i].key << '/' << arr[i].str;
    }
    fout << "] " << label << '\n';
}

// Max heapify
void maxHeapify(vector<Record>& arr, int heapSize, int i)
{
    while (true) {
        int largest = i;
        int left = 2 * i + 1;
        int right = 2 * i + 2;

        if (left < heapSize && arr[left].key > arr[largest].key)
            largest = left;

        if (right < heapSize && arr[right].key > arr[largest].key)
            largest = right;

        if (largest == i)
            break;

        swap(arr[i], arr[largest]);
        i = largest;
    }
}

// Build maxheap
void buildMaxHeap(vector<Record>& arr)
{
    int n = (int)arr.size();
    for (int i = n / 2 - 1; i >= 0; --i)
        maxHeapify(arr, n, i);
}

// Heap sort with step output
// The first output line is the initial maxheap.
// Then each line shows the array after the maximum element is placed at index i.
void heapSortStep(vector<Record>& arr, ofstream& fout)
{
    int n = (int)arr.size();

    buildMaxHeap(arr);
    writeSnapshot(fout, arr, "initial");

    for (int i = n - 1; i > 0; --i) {
        swap(arr[0], arr[i]);
        maxHeapify(arr, i, 0);

        string label = "i = " + to_string(i);
        writeSnapshot(fout, arr, label);
    }
}

// Extract n from dataset filename
long long extractInputSize(const string& datasetFile, long long fallback)
{
    string base = datasetFile;
    size_t slash = base.find_last_of("/\\");
    if (slash != string::npos) base = base.substr(slash + 1);

    size_t u = base.rfind('_');
    size_t dot = base.rfind('.');
    if (u != string::npos && dot != string::npos && dot > u)
        return atoll(base.substr(u + 1, dot - u - 1).c_str());

    return fallback;
}

int main(int argc, char* argv[])
{
    // Input: dataset file
    // Uncomment ONE line only. Tutor may specify a different file during demo.
    string datasetFile = "datasets/dataset_2000.csv";
    // string datasetFile = "datasets/dataset_5000.csv";
    // string datasetFile = "datasets/dataset_10000.csv";
    // string datasetFile = "datasets/dataset_50000.csv";
    // string datasetFile = "datasets/dataset_100000.csv";
    // string datasetFile = "datasets/dataset_500000.csv";
    // string datasetFile = "datasets/dataset_1000000.csv";
    // string datasetFile = "datasets/dataset_5000000.csv";
    // string datasetFile = "datasets/dataset_10000000.csv";
    // string datasetFile = "datasets/dataset_50000000.csv";

    // Input: start row
    long long startRow = 1;
    // long long startRow = 3;
    // long long startRow = 5;

    // Input: end row
    long long endRow = 7;
    // long long endRow = 10;
    // long long endRow = 20;

    // Optional command prompt input:
    // heap_sort_step.exe datasets/dataset_1000.csv 1 7
    if (argc >= 2) datasetFile = argv[1];
    if (argc >= 3) startRow = atoll(argv[2]);
    if (argc >= 4) endRow = atoll(argv[3]);

    vector<Record> arr;
    if (!loadRows(datasetFile, startRow, endRow, arr)) {
        cerr << "Error: cannot open " << datasetFile << "\n";
        return 1;
    }

    if (arr.empty()) {
        cerr << "Error: no records loaded. Check start row and end row.\n";
        return 1;
    }

    long long n = extractInputSize(datasetFile, (long long)arr.size());

    // outputs/dataset_<n>_heap_sorted_step_<startRow>_<endRow>.txt
    MAKE_DIR("outputs");
    string outName = "outputs/dataset_" + to_string(n)
                   + "_heap_sorted_step_"
                   + to_string(startRow) + "_"
                   + to_string(endRow) + ".txt";

    ofstream fout(outName);
    if (!fout.is_open()) {
        cerr << "Error: cannot create " << outName << "\n";
        return 1;
    }

    heapSortStep(arr, fout);
    fout.close();

    cout << "Heap sort steps written to " << outName << "\n";

    return 0;
}
