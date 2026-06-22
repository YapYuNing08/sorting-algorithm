// *********************************************************
// Program: dataset_generator.cpp.cpp
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
#include <cstdlib>
#include <string>
#include <direct.h>
using namespace std;

#define MAKE_DIR(d) _mkdir(d)

// Hand-implemented hash set: open addressing with linear probing.
// Sentinel 0 is safe, all valid integers are >= 1,000,000,000.
struct HashSet {
    long long* table;
    size_t capacity;

    HashSet(size_t cap) : capacity(cap), table(new long long[cap]()) {}
    ~HashSet() { delete[] table; }

    bool insert(long long val) {
        size_t idx = (size_t)(val % (long long)capacity);
        while (table[idx] != 0) {
            if (table[idx] == val) return false;
            if (++idx == capacity) idx = 0;
        }
        table[idx] = val;
        return true;
    }
};

int main(int argc, char* argv[]) {
    // Leader ID: 242UC244KD -> 2421324414
    srand((unsigned int)2421324414U);

    if (argc < 2) {
        cerr << "Usage: " << argv[0] << " <n>\n";
        return 1;
    }

    long long n = atoll(argv[1]);
    if (n <= 0) {
        cerr << "n must be a positive integer\n";
        return 1;
    }

    MAKE_DIR("datasets");

    string filename = "datasets/dataset_" + to_string(n) + ".csv";
    ofstream out(filename);
    if (!out.is_open()) {
        cerr << "Failed to open " << filename << "\n";
        return 1;
    }

    const long long MIN_VAL = 1000000000LL;
    const long long MAX_VAL = 9999999999LL;
    const long long RANGE   = MAX_VAL - MIN_VAL + 1; // 9,000,000,000

    // Next power of 2 >= 2*n keeps load factor <= 0.5
    size_t cap = 1;
    while (cap < (size_t)n * 2) cap <<= 1;
    HashSet used(cap);

    for (long long count = 0; count < n; ) {
        // Combine three 15-bit chunks to produce a 45-bit random value
        long long r = ((long long)(rand() & 0x7FFF) << 30) |
                      ((long long)(rand() & 0x7FFF) << 15) |
                      ((long long)(rand() & 0x7FFF));
        long long val = MIN_VAL + (r % RANGE);

        if (used.insert(val)) {
            char str[6];
            for (int i = 0; i < 5; i++)
                str[i] = 'a' + rand() % 26;
            str[5] = '\0';
            out << val << ',' << str << '\n';
            ++count;
        }
    }

    cout << "Generated " << n << " records -> " << filename << "\n";
    return 0;
}
