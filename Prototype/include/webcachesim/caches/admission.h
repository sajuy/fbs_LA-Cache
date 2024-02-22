//
//  admission.h
//  admission_sys
//
//  Created by Gang Yan on 7/4/20.
//  Copyright © 2020 Gang Yan. All rights reserved.
//

#ifndef admission_h
#define admission_h

#include "fnn.h"
#include "cache_sys.h"
#include <map>

//重载vector的一些操作符
template <typename T>
std::vector<T> operator+(std::vector<T> v1, const std::vector<T>& v2)
{
 if(v1.size()!= v2.size())
 {
     cout<<"Two vector size must be same"<<'\n';
 }

 for(unsigned int i = 0; i<v1.size(); i++)
 {
 v1[i] += v2[i];
 }
 return v1;
}

template <typename T>
std::vector<T> operator-(std::vector<T> v1, const std::vector<T>& v2)
{
 if(v1.size()!= v2.size())
 {
     cout<<"Two vector size must be same"<<'\n';
 }

 for(unsigned int i = 0; i<v1.size(); i++)
 {
 v1[i] -= v2[i];
 }
 return v1;
}


//定义一个存储特征的队列
class Quen_D
{
public:
    uint64_t Size;
    vector<double> Delta;
    void init(uint64_t);
    void operate(double);

private:
    
};

void Quen_D::init(uint64_t size)
{
    Size = size;
    Delta.resize(size,0);
}

void Quen_D::operate(double data_in)
{
    Delta.erase(Delta.begin());
    Delta.push_back(data_in);
}

//定义进行admission的框架
class RLB
{
private:
    uint64_t K;
    uint64_t M;
    list<uint64_t> IDs;
    uint64_t capacity;
    int d_in = 5;
    int d_out = 1;
    int d_hid = 10000;
    Cache_Sys cache_space;
    Cache_Sys cache_space_old;
    BPNN model_adm;
    vector<vector<double>> training_data_x;
    vector<vector<double>> training_data_y;
    vector<vector<uint64_t>> training_trace;//记录当前窗口的request的id，按照先后顺序
    vector<double> training_pred_b;//存储过程中的pred_b的值
    vector<double> training_prob;//存储过程中prob的值
    int if_trained = 0;
    double pred_belady = -1;
    double Moca_Hit = 0;
    
    
    
public:
    void init(uint64_t, uint64_t, uint64_t);
    vector<double> extractor(map<string, uint64_t> &);
    double multimodel(vector<BPNN> &,vector<double> &);
    void train();
    double decide(uint64_t &, uint64_t &, double &);
    vector<double> Moca(Cache_Sys &,vector<vector<uint64_t>> &,vector<uint64_t> &,vector<double> &,vector<double> &);
    //进行Monte Carlo过程
    void main();
    
    uint64_t num_req = 0;
    vector<double> Hit_Rates;//保存运行得到的hit rates
    map<uint64_t, vector<double>> Features;//保存每个request的特征值
    vector<BPNN> Multi_Models;//多模型结构，存储多个模型
    map<uint64_t, Quen_D> Belady_Fea;//存储不同id
    map<uint64_t, vector<double>> Med_Fea;//存储中间过渡数据
    int num_train = 0;//记录训练模型的次数
};

void RLB::init(uint64_t k, uint64_t m, uint64_t capa)
{
    K = k;
    M = m;
    capacity = capa;
    cache_space.init(capacity);
    cache_space_old.init(capacity);
    BPNN model_new(d_in,d_hid,d_out);
    model_new.set_momentum(0.5);
    model_new.set_learn_rate(0.1);
    model_adm = model_new;
    
}

vector<double> RLB::extractor(map<string, uint64_t> &Req)
{
    vector<double> result;
    num_req += 1;
    auto ID = Req["id"];
    auto size = Req["size"];
    bool found = (std::find(IDs.begin(), IDs.end(), ID) != IDs.end());//寻找该request之前是否出现过
    
    if (!found)
    {
        double s_j = log1p(size + 0.00001);
        double d_j = -1;
        //生成一个队列，存储数据
        Quen_D quen_now;
        quen_now.init(5);
        Belady_Fea[ID] = quen_now;
        Belady_Fea[ID].operate(d_j);
        vector<double> stat;
        stat.push_back(s_j);
        stat.push_back(d_j);
        stat.push_back(num_req);
        Med_Fea[ID] = stat;
        IDs.push_back(ID);
        result = Belady_Fea[ID].Delta;
    }
    
    if (found)
    {
        vector<double> last_data;
        last_data = Med_Fea[ID];
        double s_j = log1p(size + 0.00001);
        double d_j = log1p(num_req - last_data[2]);
        Belady_Fea[ID].operate(d_j);
        vector<double> stat;
        stat.push_back(s_j);
        stat.push_back(d_j);
        stat.push_back(num_req);
        Med_Fea[ID] = stat;
        result = Belady_Fea[ID].Delta;
    }
    
    return result;
}

