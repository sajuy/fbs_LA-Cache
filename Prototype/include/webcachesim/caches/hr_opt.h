//
//  HR_OPT.h
//  ATS- One Model Method
//
//  Created by Gang Yan on 9/26/20.
//  Copyright © 2020 Gang Yan. All rights reserved.
//

#ifndef HR_OPT_h
#define HR_OPT_h

#include <vector>
#include <cmath>
#include <map>

using namespace std;

typedef pair<uint64_t, float> PAIR_OPT;
 
struct CMP_OPT  //自定义比较规则
{
    bool operator() (const PAIR_OPT& P1, const PAIR_OPT& P2)  //注意是PAIR类型，需要.firt和.second。这个和map类似
    {
        return P1.second > P2.second;
    }
};


void print_vec_here(vector<uint64_t> Vec)
{
    cout<<"Size:"<<Vec.size()<<" ";
    for(int i=0;i<Vec.size();i++)
        cout << Vec[i] << " ";
    cout<<endl;
}

int in_vec(uint64_t id, vector<uint64_t> IDs){
   int In = 0;
   for(int i=0;i<IDs.size();i++){
	if(IDs[i]==id){
           In = 1;
           break;
        }
   }

   return In;
}

class HR_OPT
{
public:
    vector<vector<uint64_t>> Trace;
    map<uint64_t, uint64_t> Last_Req_Times;
    uint64_t K;
    uint64_t Num_Cache;
    uint64_t Capacity;
    map<uint64_t,vector<uint64_t>> Inter_Times;//获取当前trace中所有items的inter-arrival time
    map<uint64_t, float> Parameters;//记录估计的参数
    vector<float> OPT;
    map<uint64_t,float> Pops;//记录每个item对应的hazard rates
    vector<uint64_t> IDs_Hap; //记录出现的item对应的id
    vector<uint64_t> Own_Paras_IDs;//记录能够计算出参数的item的id
    vector<uint64_t> Last_Req_Time_IDs;//记录每个item的last request time
    map<uint64_t, uint64_t> Sizes;//记录item的size
    
    void init(vector<vector<uint64_t>> trace, uint64_t k, uint64_t num_cache,uint64_t capacity);
    float exponential(float R, uint64_t T);
    float Exponential(float R, uint64_t T);//使用指数分布
    float est(vector<uint64_t> data);//估计参数
    void get_paras();//批量获取参数
    void get_inter_times();//获取inter-arrival time
    float get_hr(uint64_t id_now,uint64_t T);
    float rank(uint64_t id_now, float hr_now); //更新Pops并进行排序选择
    void init_pops(uint64_t time_now, vector<uint64_t> IDs);//根据当前的时间戳更新Pops
    vector<float> estimate();
    
};


void HR_OPT::init(vector<vector<uint64_t>> trace, uint64_t k, uint64_t num_cache, uint64_t capacity)
{
    Trace = trace;
    K = k;
    Num_Cache = num_cache;
    Capacity = capacity;
}

float HR_OPT::exponential(float R, uint64_t T)
{
    return R * exp(- R * (T+0.000000000000001));
}

float HR_OPT::Exponential(float R, uint64_t T)
{
    if(T >= 0)
        return 1 - exp(- R * (T+0.000000000000001));
    else
        return 0;
}

float HR_OPT::est(vector<uint64_t> data)
{
    float sum = 0.0;
    for(int i=0;i<data.size();i++)
        sum += data[i];
    float mean = sum / (data.size()+0.00000001);
    float para = 1.0 / mean;
    return para;
}

void HR_OPT::get_inter_times()
{
    //按照ID对数据进行归类
    vector<uint64_t> Have_IDs;
    map<uint64_t,vector<uint64_t>> Occur_Times;
    //统计item出现的所有时间
    for(int i=0;i<Trace.size();i++)
    {
        vector<uint64_t> req_now = Trace[i];
        uint64_t id_now = req_now[1];
        uint64_t time_now = req_now[0];
        int In = in_vec(id_now, Have_IDs);
        if(In == 0)
        {
            vector<uint64_t> Add;
            Add.push_back(time_now);
            Occur_Times[id_now] = Add;
            Have_IDs.push_back(id_now);
        }
        else
        {
            vector<uint64_t> Add = Occur_Times[id_now];
            Add.push_back(time_now);
            Occur_Times[id_now] = Add;
        }
    }
    //根据统计的出现时间，计算inter-arrival time
    for(int i=0;i<Have_IDs.size();i++)
    {
        uint64_t id_now = Have_IDs[i];
        vector<uint64_t> occur_times = Occur_Times[id_now];
        if(occur_times.size()>1)
        {
            vector<uint64_t> inter_times;
            for(int j=0;j<occur_times.size()-1;j++)
            {
                uint64_t gap_time = occur_times[j+1] - occur_times[j];
                inter_times.push_back(gap_time);
            }
            Inter_Times[id_now] = inter_times;
        }
    }
}

void HR_OPT::get_paras()
{
    map<uint64_t,vector<uint64_t>>::iterator Iter;
    Iter = Inter_Times.begin();
    while(Iter != Inter_Times.end()) {
        uint64_t id_now = Iter->first;
        vector<uint64_t> times_now = Inter_Times[id_now];
        Own_Paras_IDs.push_back(id_now);
        //估计参数
        float R = est(times_now);
        Parameters[id_now] = R;
        Iter++;
    }
    cout<<endl;
}


