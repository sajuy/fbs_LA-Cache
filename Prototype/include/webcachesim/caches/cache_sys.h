//
//  Cache_Sys.h
//  ATS- One Model Method
//
//  Created by Gang Yan on 9/26/20.
//  Copyright © 2020 Gang Yan. All rights reserved.
//

#ifndef Cache_Sys_h
#define Cache_Sys_h

#include <cmath>
#include <vector>
#include <map>

using namespace std;

//缓存系统的模拟器

class Cache_Sys
{
public:
    map<uint64_t,uint64_t> cache;//存储id:size
    uint64_t all_capacity = 0; //cache的大小
    uint64_t used_space = 0; //已经使用的cache space
    uint64_t hits = 0;
    uint64_t misses = 0;
    map<uint64_t, float> evict_set; //eviction的备选集合
    map<uint64_t, uint64_t> cache_hits;//记录每个cached item被request的次数
    float currentL = 0.0;
    map<uint64_t, float> All_Hs;
    float threshold = 0.0;
    
    void init(uint64_t capacity);//初始化
    void reset();//相关参数重置
    void update();//维护内存空间
    void decide(vector<uint64_t> Req, float prob, float threshold);
    
};


void Cache_Sys::init(uint64_t capacity)
{
    all_capacity = capacity;
}

void Cache_Sys::reset()
{
    cout<<"Reset..."<<endl;
    hits = 0;
    misses = 0;
}

void Cache_Sys::update()
{
    //cout<<"Update Caches..."<<endl;
    //更新recency
    //进行eviction
    uint64_t min_key = 0;
    float min_val = 1000000.0;
    while(used_space>all_capacity)
    {
        uint64_t delete_id = -1;
        if(evict_set.size()>0)
        {
            map<uint64_t, float>::iterator It;
            It = evict_set.begin();
            while(It != evict_set.end())
            {
                uint64_t key_now = It->first;
                uint64_t val_now = It->second;
                if(min_val>val_now)
                {
                    min_val = val_now;
                    min_key = key_now;
                }
                It++;
            }
            delete_id = min_key;
            evict_set.erase(delete_id);
        }
        else
        {
            map<uint64_t, float>::iterator It;
            It = All_Hs.begin();
            while(It != All_Hs.end())
            {
                uint64_t key_now = It->first;
                uint64_t val_now = It->second;
                if(min_val>val_now)
                {
                    min_val = val_now;
                    min_key = key_now;
                }
                It++;
            }
            delete_id = min_key;
        }
        currentL = min_val;

        if(delete_id != -1)
        {
            used_space = used_space - cache[delete_id];
            All_Hs.erase(delete_id);
            cache.erase(delete_id);
        }
        
        
        map<uint64_t, float>(All_Hs).swap(All_Hs);
        map<uint64_t, float>(evict_set).swap(evict_set);
        map<uint64_t,uint64_t>(cache).swap(cache);
    }
    
}

void Cache_Sys::decide(vector<uint64_t> Req, float prob, float threshold)
{
    uint64_t Id = Req[1];
    uint64_t Time = Req[0];
    uint64_t Size = Req[2];
    float Prob = prob;
    float T = threshold;
    //找到当前cache里面是否存在该item
    bool In = 0;
    map<uint64_t, uint64_t>::iterator It_In;
    It_In = cache.begin();
    while(It_In != cache.end())
    {
        uint64_t key_now = It_In->first;
        if(key_now==Id)
        {
            In = 1;
            break;
        }
        It_In++;
    }
    if(In == 1){
        cache_hits[Id] += 1.0;
    }
    else{
        cache_hits[Id] = 1.0;
    }

    float Score = currentL + cache_hits[Id] / Size + pow(Prob, 2.0) / Size;
    
    if(Prob < T)
    {
        if(In == 1)
        {
            hits++;
            evict_set[Id] = Score;//添加进evict候选集合
            All_Hs[Id] = Score;
        }
        else
        {
            misses++;
        }
    }
    
    if(Prob >= T)
    {
        //更新或者添加记录
        cache[Id] = Size;
        All_Hs[Id] = Score;
        
        if(In == 1)
        {
            hits++;
            //查找其是否存在于evict set中
            int In_Evict = 0;
            map<uint64_t, float>::iterator It;
            It = evict_set.begin();
            while(It != evict_set.end())
            {
                uint64_t key_now = It->first;
                if(key_now==Id)
                {
                    In_Evict = 1;
                    break;
                }
                It++;
            }
            if(In_Evict==1)
            {
                evict_set.erase(Id);
                map<uint64_t, float>(evict_set).swap(evict_set);
            }
        }
        else
        {
            misses++;
            used_space += Size;
        }
    }
    
    //更新数据
    update();
    
}

#endif /* Cache_Sys_h */