double RLB::multimodel(vector<BPNN> &models,vector<double> &fea)
{
    double result;
    
    int sum = 0;
    for (int s=0;s<models.size();s++)
        sum += (s+1);

    double res = 0;
    for(int j=0;j<models.size();j++)
    {
        double res_now = models[j].compute(fea)[0];
        res += (res_now * (j+1)/(sum + 0.0001));
        cout<<res_now * (j+1)/(sum + 0.0001)<<" ";
    }
    cout<<res<<" "<<endl;

    result = res;

    return result;
}

double RLB::decide(uint64_t &key, uint64_t &size, double &pred_b)
{
    double prob = 0.6;
    //首先提取特征
    map<string, uint64_t> req;
    req["id"] = key;
    req["size"] = size;
    vector<double> fea_now = extractor(req);
    fea_now.push_back(pred_b);
    
    if (if_trained == 1)
    {
        prob = multimodel(Multi_Models, fea_now);
    }
    training_data_x.push_back(fea_now);//添加神经网络的训练数据X
    vector<uint64_t> trace_now;
    trace_now.push_back(key);
    trace_now.push_back(size);
    training_trace.push_back(trace_now);//添加当前key，用以进行LSO
    if (cache_space.once_full == 1)
    {
        //操作cache的simulator
        cache_space.decide(req, prob, pred_b);
        training_prob.push_back(prob);
        training_pred_b.push_back(pred_b);
    }
    else
    {
        prob = 0.8;
        cache_space.decide(req, prob, pred_b);
        training_prob.push_back(0.8);
        training_pred_b.push_back(pred_b);
    }
    
    return prob;
}

vector<double> RLB::Moca(Cache_Sys &cache, vector<vector<uint64_t>> &trace, vector<uint64_t> &occur, vector<double> &probs, vector<double> &pred_b)
{
    cout<<"Occur size:"<<occur.size()<<endl;
    vector<double> Labels;
    //精简化occur,避免出现大的错误
    vector<uint64_t> good_occur;
    for (int i=0;i<occur.size();i++)
    {
        auto ocr = occur[i];
        if (ocr < 1000)
            good_occur.push_back(ocr);
    }
    //基于精简化的occur，进行抽样，从而得到阈值点
    double perp1 = 0.05;
    double perp2 = 0.95;
    sort(good_occur.begin(), good_occur.end());
    int loc1 = perp1 * good_occur.size();
    int loc2 = perp2 * good_occur.size();
    uint64_t cut_node1 = good_occur[loc1];
    uint64_t cut_node2 = good_occur[loc2];
    
    //根据阈值点，对action进行限制
    vector<double> If_Rand;
    for (int i=0;i<occur.size();i++)
    {
        if (occur[i] <= cut_node1)
            If_Rand.push_back(0);
        else
        {
            if (occur[i] >= cut_node2)
                If_Rand.push_back(2);
            else
                If_Rand.push_back(1);
        }
    }
    
    vector<double> Ratings;
    vector<vector<double>> Actions;
    
    for (int m=0;m<M;m++)
    {
        //生成随机数，基于此，产生随机行为,进行抽样
        vector<double> Rands;
        srand((unsigned)time(NULL));
        for (int i=0;i<occur.size();i++)
        {
            double r = rand() % 100;
            double val = r / 100.0;
            Rands.push_back(val);
        }
        
        Cache_Sys cache_used = cache;
        cache.reset();
        
        double Rate = 0;
        vector<double> actions;
        
        for (int i=0;i<occur.size();i++)
        {
            map<string, uint64_t> req_now;
            uint64_t key_now = trace[i][0];
            uint64_t size_now = trace[i][1];
            req_now["id"] = key_now;
            req_now["size"] = size_now;
            auto prob_now = probs[i];
            auto pred_b_now = pred_b[i];
            if (If_Rand[i] == 0)
            {
                actions.push_back(1.0);
                double prob_now = 1.0;
                auto hit_now = cache_used.decide(req_now, prob_now, pred_b_now);
                if (hit_now)
                    Rate += 1;
            }
            else
            {
                if (If_Rand[i] == 2)
                {
                    actions.push_back(0);
                    double prob_now = 0.0;
                    auto hit_now = cache_used.decide(req_now, prob_now, pred_b_now);
                    if (hit_now)
                        Rate += 1;
                }
                else
                {
                    if (Rands[i] < prob_now)
                    {
                        actions.push_back(0.8);
                        double prob_now = 0.8;
                        auto hit_now = cache_used.decide(req_now, prob_now, pred_b_now);
                        if (hit_now)
                            Rate += 1;
                    }
                    else
                    {
                        actions.push_back(0.2);
                        double prob_now = 0.2;
                        auto hit_now = cache_used.decide(req_now, prob_now, pred_b_now);
                        if (hit_now)
                            Rate += 1;
                    }
                }
            }
        }
        Ratings.push_back(Rate);
        Actions.push_back(actions);
        vector<double>().swap(actions);
    }
    
    double Max_Rate = 0;
    int loc = -1;
    for (int k=0;k<M;k++)
    {
        if (Ratings[k] > Max_Rate)
        {
            Max_Rate = Ratings[k];
            loc = k;
        }
    }
    
    Moca_Hit = Max_Rate;
    vector<double> Result = Actions[loc];
    
    //释放内存
    vector<double>().swap(Labels);
    vector<uint64_t>().swap(good_occur);
    vector<double>().swap(If_Rand);
    vector<double>().swap(Ratings);
    vector<vector<double>>().swap(Actions);

    return Result;
}

