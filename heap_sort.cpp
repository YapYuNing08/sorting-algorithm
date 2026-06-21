// *********************************************************
// Program: heap_sort.cpp
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
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <direct.h>
using namespace std;
using namespace chrono;

#define MAKE_DIR(d) _mkdir(d)

// Data record
struct Record {
    long long key;
    string str;
};

// Read entire CSV file
bool loadDataset(const string& filename, vector<Record>& out)
{
    ifstream fin(filename);
    if (!fin.is_open()) return false;

    string line;
    while (getline(fin, line)) {
        if (line.empty()) continue;

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

// Max heapify
// Maintains the maxheap property for subtree rooted at index i.
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

// Heap sort using maxheap
// Sorts records in ascending order by integer key.
// Time complexity:  O(n log n)
// Space complexity: O(1) extra space
void heapSort(vector<Record>& arr)
{
    int n = (int)arr.size();

    buildMaxHeap(arr);

    for (int i = n - 1; i > 0; --i) {
        swap(arr[0], arr[i]);      // Move current maximum to the end
        maxHeapify(arr, i, 0);     // Restore maxheap for remaining heap
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
    // Input: dataset filename
    // Uncomment ONE line only for the desired input size.
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

    // Optional: allow command prompt input also, for easier experiment runs.
    // Example: heap_sort.exe datasets/dataset_10000.csv
    if (argc >= 2)
        datasetFile = argv[1];

    // Load data (I/O excluded from timing)
    vector<Record> arr;
    if (!loadDataset(datasetFile, arr)) {
        cerr << "Error: cannot open " << datasetFile << "\n";
        return 1;
    }

    long long n = extractInputSize(datasetFile, (long long)arr.size());

    // Sort and measure only heap sort
    auto t1 = high_resolution_clock::now();
    heapSort(arr);
    auto t2 = high_resolution_clock::now();

    double elapsed = duration<double>(t2 - t1).count();

    // Print running time to console
    cout << "Input file  : " << datasetFile << "\n";
    cout << "Input size  : " << arr.size() << "\n";
    cout << fixed << setprecision(9);
    cout << "Running time: " << elapsed << " seconds\n";

    //  Build output filename
    // outputs/heap_sorted_dataset_<n>.csv
    MAKE_DIR("outputs");
    string outName = "outputs/heap_sorted_dataset_" + to_string(n) + ".csv";

    // Write sorted output (I/O excluded from timing)
    ofstream fout(outName);
    if (!fout.is_open()) {
        cerr << "Error: cannot create " << outName << "\n";
        return 1;
    }

    for (const Record& r : arr)
        fout << r.key << '/' << r.str << '\n';

    fout << "\nInput file  : " << datasetFile << "\n";
    fout << "Input size  : " << arr.size() << "\n";
    fout << fixed << setprecision(9);
    fout << "Running time: " << elapsed << " seconds\n";
    fout.close();

    cout << "Sorted output written to " << outName << "\n";

    return 0;
}
