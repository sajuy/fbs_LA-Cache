//
//  get_policy.h
//  RLB-Modules
//
//  Created by Gang Yan on 11/15/20.
//

#ifndef get_policy_h
#define get_policy_h

#include <cmath>
#include <vector>
#include <map>
#include <stdlib.h>
#include "cache_sys_rlb.h"
#include <fstream>
#include <stdlib.h>
#include <sstream>

using namespace std;

class Get_Policy
{
public:
    vector<uint64_t> Lso;
    vector<float> Opts;
    vector<vector<uint64_t>> Reqs;
    vector<float> Nrts;
    vector<float> Probs;
    map<uint64_t,float> Est_Nrt;
    map<uint64_t,uint64_t> Caches;
    uint64_t K;
    Cache_Sys Simulator;
    float hits;
    uint64_t Cache_Space;
    map<uint64_t,uint64_t> Sizes;//记录每个item的size
    vector<float> Final_OPTs;//记录最终得到的OPT
    uint64_t Final_Hits;
    
    void init(vector<vector<uint64_t>>reqs,vector<float> nrts,vector<float> probs,uint64_t cache_space,map<uint64_t,uint64_t> caches,uint64_t k);//初始化
    void lso_compute();//计算LSO
    void opt_compute();//计算optimal policy
};

void Get_Policy::init(vector<vector<uint64_t>> reqs,vector<float> nrts,vector<float> probs,uint64_t cache_space,map<uint64_t,uint64_t> caches,uint64_t k){
    Reqs = reqs;
    Nrts = nrts;
    Probs = probs;
    Cache_Space = cache_space;
    Caches = caches;
    K = k;
    
}

void Get_Policy::lso_compute(){
    //统计每个request的next request time
    //首先估计每个item对应的平均Next Request Time
    map<uint64_t,vector<uint64_t>> Occurs;
    for(int i=0;i<Reqs.size();i++){
        vector<uint64_t> req = Reqs[i];
        uint64_t Time = req[0];
        uint64_t Id = req[1];
        uint64_t Size = req[2];
        Occurs[Id].push_back(Time);
        Sizes[Id] = Size;
    }
    
    //cout<<"Reqs的长度："<<Reqs.size()<<endl;
    
    map<uint64_t,vector<uint64_t>> NRTs;
    map<uint64_t,uint64_t> allocate;//为每个request分配next request time所定义的计数器
    map<uint64_t,vector<uint64_t>>::iterator It1;
    It1 = Occurs.begin();
    while(It1 != Occurs.end()){
        uint64_t key_now = It1->first;
        allocate[key_now] = 0;
        vector<uint64_t> occurs = Occurs[key_now];
        float est_nrt = 10000.0;
        vector<uint64_t> NRTs_now;
        if(occurs.size()>1){
            float sum_now = 0.0;
            float count_now = 0.0;
            for(int j=0;j<occurs.size()-1;j++){
                uint64_t nt = occurs[j+1] - occurs[j];
                sum_now += nt;
                count_now++;
                NRTs_now.push_back(nt);
            }
            est_nrt = uint64_t(sum_now / count_now);
        }
        NRTs_now.push_back(est_nrt);
        NRTs[key_now] = NRTs_now;
        Est_Nrt[key_now] = est_nrt;
        It1++;
    }
    
    //cout<<"Num of Items:"<<Est_Nrt.size()<<endl;
    
    //其次，分配所有request的next request time
    vector<uint64_t> Nexts;
    for(int l=0;l<Reqs.size();l++){
        vector<uint64_t> req = Reqs[l];
        uint64_t key_now = req[1];
        uint64_t counter_now = allocate[key_now];
        vector<uint64_t> NRTs_now = NRTs[key_now];
        float nrt_now = NRTs_now[counter_now];
        Nexts.push_back(nrt_now);
        allocate[key_now]++;
    }
    
    //最后，计算每个request内含的request数目
    vector<uint64_t> Overlaps;
    vector<uint64_t> Overlaps_Backup;
    vector<uint64_t> Valid_Overlap;
    for(int m=0;m<Reqs.size();m++){
        vector<uint64_t> req = Reqs[m];
        uint64_t key_now = req[1];
        uint64_t next_now = Nexts[m];
        uint64_t num = 0;
        uint64_t valid_time = m+next_now;
        if(m+next_now>Reqs.size())
            valid_time = Reqs.size();
        if(m+next_now>Reqs.size()){
            Overlaps.push_back(10000);
        }
        else{
            for(int n=m+1;n<valid_time;n++){
                vector<uint64_t> req_new = Reqs[n];
                uint64_t next_new = Nexts[n];
                if(n+next_new < m+next_now){
                    num++;
                }
            }
            uint64_t size_now = 1.0;//log(req[2]+0.00001);
            Overlaps.push_back(num * size_now);
            Valid_Overlap.push_back(num * size_now);
        }
    }
    
    Overlaps_Backup = Overlaps;
    std::sort (Valid_Overlap.begin(), Valid_Overlap.begin()+Valid_Overlap.size());
    std::sort (Overlaps_Backup.begin(), Overlaps_Backup.begin()+Overlaps_Backup.size());
    //cout<<"有效的数据长度为："<<Valid_Overlap.size()<<endl;
    
    //cout<<"Overlap Size:"<<Overlaps.size()<<endl;
    
    
    vector<uint64_t> Get_Overlap = Valid_Overlap;//Overlaps_Backup;//
    uint64_t valid_per1 = uint64_t(Get_Overlap.size()*0.05);
    uint64_t valid_per2 = uint64_t(Get_Overlap.size()*0.95);
    uint64_t Per1 = Get_Overlap[valid_per1];
    uint64_t Per2 = Get_Overlap[valid_per2];
    
    for(int i=0;i<Overlaps.size();i++){
        uint64_t lap = Overlaps[i];
        if(lap <= Per1){
            Lso.push_back(0);
            Opts.push_back(0.9);
        }
        if(lap >= Per2){
            Lso.push_back(0);
            Opts.push_back(0.1);
        }
        if(lap > Per1 && lap < Per2){
            Lso.push_back(1);
            Opts.push_back(0.5);
        }
    }
    //cout<<"LSO Size:"<<Lso.size()<<endl;
   // cout<<"OPTs Size:"<<Opts.size()<<endl;
    //cout<<"估计NRT的有效长度:"<<Est_Nrt.size()<<endl;
    std::ofstream outFile;
    outFile.open("/home/gangyan/桌面/ATS_Testing/opt_com.log",ios::app);
    outFile<<"LSO Finished..."<<"\n";
    outFile.close();
}