void RLB::train()
{
    //进行LSO的前一步，寻找request出现的规律，next request time
    vector<uint64_t> next_req_time;
    vector<uint64_t> occur;
    for (int i=0;i<training_trace.size();i++)
    {
        auto id_now = training_trace[i];
        int count = 0;
        for (int j=i+1;j<training_trace.size();j++)
        {
            auto id_here = training_trace[j];
            count += 1;
            if (id_now == id_here)
            {
                next_req_time.push_back(count);
                break;
            }
            if (j == training_trace.size() - 1)
                next_req_time.push_back(10000000);
        }
    }
    
    next_req_time.push_back(10000000);
    
    //cout<<"next req time:"<<next_req_time.size()<<endl;
    
    for (int i=0;i<training_trace.size();i++)
    {
        auto id_now = training_trace[i];
        auto nr0 = next_req_time[i];
        int count = 0;
        int num_in = 0;
        for (int j=i+1;j<training_trace.size();j++)
        {
            auto id_here = training_trace[j];
            auto nr1 = next_req_time[j];
            if (count + nr1 < nr0)
                num_in += 1;
            if (id_now == id_here)
            {
                occur.push_back(num_in);
                break;
            }
            if (nr0 > 10000)
            {
                occur.push_back(1000);
                break;
            }
            count += 1;
        }
        
    }
    
    occur.push_back(1000);
    
    //cout<<"training_trace:"<<training_trace.size()<<endl;
    
    //强化训练模型
    
    int Iter = 0;
    int Max_Iter = 5;
    double rate_old = cache_space.hits;
    int good = 0;
    int bad = 0;
    
    while(Iter < Max_Iter)
    {
        vector<double> Probs;
        if (Iter > 0)
        {
            for (int n=0;n<training_data_x.size();n++)
            {
                auto pre = model_adm.compute(training_data_x[n])[0];
                Probs.push_back(pre);
            }
        }
        else
            Probs = training_prob;
        
        //cout<<"Probs大小："<<Probs.size()<<endl;
        //cout<<"输出Moca的结果："<<endl;
        vector<double> Ys;
        Ys = Moca(cache_space_old, training_trace, occur, Probs, training_pred_b);
        
        //将Ys转换成我们需要的形式
        for (int ts=0;ts<Ys.size();ts++)
        {
            vector<double> y_vec;
            y_vec.push_back(Ys[ts]);
            training_data_y.push_back(y_vec);
        }
        //cout<<endl;
        //cout<<cache_space_old.hits<<endl;
        //cout<<"数据X大小检查"<<training_data_x.size()<<" "<<training_data_x[0].size()<<endl;
        //cout<<"数据Y大小检查"<<training_data_y.size()<<" "<<training_data_y[0].size()<<endl;
        //cout<<Ys.size()<<endl;
        
        double rate_new = Moca_Hit;
        
        //cout<<"Ratings:"<<rate_old<<"|"<<rate_new<<endl;
        
        if (rate_new > rate_old)
        {
            good += 1;
            //cout<<"训练模型"<<endl;
            BPNN model_now = model_adm;
            model_now.learn_all(training_data_x, training_data_y, 5);
            model_adm.learn_all(training_data_x, training_data_y, 5);
            Multi_Models.push_back(model_now);
            rate_old = rate_new;
        }
        else
            bad += 1;
        
        vector<vector<double>>().swap(training_data_y);
        vector<double>().swap(Ys);
        
        //cout<<"模型大小为:"<<Multi_Models.size()<<endl;
        
        if (Multi_Models.size() > 4)
        {
            //cout<<"删除模型...";
            Multi_Models.erase(Multi_Models.begin());
        }
        
        if (good == 0 && bad == 0)
            break;
        
        if (bad == 2 || good == 2)
            break;
        
        vector<double>().swap(Probs);
        Iter += 1;
        
    }
    
    vector<vector<double>>().swap(training_data_x);
    vector<vector<uint64_t>>().swap(training_trace);
    vector<double>().swap(training_pred_b);
    vector<double>().swap(training_prob);
    
    if_trained = 1;
    
    cache_space_old = cache_space;//交换两个cache system,方便进行下次运算
    
}


void RLB::main()
{
    if (num_req % K == 0 && num_req > 0)
    {
        //cout<<endl;
        //cout<<"-----------------------------------------------------"<<endl;
        //cout<<"第"<<num_train + 1<<"次训练模型..."<<endl;
        //cout<<"Hit Rates is:"<<cache_space.hits<<endl;
        Hit_Rates.push_back(cache_space.hits);
        //cout<<num_req<<endl;
        train();
        cache_space.reset();
        num_train += 1;
    }
    
}


#endif /* admission_h */
