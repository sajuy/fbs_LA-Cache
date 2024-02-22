//
//  cache_sys_simulator.h
//  RLB-Modules
//
//  Created by Gang Yan on 11/15/20.
//

#ifndef cache_sys_rlb_h
#define cache_sys_rlb_h
//模拟B-LRU的eviction过程

#include <cmath>
#include <vector>
#include <map>

using namespace std;

//缓存系统的模拟器

class Cache_Sys
{
public:
    map<uint64_t,uint64_t> cache;//存储id:size
    map<uint64_t,uint64_t> last_req_time;//存储id:last request time
    uint64_t all_capacity = 0; //cache的大小
    uint64_t used_space = 0; //已经使用的cache space
    uint64_t hits = 0;
    uint64_t misses = 0;
    uint64_t timer = 0;//全局使用的计时器

    map<uint64_t,float> b_recency;//eviction的依据
    
    void init(uint64_t capacity,map<uint64_t,uint64_t> already_cache);//初始化
    void reset();//相关参数重置
    void update();//维护内存空间
    void decide(vector<uint64_t> Req, float prob, float nrt);
};


void Cache_Sys::init(uint64_t capacity,map<uint64_t,uint64_t> already_cache)
{
    all_capacity = capacity;
    cache = already_cache;
    map<uint64_t,uint64_t>::iterator It;
    It = already_cache.begin();
    while(It != already_cache.end()){
        uint64_t size = It->second;
        used_space +=size;
        It++;
    }
}

void Cache_Sys::reset()
{
    cout<<"Reset..."<<endl;
    hits = 0;
    misses = 0;
}

void Cache_Sys::update()
{
    //进行eviction
    int count = 1;
    while(used_space>all_capacity)
    {
        //cout<<"Update..."<<endl;
        uint64_t max_key = 0;
        float max_val = 0.0;
        uint64_t delete_id = -1;
        map<uint64_t, float>::iterator It;
        It = b_recency.begin();
        while(It != b_recency.end())
        {
            uint64_t key_now = It->first;
            uint64_t val_now = It->second;
            if(max_val < val_now)
            {
                max_val = val_now;
                max_key = key_now;
            }
            It++;
        }
        delete_id = max_key;
        //cout<<used_space<<endl;
        used_space = used_space - cache[delete_id];
        //cout<<count<<" | "<<cache[delete_id]<<" | "<<b_recency.size()<<" | "<<used_space<<" | "<<all_capacity<<endl;
        count++;
        cache.erase(delete_id);
        b_recency.erase(delete_id);
        
        //map<uint64_t, float>(b_recency).swap(b_recency);
        //map<uint64_t,uint64_t>(cache).swap(cache);
    }
    
}

void Cache_Sys::decide(vector<uint64_t> Req, float prob, float nrt)
{
    uint64_t Id = Req[1];
    uint64_t Time = Req[0];
    uint64_t Size = Req[2];
    float Prob = prob;
    float T = 0.5;
    //找到当前cache里面是否存在该item
    bool In = 0;
    if(cache.find(Id) != cache.end()){
        In = 1;
    }

    last_req_time[Id] = timer;

    float score = nrt;
    //更新eviction依据-past time
    map<uint64_t, uint64_t>::iterator It;
    It = cache.begin();
    while(It != cache.end()){
        uint64_t key_now = It->first;
        if(key_now!=Id){
            float lrt_now = float(last_req_time[key_now]*1.0);
            float br_now = b_recency[key_now];
            float pt_now = timer - lrt_now + 0.0;
            b_recency[key_now] = std::max(pt_now,br_now);
        }
        else{
            b_recency[key_now] = score;
        }
        It++;
    }
    
    if(In == 1)
    {
        hits++;
    }
    else
    {
        misses++;
    }
    
    if(Prob > T && In == 0)
    {
        //更新或者添加记录
        cache[Id] = Size;
        b_recency[Id] = score;
        used_space += Size;
    }
    
    //更新数据
    //cout<<"cached items:"<<cache.size()<<endl;
    update();
    timer++;
    
}




#endif /* cache_sys_rlb_h */