float HR_OPT::get_hr(uint64_t id_now,uint64_t T)
{
    float hr = 0.0;
    int In = in_vec(id_now,Own_Paras_IDs);
    if(In == 1)
    {
        float para = Parameters[id_now];
        float pdf = exponential(para, T);
        float cdf = Exponential(para, T);
        hr = pdf / (1.0 - cdf + 0.00001);
    }
    return hr;
}


void HR_OPT::init_pops(uint64_t time_now, vector<uint64_t> IDs)
{
    for(int i=0;i<IDs.size();i++)
    {
        uint64_t id_now = IDs[i];
        uint64_t time_len = 100000;
        int In = in_vec(id_now, Last_Req_Time_IDs);
        if(In == 1)
            time_len = time_now - Last_Req_Times[id_now];
        float hr_now = get_hr(id_now, time_len);
        if(hr_now > 0.0)
        {
            Pops[id_now] = hr_now;
        }
    }
}

float HR_OPT::rank(uint64_t id_now, float hr_now)
{
    //更新
    if(hr_now > 0.0)
        Pops[id_now] = hr_now;
    //排序并筛选
    map<uint64_t, float>::iterator iter;
    vector<PAIR_OPT> New_Pops;
    for(iter=Pops.begin(); iter!=Pops.end();iter++)
           New_Pops.push_back(*iter);
    //转化为PAIR的vector,从大到小进行排序
    sort(New_Pops.begin(), New_Pops.end(), CMP_OPT());  //需要指定cmp
    vector<float> Scores;
    uint64_t used_space = 0;
    uint64_t count = 0;
    for(int i=0;i<New_Pops.size();i++)
    {
        uint64_t id_here = New_Pops[i].first;
        uint64_t size_here = Sizes[id_here];
        if(New_Pops[i].second >0)
        {
            Scores.push_back(New_Pops[i].second);
            if(used_space + size_here <= Capacity)
            {
                used_space += size_here;
                count += 1;
            }
        }
        
    }
    uint64_t valid_len = min(Num_Cache,Scores.size());
    float lower_value = New_Pops[valid_len-1].second;
    //cout<<"Valid Length:"<<valid_len<<" "<<hr_now<<" "<<lower_value<<endl;
    
    return lower_value;
}

vector<float> HR_OPT::estimate()
{
    vector<float> OPT;
    //获取inter times
    get_inter_times();
    //参数估计
    get_paras();
    
    //更新last request time
    uint64_t init_time = 0;
    for(int i=0;i<Trace.size()-K;i++)
    {
        vector<uint64_t> req_now = Trace[i];
        uint64_t id_now = req_now[1];
        uint64_t time_now = req_now[0];
        uint64_t size_now = req_now[2];
        init_time = time_now;
        Last_Req_Times[id_now] = time_now;
        int In = in_vec(id_now, Last_Req_Time_IDs);
        if(In == 0)
        {
            Last_Req_Time_IDs.push_back(id_now);
            Sizes[id_now] = size_now;
        }
        
    }
    
    //print_vec_here(Last_Req_Time_IDs);
    cout<<Parameters[1]<<endl;
    cout<<"Time:"<<init_time<<endl;
    
    
    //更新已出现过的IDs
    int valid_len = 0;
    if(Trace.size()-2*K >= 0){
       valid_len = Trace.size()-2*K;
    }
    else{
       valid_len = 0;
    }
    for(int i=valid_len;i<Trace.size()-K;i++)
    {
        vector<uint64_t> req_now = Trace[i];
        uint64_t id_now = req_now[1];
        int In = in_vec(id_now,IDs_Hap);
        if(In == 0)
            IDs_Hap.push_back(id_now);
    }
    
    //初始化Pops
    init_pops(init_time, IDs_Hap);
    cout<<"Pops:"<<Pops[1]<<" "<<Pops[2]+0.000000001<<endl;
    
    
    
    for(int i=Trace.size()-K;i<Trace.size();i++)
    {
        vector<uint64_t> req_now = Trace[i];
        uint64_t id_now = req_now[1];
        uint64_t time_now = req_now[0];
        uint64_t size_now = req_now[2];
        uint64_t time_len = 100000;
        int In = in_vec(id_now, Last_Req_Time_IDs);
        if(In == 1)
            time_len = time_now - Last_Req_Times[id_now];
        //计算hazard rate
        float hr_now = get_hr(id_now, time_len);
        float low_val = rank(id_now, hr_now);
        
        if(hr_now >= low_val)
            OPT.push_back(1.0);
        else
            OPT.push_back(0.0);
        
        //更新数据
        int In2 = in_vec(id_now,IDs_Hap);
        if(In2 == 0)
        {
            IDs_Hap.push_back(id_now);
            Sizes[id_now] = size_now;
        }
        int In3 = in_vec(id_now, Last_Req_Time_IDs);
        if(In3 == 0)
            Last_Req_Time_IDs.push_back(id_now);
        Last_Req_Times[id_now] = time_now;
        //更新Pops
        if(i % 1000 == 0 && i > 0)
        {
            init_pops(time_now, IDs_Hap);
        }
        
    }
    
    uint64_t Num_One = 0;
    for(int i=0;i<OPT.size();i++)
    {
        if(OPT[i] == 1)
            Num_One++;
    }
    cout<<"Num of Ones:"<<Num_One<<endl;
    return OPT;
}


#endif /* HR_OPT_h */
