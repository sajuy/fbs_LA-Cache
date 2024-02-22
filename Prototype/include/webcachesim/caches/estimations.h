//
//  Estimations.h
//  ATS- One Model Method
//
//  Created by Gang Yan on 9/26/20.
//  Copyright © 2020 Gang Yan. All rights reserved.
//

#ifndef Estimations_h
#define Estimations_h

#include <cmath>
#include <vector>
#include <map>
#include "cache_sys.h"

using namespace std;

//对阈值进行估计

float max(float v1, float v2)
{
    if(v1<v2)
        return v2;
    else
        return v1;
}

float min(float v1, float v2)
{
    if(v1<v2)
        return v1;
    else
        return v2;
}

class DET_EST
{
public:
    vector<vector<uint64_t>> Trace;
    float threshold_now;
    float hits_now;
    vector<float> decisions_now;
    Cache_Sys cache_test;
    
    void init(vector<vector<uint64_t>> trace, Cache_Sys cache_sys, vector<float> decs,float threshold, float hits);
    float get_hits(Cache_Sys cache_sys, float threshold);
    float estimate();
    
};


void DET_EST::init(vector<vector<uint64_t>> trace, Cache_Sys cache_sys, vector<float> decs,float threshold, float hits)
{
    Trace = trace;
    threshold_now = threshold;
    cache_test = cache_sys;
    hits_now = hits;
    decisions_now = decs;
}


float DET_EST::get_hits(Cache_Sys cache_sys, float threshold)
{
    float hits = 0;
    Cache_Sys cache_sys_he = cache_sys;
    for(int i=0;i<Trace.size();i++)
    {
        vector<uint64_t> req_he = Trace[i];
        float decision_he = decisions_now[i];
        cache_sys_he.decide(req_he, decision_he, threshold);
    }
    hits = cache_sys_he.hits / (Trace.size() + 0.01);
    
    
    return hits;
}

float DET_EST::estimate()
{
    float hits_all = get_hits(cache_test, 0.0);
    float hits_half = get_hits(cache_test, 0.5);
    cout<<"Gap Hits:"<<hits_now<<" "<<hits_all<<" "<<hits_half<<endl;
    vector<float> Left = {0.1,0.2,0.3,0.4};
    vector<float> Right = {0.6,0.7,0.8,0.9};
    vector<float> Interval_Get;
    float t1 = threshold_now - 0.1;
    float t2 = threshold_now + 0.1;
    if(t1 > 0 && t1 != 0.5){
        Interval_Get.push_back(t1);
    }
    if(t2 <= 1.0 && t2 != 0.5){
        Interval_Get.push_back(t2);
    }
    //选取测试的有效区间
    
    float best_hits = 0.0;
    float best_thresh = 0.0;
    if(hits_all > hits_half)
    {
        best_hits = hits_all;
        best_thresh = 0.0;
    }
    else
    {
        best_hits = hits_half;
        best_thresh = 0.5;
    }
    for(int i=0;i<Interval_Get.size();i++)
    {
        float threshold_new = Interval_Get[i];
        float hits_new = get_hits(cache_test, threshold_new);
        if(hits_new>best_hits)
        {
            best_hits = hits_new;
            best_thresh = threshold_new;
        }
    }
    
    if(best_hits < hits_now || abs(best_hits - hits_now) < 0.001)
    {
        best_thresh = threshold_now;
    }
    
    return best_thresh;
}


#endif /* Estimations_h */
