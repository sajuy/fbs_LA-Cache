//
//  Belady_OPT.h
//  ATS- One Model Method
//
//  Created by Gang Yan on 9/26/20.
//  Copyright © 2020 Gang Yan. All rights reserved.
//

#ifndef Belady_OPT_h
#define Belady_OPT_h

#include <cmath>
#include <vector>
#include <map>

using namespace std;

//定义函数，打印vector的元素
void print_vec_b(vector<float> Vec)
{
    cout<<"Size:"<<Vec.size()<<" ";
    for(int i=0;i<Vec.size();i++)
        cout << Vec[i] << " ";
    cout<<endl;
}

void print_map_b(map<uint64_t,uint64_t> M)
{
    map<uint64_t,uint64_t>::iterator Iter;
    Iter = M.begin();
    while(Iter != M.end())
    {
        cout<<Iter->first<<"|"<<Iter->second<<" ";
        Iter++;
    }
    cout<<endl;
}

class OPTGen
{
public:
    vector<vector<uint64_t>> Trace;
    uint64_t Num_Cache;
    uint64_t K;
    vector<float> Occupy;
    map<uint64_t,uint64_t> Last_Locs; //记录Item上一次出现的位置
    
    void init(vector<vector<uint64_t>> trace, uint64_t num_cache, uint64_t k);
    vector<float> estimate();
    
};

void OPTGen::init(vector<vector<uint64_t>> trace, uint64_t num_cache, uint64_t k)
{
    Trace = trace;
    Num_Cache = num_cache;
    K = k;
}

vector<float> OPTGen::estimate()
{
    vector<float> OPT;
    vector<uint64_t> Own_Ids;
    
    for(int i=0;i<Trace.size();i++)
    {
        vector<uint64_t> req_now = Trace[i];
        uint64_t id_now = req_now[1];
        uint64_t loc_now = uint64_t(i);
        uint64_t loc_last = -1;
        //添加occupy
        Occupy.push_back(0);
        OPT.push_back(0);
        
        int In = in_vec(id_now, Own_Ids);
        if(In == 0)
        {
            Own_Ids.push_back(id_now);
        }
        else
        {
            loc_last = Last_Locs[id_now];
        }
        //更新位置记录
        //cout<<"Loc Now:"<<loc_now<<endl;
        //print_map_b(Last_Locs);
        Last_Locs[id_now] = i;
        
        if(loc_last != -1)
        {
            uint64_t max_use_space = 0;
            for(int j=(int)loc_last;j<Occupy.size()-1;j++)
            {
                if(Occupy[j] > max_use_space)
                    max_use_space = Occupy[j];
            }
            if(max_use_space < Num_Cache)
            {
                for(int j=(int)loc_last;j<Occupy.size()-1;j++)
                {
                    Occupy[j] += 1;
                }
                OPT[loc_last] = 1;
            }
        }
        //print_vec_b(Occupy);
        
    }
    
    vector<float> Decisions;
    int loc_begin = int(OPT.size() - K);
    if(loc_begin < 0)
        loc_begin = 0;
    for(int b=loc_begin;b<OPT.size();b++)
    {
        Decisions.push_back(OPT[b]);
    }
    
    return Decisions;
}


#endif /* Belady_OPT_h */
