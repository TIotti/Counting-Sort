#include "Insert_sort.h"
#include <iostream>
#include <parallel/algorithm>
#include <thread>


InsertSort::InsertSort()
#if USE_CACHED_ELEMENTS
: countA(0), countB(0), countBoth(0)
#endif
{
    //ctor
}

InsertSort::~InsertSort()
{
    Clear();
}


void InsertSort::Clear()
{
    m_Elements.clear();
    #if USE_CACHED_ELEMENTS
    m_Groups.clear();
    m_Groups[1] = std::map<InsertKey,void*>();
    m_Groups[2] = std::map<InsertKey,void*>();
    m_Groups[3] = std::map<InsertKey,void*>();
    #endif // USE_CACHED_ELEMENTS
}
void InsertSort::CountElements(unsigned long long &groupA, unsigned long long &groupB, unsigned long long &bothGroups)
{
    #if USE_CACHED_ELEMENTS
        groupA = countA;
        groupB = countB;
        bothGroups = countBoth;
        /*groupA = m_Groups[1].size();
        groupB = m_Groups[2].size();
        bothGroups = m_Groups[3].size();*/
    #else
        unsigned long long * elementCount[] = {NULL, &groupA, &groupB, &bothGroups};
        for (auto it = m_Elements.begin(); it != m_Elements.end(); ++it) {
    #if PRINT_DEBUG
            std::cout << it->first << " belongs to group " << (int)it->second.group << std::endl;
    #endif
            elementCount[it->second.group][0]++;
        }
    #endif // USE_CACHED_COUNTER

}
void InsertSort::GetGroups(std::map<InsertKey, void*> *groupA, std::map<InsertKey, void*> *groupB, std::map<InsertKey, void*> *bothGroups)
{

    #if USE_CACHED_ELEMENTS
        groupA = &m_Groups[1];
        groupB = &m_Groups[2];
        bothGroups = &m_Groups[3];
    #else
    std::map<InsertKey, void*> * elementCount[] = {NULL, groupA, groupB, bothGroups};
    std::for_each(m_Elements.begin(), m_Elements.end(), [elementCount](auto& it){
            std::map<InsertKey, void*> *group = elementCount[it.second.group];
            (*group)[it.first] = it.second.data;
         });

    #endif
}

void InsertSort::GetGroups(std::vector<void*> *groupA, std::vector<void*> *groupB, std::vector<void*> *bothGroups, bool preAllocate)
{
    #if USE_CACHED_ELEMENTS

    #else
    if(preAllocate)
    {
        size_t sizeA, sizeB, sizeC;
        CountElements(sizeA, sizeB, sizeC);
        groupA->reserve(sizeA);
        groupB->reserve(sizeB);
        bothGroups->reserve(sizeC);
    }
    std::vector<void*> * elementCount[] = {NULL, groupA, groupB, bothGroups};

    std::for_each(m_Elements.begin(), m_Elements.end(), [elementCount](auto& it){
            std::vector<void*> *group = elementCount[it.second.group];
            group->push_back(it.second.data);
         });

    #endif // USE_CACHED_ELEMENTS
}

void InsertSort::RestoreListA()
{
    std::vector<InsertKey> removeKeys;
    std::for_each(m_Elements.begin(), m_Elements.end(), [&removeKeys, this](auto& it){
            if(it.second.group == 1) {
            }
            else if(it.second.group == 3) {
                it.second.group = 1;
                #if USE_CACHED_ELEMENTS
                m_Groups[1][it.first] = it.second.data;
                m_Groups[3].erase(it.first);
                #endif
                // USE_CACHED_ELEMENTS
            }
            else {
               removeKeys.push_back(it.first);
            }

         });
         #if USE_CACHED_ELEMENTS
         m_Groups[2].clear();
         #endif
    std::for_each(removeKeys.begin(), removeKeys.end(), [this](auto& it)
         {
            m_Elements.erase(it);
         });
}

void InsertSort::ParallelLoadLists(std::vector<InsertionElement> *listA, std::vector<InsertionElement> *listB)
{
    std::mutex lockA, lockB;

    std::thread([this, listA, &lockA](){
                lockA.lock();
                const int bufferSize = 50000;
                std::vector<InsertionElement*>buffer;
                std::for_each(listA->begin(), listA->end(), [this, &buffer, &bufferSize](auto &la){
                      buffer.push_back(&la);
                      if(buffer.size()>=bufferSize)
                      {
                          group_mutex.lock();
                          std::for_each(buffer.begin(), buffer.end(),[this](auto &lx){
                                        this->InsertFromA(lx->key, lx->data);
                                        });
                          group_mutex.unlock();
                          buffer.clear();
                      }

                  });
                 if(buffer.size() > 0)
                 {
                     group_mutex.lock();
                     std::for_each(buffer.begin(), buffer.end(),[this](auto &lx){
                                                this->InsertFromA(lx->key, lx->data);
                                                });
                     group_mutex.unlock();
                 }
                 buffer.clear();
                 lockA.unlock();
                 }).detach();
    std::thread([this, listB, &lockB](){
                lockB.lock();
                const int bufferSize = 50000;
                std::vector<InsertionElement*>buffer;
                std::for_each(listB->begin(), listB->end(), [this, &buffer, &bufferSize](auto &lb){
                     buffer.push_back(&lb);
                     if(buffer.size()>=bufferSize)
                     {
                         group_mutex.lock();
                         std::for_each(buffer.begin(), buffer.end(),[this](auto &lx){
                                       this->InsertFromB(lx->key, lx->data);
                                       });
                         group_mutex.unlock();
                         buffer.clear();
                     }
                });
                if(buffer.size() > 0)
                {
                    group_mutex.lock();
                    std::for_each(buffer.begin(), buffer.end(),[this](auto &lx){
                                               this->InsertFromB(lx->key, lx->data);
                                               });
                    group_mutex.unlock();
                }
                buffer.clear();
                lockB.unlock();
                }).detach();
    lockA.lock();
    lockB.lock();
    lockA.unlock();
    lockB.unlock();
}

bool InsertSort::InsertFromA(const InsertKey &key, void *data)
{
    return AddElement(key, data, true);
}
bool InsertSort::InsertFromB(const InsertKey &key, void *data)
{
    return AddElement(key, data, false);
}
bool InsertSort::AddElement(const InsertKey &key, void *data, bool isListA)
{
    ListElement &element = m_Elements[key];
    element.data = data;
    #if USE_CACHED_ELEMENTS
    if(element.group==1)
    {
        if(!isListA)
        {
            countA--;
            countBoth++;
            m_Groups[1].erase(key);
            m_Groups[3][key] = data;
        }
    }
    else if(element.group==2)
    {
        if(isListA)
        {
            countB--;
            countBoth++;
            m_Groups[2].erase(key);
            m_Groups[3][key] = data;
        }
    }
    else if(element.group!=3)
    {
        if(isListA){
            countA++;
        }
        else{
            countB++;
        };
        m_Groups[(isListA ? 1 : 2)][key] = data;
    }
    #endif // USE_CACHED_COUNTER
    element.group |= (isListA ? 1 : 2);
    return true;
}