void Get_Policy::opt_compute(){
    Simulator.init(Cache_Space,Caches);
    Simulator.reset();
    //lso_compute();
    
    uint64_t init_num = K;
    //cout<<"Init Num is:"<<init_num<<endl;
    
    int Num_MC = 5;
    int counter = 0;
    vector<float> Best_Opts;
    float Best_Hits = 0;
    while(counter < Num_MC){
        Cache_Sys Sim_Now = Simulator;
        vector<float> Opts_Now = Opts;
        vector<float> get_opts;
        srand((unsigned)time(NULL));
        for(uint64_t i=init_num; i<Lso.size(); i++){
            vector<uint64_t> req = Reqs[i];
            uint64_t get_opt = Lso[i];
            float opt_now = Opts_Now[i];
            float nrt_now = Nrts[i];
            //if(get_opt == 1||get_opt == 0){
            if(get_opt == 1){
                float prob = Probs[i];
                float rd = (rand() % 100000) / 100000.0;
                if(rd > prob)
                {
                    Opts_Now[i] = 0.2;
                    opt_now = 0.2;
                }
                else{
                    Opts_Now[i] = 0.8;
                    opt_now = 0.8;
                }
            }
            //opt_now = 0.6;
            get_opts.push_back(opt_now);
            
            Sim_Now.decide(req,opt_now,nrt_now);
            if(i%5000==0){
                cout<<"Cached Items:"<<Sim_Now.cache.size()<<endl;
                cout<<"Used Space:"<<Sim_Now.used_space<<" | "<<Sim_Now.all_capacity<<endl;
            }
        }
        
        uint64_t hits_now = Sim_Now.hits;
        
        if(Best_Hits < hits_now){
            Best_Hits = hits_now;
            Best_Opts = get_opts;
        }
        
        //cout<<counter<<"-th Hits Now:"<<hits_now<<endl;
        //cout<<"-------------------------"<<endl;
        counter += 1;
    }
    
    //选出最好的OPT
    vector<float>::iterator Begin1 = Best_Opts.begin() + init_num;
    vector<float>::iterator End1 = Best_Opts.end();
    vector<float> Opts_Lso(Begin1,End1);
    float Hits_Lso = Best_Hits;
    
    //按照admit all进行缓存得到hit rates
    Cache_Sys Admit_All = Simulator;
    vector<float> Opts_All;
    for(uint64_t i=init_num; i<Lso.size(); i++){
        vector<uint64_t> req = Reqs[i];
        Opts_All.push_back(0.8);
        float nrt_now = Nrts[i];
        Admit_All.decide(req,0.8,nrt_now);
    }
    
    float Hits_All = Admit_All.hits;
    
    

    //选择确定最优OPT
    cout<<Hits_Lso<<" | "<<Hits_All<<endl;
    Final_Hits = Hits_Lso > Hits_All ? Hits_Lso : Hits_All;
    vector<float> Final_Opts = Hits_Lso > Hits_All ? Opts_Lso : Opts_All;
    
    //cout<<"OPT Size: "<<Final_OPTs.size()<<endl;
    std::ofstream outFile;
    outFile.open("/home/gangyan/桌面/ATS_Testing/opt_com.log",ios::app);
    outFile<<"OPT Finished..."<<"\n";
    outFile.close();
}

#endif /* get_policy_h */
