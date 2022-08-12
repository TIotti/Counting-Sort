#ifndef INSERTSORT_H
#define INSERTSORT_H
#include<stdlib.h>
#include<string>
#include<map>
#include<vector>
#include <algorithm>
#include <mutex>

#ifndef PRINT_DEBUG
#define PRINT_DEBUG false
#endif
#ifndef USE_CACHED_ELEMENTS
#define USE_CACHED_ELEMENTS 1
#endif

typedef std::string InsertKey;


class InsertSort
{

    public:
        struct ListElement
        {
            char group;
            void *data;

            ListElement()
            {
                group = 0;
                data = NULL;
            }
        };

        struct InsertionElement
        {
            InsertKey key;
            void *data;

            InsertionElement() : data(NULL)
            {
            }
            InsertionElement(const char* _key, void* _data) : data(_data), key(_key) {}
        };

    public:
        InsertSort();
        virtual ~InsertSort();

        std::map<InsertKey, ListElement> GetElements() { return m_Elements; }

        #if USE_CACHED_ELEMENTS
        std::map<char, std::map<InsertKey, void*>> *GetGroups() { return &m_Groups; }
        #endif

        void Clear();
        void CountElements(unsigned long long &groupA, unsigned long long &groupB, unsigned long long &bothGroups);
        void GetGroups(std::map<InsertKey, void*> *groupA, std::map<InsertKey, void*> *groupB, std::map<InsertKey, void*> *bothGroups);
        void GetGroups(std::vector<void*> *groupA, std::vector<void*> *groupB, std::vector<void*> *bothGroups, bool preAllocate = true);

        bool InsertFromA(const InsertKey &key, void *data);
        bool InsertFromB(const InsertKey &key, void *data);
        void ParallelLoadLists(std::vector<InsertionElement> *listA, std::vector<InsertionElement> *listB);

        void RestoreListA();


    protected:

    private:
        bool AddElement(const InsertKey &key, void *data, bool isListA);
        std::map<InsertKey, ListElement> m_Elements;
        #if USE_CACHED_ELEMENTS
        unsigned long long countA, countB, countBoth;
        std::map<char, std::map<InsertKey, void*>> m_Groups;
        #endif
        std::mutex group_mutex;

};

#endif // INSERTSORT_H
