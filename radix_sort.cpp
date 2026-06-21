// *********************************************************
// Program: radix_sort.cpp
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
#include <direct.h>
using namespace std;

#define MAKE_DIR(d) _mkdir(d)

// Data record
struct Record {
    long long key;
    string    str;
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

// Counting sort on a single decimal digit position
// digitPos: 1 = units place, 10 = billions place
void countingSortByDigit(vector<Record>& arr, int digitPos)
{
    const int BASE = 10;
    long long divisor = 1;
    for (int i = 1; i < digitPos; ++i) divisor *= 10;

    vector<int> count(BASE, 0);

    for (const Record& r : arr)
        count[(r.key / divisor) % BASE]++;

    for (int i = 1; i < BASE; ++i)
        count[i] += count[i - 1];

    vector<Record> output(arr.size());
    for (int i = (int)arr.size() - 1; i >= 0; --i) {
        int digit = (int)((arr[i].key / divisor) % BASE);
        output[--count[digit]] = arr[i];
    }

    arr = output;
}

// Full radix sort (10 digit positions) 
// Sorts arr in ascending order by the integer key.
// Time complexity:  O(d * (n + k)) where d=10, k=10 -> O(n)
// Space complexity: O(n + k)
void radixSort(vector<Record>& arr)
{
    const int NUM_DIGITS = 10;
    for (int digitPos = 1; digitPos <= NUM_DIGITS; ++digitPos)
        countingSortByDigit(arr, digitPos);
}


int main()
{
    // Input: dataset filename 
    // Uncomment ONE line only for the desired input size.
    string datasetFile = "datasets/dataset_2000.csv";
    // string datasetFile = "datasets/dataset_10000.csv";
    // string datasetFile = "datasets/dataset_100000.csv";
    // string datasetFile = "datasets/dataset_500000.csv";
    // string datasetFile = "datasets/dataset_1000000.csv";
    // string datasetFile = "datasets/dataset_5000000.csv";
    // string datasetFile = "datasets/dataset_10000000.csv";
    // string datasetFile = "datasets/dataset_50000000.csv";
    // string datasetFile = "datasets/dataset_100000000.csv";
    // string datasetFile = "datasets/dataset_500000000.csv";
    // string datasetFile = "datasets/dataset_1000.csv";

    // Load data (I/O excluded from timing)
    vector<Record> arr;
    if (!loadDataset(datasetFile, arr)) {
        cerr << "Error: cannot open " << datasetFile << "\n";
        return 1;
    }
    long long n = (long long)arr.size();

    // Sort and measure
    auto t1 = chrono::high_resolution_clock::now();
    radixSort(arr);
    auto t2 = chrono::high_resolution_clock::now();

    double elapsed = chrono::duration<double>(t2 - t1).count();

    // Print running time to console
    cout << "Input file  : " << datasetFile << "\n";
    cout << "Input size  : " << n << "\n";
    cout << "Running time: " << elapsed << " seconds\n";

    // Build output filename
    // outputs/radix_sorted_dataset_<n>.csv
    MAKE_DIR("outputs");
    string outName = "outputs/radix_sorted_dataset_" + to_string(n) + ".csv";

    // Write sorted output (I/O excluded from timing) 
    ofstream fout(outName);
    if (!fout.is_open()) {
        cerr << "Error: cannot create " << outName << "\n";
        return 1;
    }
    for (const Record& r : arr)
        fout << r.key << '/' << r.str << '\n';

    // Append running time at end of output file
    fout << "\nInput file  : " << datasetFile << "\n";
    fout << "Input size  : " << n << "\n";
    fout << "Running time: " << elapsed << " seconds\n";
    fout.close();

    cout << "Sorted output written to " << outName << "\n";

    return 0;
}