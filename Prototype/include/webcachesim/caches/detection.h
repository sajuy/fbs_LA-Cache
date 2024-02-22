//
//  Detection.h
//  ATS- One Model Method
//
//  Created by Gang Yan on 9/26/20.
//  Copyright © 2020 Gang Yan. All rights reserved.
//

#ifndef Detection_h
#define Detection_h

#include <iostream>
#include <vector>
#include <map>

using namespace std;

typedef struct{
    float k;
    float b;
}Linekb;


typedef pair<uint64_t, uint64_t> PAIR;
 
struct cmp  //自定义比较规则
{
    bool operator() (const PAIR& P1, const PAIR& P2)  //注意是PAIR类型，需要.firt和.second。这个和map类似
    {
        return P1.second > P2.second;
    }
};



//进行最小二乘拟合
Linekb CalcLine(vector<float> srcX, vector<float> srcY)
{
    Linekb LKB;
    float sumX = 0, sumY = 0, s_xy = 0, s_xx = 0;
    int nX = (int)srcX.size();
    for(int i=0;i<nX;i++)
    {
        sumX += srcX[i];
        sumY += srcY[i];
        s_xy += srcX[i] * srcY[i];
        s_xx += srcX[i] * srcX[i];
    }
    
    float _x = sumX / (nX + pow(0.1, 20));
    float _y = sumY / (nX + pow(0.1, 20));
    
    LKB.k = (s_xy - nX * _x * _y) / (s_xx - nX * _x * _x + 0.000000000001);
    LKB.b = _y - LKB.k * _x;
    
    return LKB;
    
}

//检查某个元素是否在向量中
//int in_vec(uint64_t ele, vector<uint64_t> vec)
//{
//    int In = 0;
//    for(int i=0;i<vec.size();i++)
//    {
//        if(vec[i] == ele)
//        {
//            In = 1;
//            break;
//       }
//    }
    
//    return In;
//}

//统计并转换trace数据，同时估计zipf分布
float Zipf(vector<vector<uint64_t>> Reqs)
{
    float alpha = 0;
    uint64_t num_data = Reqs.size();
    //cout<<"Num of Data:"<<num_data<<endl;
    //统计出现频率
    map<uint64_t, uint64_t> Freq;
    vector<uint64_t> IDs;
    for(int i=0;i<num_data;i++)
    {
        vector<uint64_t> req_now = Reqs[i];
        uint64_t id_now = req_now[1];
        int In = in_vec(id_now, IDs);
        if(In == 0)
        {
            Freq[id_now] = 1;
            IDs.push_back(id_now);
        }
        else
        {
            Freq[id_now] += 1;
        }
    }
    //转换为XY形式，进行最小二乘拟合
    vector<float> X;
    vector<float> Y;
    
    //根据出现频率进行排序，同时分配所属ID
    map<uint64_t, uint64_t>::iterator iter;
    
    vector<PAIR> New_Freq;
    for(iter=Freq.begin(); iter!=Freq.end();iter++)
           New_Freq.push_back(*iter);
    //转化为PAIR的vector
    sort(New_Freq.begin(), New_Freq.end(), cmp());  //需要指定cmp
    cout<<New_Freq[0].first<<" "<<New_Freq[0].second<<endl;
    for(int i=0;i<New_Freq.size();i++)
    {
        uint64_t key_new = i+1;
        uint64_t freq = New_Freq[i].second;
        float x = log(key_new);
        float y = log(freq/(num_data+0.0000000000000001));
        X.push_back(x);
        Y.push_back(y);
    }
    
    //cout<<"X and Y:"<<X[0]<<" "<<X[100]<<"|"<<Y[0]<<" "<<Y[100]<<endl;
    
    Linekb KB = CalcLine(X, Y);
    
    alpha = abs(KB.k);
    
    
    return alpha;
}





#endif /* Detection_h */
