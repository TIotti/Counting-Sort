#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <chrono>
#include "Insert_sort.h"

typedef std::chrono::high_resolution_clock clock_;
typedef std::chrono::duration<double, std::ratio<1> > second_;



std::chrono::time_point<clock_> m_beg;

using namespace std;

vector<InsertSort::InsertionElement> ListA;
vector<InsertSort::InsertionElement> ListB;
vector<unsigned int>storage;

InsertSort sorter;

const char* getfield(char* line, int num)
{
    const char* tok;
    for (tok = strtok(line, ",");
        tok && *tok;
        tok = strtok(NULL, ",\n"))
    {
        if (!--num)
            return tok;
    }
    return NULL;
}


void LoadFromFile(const char* path, vector<InsertSort::InsertionElement> &list)
{
    FILE* stream = fopen(path, "r");
    char line[1024];
    while (fgets(line, 1024, stream))
    {
        char* tmp = strdup(line);
        char *id = strtok(tmp, ";");
        char *email = strtok(NULL, "\n");
        storage.push_back(atoi(id));
        InsertSort::InsertionElement item(email, &storage[storage.size()-1]);
        list.push_back(item);
        free(tmp);
    }
}


void LoadFiles()
{
    std::chrono::steady_clock::time_point beg, fin;
    beg = std::chrono::steady_clock::now();
    LoadFromFile("list_a.csv", ListA);
    LoadFromFile("list_b.csv", ListB);
    fin = std::chrono::steady_clock::now();
    cout << "List A:" << ListA.size()
    << " List B:" << ListB.size()
    << " in " << std::chrono::duration_cast<std::chrono::milliseconds> (fin - beg).count() << "ms" << endl;
}

void LoadListOnMemory()
{
    m_beg = clock_::now();
    #pragma omp parallel for
    std::for_each(ListA.begin(), ListA.end(), [&sorter](auto &it) {
        InsertSort::InsertionElement *n = &it;
        sorter.InsertFromA(n->key, n);
    });
    #pragma omp parallel for
    std::for_each(ListB.begin(), ListB.end(), [&sorter](auto &it) {
        InsertSort::InsertionElement *n = &it;
        sorter.InsertFromB(n->key, n);
    });

    double time_spent = std::chrono::duration_cast<std::chrono::milliseconds>(
                clock_::now() - m_beg).count();
    size_t total = sorter.GetElements().size();

    cout << "Total:" << total << " in " << time_spent << "ms" << endl;
}

void CountElements()
{
    unsigned long long countA = 0, countB = 0, countC = 0;
    std::chrono::steady_clock::time_point beg, fin;
    beg = std::chrono::steady_clock::now();
    sorter.CountElements(countA, countB, countC);
    fin = std::chrono::steady_clock::now();
    unsigned long long elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(fin - beg).count();
    cout
    << " Group A:" << countA
    << ", Group B:" << countB
    << ", Both:" << countC
    << " in " << elapsed << "ns" << endl;
}


int main()
{
    cout << "Reading CSV files" << endl;
    LoadFiles();
    cout << "Feeding lists to the sorter" << endl;
    LoadListOnMemory();
    cout << "Collecting results" <<endl;
    CountElements();
    return 0;
}
