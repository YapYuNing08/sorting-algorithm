// *********************************************************
// Program: radix_sort_step.cpp
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
#include <sstream>
#include <string>
#include <vector>
#include <cstdlib>
#include <direct.h>
using namespace std;

#define MAKE_DIR(d) _mkdir(d)

// Data record
struct Record {
    long long key;
    string    str;
};

// Read CSV rows [startRow, endRow] (1-indexed) 
// Returns false if file cannot be opened.
bool loadRows(const string& filename,
              long long startRow, long long endRow,
              vector<Record>& out)
{
    ifstream fin(filename);
    if (!fin.is_open()) return false;

    string line;
    long long row = 0;
    while (getline(fin, line)) {
        ++row;
        if (row < startRow) continue;
        if (row > endRow)   break;

        size_t comma = line.find(',');
        if (comma == string::npos) continue;

        Record r;
        r.key = atoll(line.substr(0, comma).c_str());
        r.str = line.substr(comma + 1);
        // strip trailing \r if present
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

// Counting sort on a single decimal digit position 
// digitPos: 1 = units, 2 = tens, ..., 10 = billions place
void countingSortByDigit(vector<Record>& arr, int digitPos)
{
    const int BASE = 10;
    // Compute divisor: 10^(digitPos-1)
    long long divisor = 1;
    for (int i = 1; i < digitPos; ++i) divisor *= 10;

    vector<int> count(BASE, 0);

    // Count occurrences of each digit
    for (const Record& r : arr)
        count[(r.key / divisor) % BASE]++;

    // Prefix sums (cumulative counts)
    for (int i = 1; i < BASE; ++i)
        count[i] += count[i - 1];

    // Build output in stable order (traverse from right for stability)
    vector<Record> output(arr.size());
    for (int i = (int)arr.size() - 1; i >= 0; --i) {
        int digit = (int)((arr[i].key / divisor) % BASE);
        output[--count[digit]] = arr[i];
    }

    arr = output;
}

// Radix sort with step output
// All integers are 10-digit (1,000,000,000 to 9,999,999,999)
// so we always process exactly 10 digit positions.
void radixSortStep(vector<Record>& arr, ofstream& fout)
{
    const int NUM_DIGITS = 10; // positions d=10 (units) down to d=1 (billions)

    // Write original state
    writeSnapshot(fout, arr, "original");

    // Process from rightmost digit (position 1 = units digit)
    // d=10 means the 10th digit from the left = units place
    for (int d = NUM_DIGITS; d >= 1; --d) {
        int digitPos = NUM_DIGITS - d + 1; // 1 when d=10, 10 when d=1
        countingSortByDigit(arr, digitPos);

        // Label matches sample output: d=10, d=9, ..., d=1
        string label = "d=" + to_string(d);
        writeSnapshot(fout, arr, label);
    }
}

int main()
{
    // Input: dataset file
    // Uncomment ONE line only. Tutor may specify a different file during demo.
    // string datasetFile = "datasets/dataset_1000.csv";
    // string datasetFile = "datasets/dataset_10000.csv";
    // string datasetFile = "datasets/dataset_100000.csv";
    // string datasetFile = "datasets/dataset_500000.csv";
    // string datasetFile = "datasets/dataset_1000000.csv";
    string datasetFile = "datasets/dataset_2000.csv";

    // Input: start row (row number in CSV file, 1-indexed)
    // Uncomment ONE line only. Tutor specifies in the code file during demo.
    // long long startRow = 1;
    // long long startRow = 3;
    // long long startRow = 5;
    long long startRow = 1;

    // Input: end row
    // Uncomment ONE line only. Tutor specifies in the code file during demo.
    // long long endRow = 7;
    // long long endRow = 10;
    // long long endRow = 20;
    long long endRow = 7;

    // Load rows
    vector<Record> arr;
    if (!loadRows(datasetFile, startRow, endRow, arr)) {
        cerr << "Error: cannot open " << datasetFile << "\n";
        return 1;
    }
    if (arr.empty()) {
        cerr << "Error: no records loaded (check start/end row)\n";
        return 1;
    }

    // Derive n from the dataset filename (look for _NNN. pattern)
    // e.g. dataset_1000.csv -> 1000
    long long n = (long long)arr.size(); // fallback
    {
        string base = datasetFile;
        // strip directory
        size_t slash = base.find_last_of("/\\");
        if (slash != string::npos) base = base.substr(slash + 1);
        // find underscore before number
        size_t u = base.rfind('_');
        size_t dot = base.rfind('.');
        if (u != string::npos && dot != string::npos && dot > u)
            n = atoll(base.substr(u + 1, dot - u - 1).c_str());
    }

    // Build output filename:
    // outputs/dataset_<n>_radix_sorted_step_<startRow>_<endRow>.txt
    MAKE_DIR("outputs");
    string outName = "outputs/dataset_" + to_string(n)
                   + "_radix_sorted_step_"
                   + to_string(startRow) + "_"
                   + to_string(endRow) + ".txt";

    ofstream fout(outName);
    if (!fout.is_open()) {
        cerr << "Error: cannot create " << outName << "\n";
        return 1;
    }

    radixSortStep(arr, fout);

    cout << "Radix sort steps written to " << outName << "\n";
    return 0;
}