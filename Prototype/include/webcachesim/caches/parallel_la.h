#ifndef WEBCACHESIM_PARALLEL_LA_H
#define WEBCACHESIM_PARALLEL_LA_H

#include "parallel_cache.h"
#include <atomic>
#include <unordered_map>
#include <vector>
#include <string>
#include <chrono>
#include <random>
#include <assert.h>
#include <LightGBM/c_api.h>
#include <mutex>
#include <thread>
#include <queue>
#include <shared_mutex>
#include <list>
#include <cmath>
#include "sparsepp/spp.h"
#include <map>
#include <fstream>
#include <exception>

using namespace webcachesim;

using namespace std;

using spp::sparse_hash_map;

typedef uint64_t LAKey;

using bsoncxx::builder::basic::kvp;

using bsoncxx::builder::basic::sub_array;

namespace ParallelLA {

    uint8_t max_n_past_timestamps = 32;

    uint8_t max_n_past_distances = 31;

    uint8_t base_edc_window = 10;

    const uint8_t n_edc_feature = 10;

    vector<uint32_t> edc_windows;

    vector<double> hash_edc;

    uint32_t max_hash_edc_idx;

    uint32_t memory_window = 1;

    uint32_t n_extra_fields = 0;

    const uint max_n_extra_feature = 4;

    uint32_t n_feature;
}

struct ParalllelLAMetaExtra {

    float _edc[10];

    vector<uint32_t> _past_distances;

    uint8_t _past_distance_idx = 1;

    ParalllelLAMetaExtra(const uint32_t &distance) {

        _past_distances = vector<uint32_t>(1, distance);

        for (uint8_t i = 0; i < ParallelLA::n_edc_feature; ++i) {

            uint32_t _distance_idx = min(uint32_t(distance / ParallelLA::edc_windows[i]),
                                         ParallelLA::max_hash_edc_idx);

            _edc[i] = ParallelLA::hash_edc[_distance_idx] + 1;

        }
    }

    void update(const uint32_t &distance) {

        uint8_t distance_idx = _past_distance_idx % ParallelLA::max_n_past_distances;

        if (_past_distances.size() < ParallelLA::max_n_past_distances)
            _past_distances.emplace_back(distance);

        else
            _past_distances[distance_idx] = distance;

        assert(_past_distances.size() <= ParallelLA::max_n_past_distances);

        _past_distance_idx = _past_distance_idx + (uint8_t) 1;

        if (_past_distance_idx >= ParallelLA::max_n_past_distances * 2)
            _past_distance_idx -= ParallelLA::max_n_past_distances;

        for (uint8_t i = 0; i < ParallelLA::n_edc_feature; ++i) {

            uint32_t _distance_idx = min(uint32_t(distance / ParallelLA::edc_windows[i]),
                                         ParallelLA::max_hash_edc_idx);

            _edc[i] = _edc[i] * ParallelLA::hash_edc[_distance_idx] + 1;
        }
    }
};

class ParallelLAMeta {

public:

    uint64_t _key;

    uint32_t _size;

    uint32_t _past_timestamp;

    uint16_t _extra_features[ParallelLA::max_n_extra_feature];

    ParalllelLAMetaExtra *_extra = nullptr;

    vector<uint32_t> _sample_times;

    ParallelLAMeta(const uint64_t &key, const uint64_t &size, const uint64_t &past_timestamp,
                    const uint16_t *&extra_features) {

        _key = key;

        _size = size;

        _past_timestamp = past_timestamp;

        for (int i = 0; i < ParallelLA::n_extra_fields; ++i)
            _extra_features[i] = extra_features[i];
    }

    virtual ~ParallelLAMeta() = default;

    void emplace_sample(uint32_t &sample_t) {

        _sample_times.emplace_back(sample_t);

    }

    void free() {

        delete _extra;

    }

    void update(const uint32_t &past_timestamp) {

        //distance
        uint32_t _distance = past_timestamp - _past_timestamp;

        assert(_distance);

        if (!_extra) {

            _extra = new ParalllelLAMetaExtra(_distance);

        } else
            _extra->update(_distance);

        //timestamp
        _past_timestamp = past_timestamp;

    }

    int feature_overhead() {

        int ret = sizeof(ParallelLAMeta);

        if (_extra)
            ret += sizeof(ParalllelLAMetaExtra) - sizeof(_sample_times) +
                   _extra->_past_distances.capacity() * sizeof(uint32_t);

        return ret;
    }

    int sample_overhead() {

        return sizeof(_sample_times) + sizeof(uint32_t) * _sample_times.capacity();

    }
};


class ParallelInCacheMeta : public ParallelLAMeta {

public:

    list<LAKey>::const_iterator p_last_request;

    ParallelInCacheMeta(const uint64_t &key,

                        const uint64_t &size,

                        const uint64_t &past_timestamp,

                        const uint16_t *&extra_features, 

                        const list<LAKey>::const_iterator &it) :
            ParallelLAMeta(key, size, past_timestamp, extra_features) {

        p_last_request = it;

    };

    ParallelInCacheMeta(const ParallelLAMeta &meta, const list<LAKey>::const_iterator &it) : ParallelLAMeta(meta) {

        p_last_request = it;

    };

};

class ParallelInCacheLRUQueue {

public:

    list<LAKey> dq;

    list<LAKey>::const_iterator request(LAKey key) {

        dq.emplace_front(key);

        return dq.cbegin();
    }

    list<LAKey>::const_iterator re_request(list<LAKey>::const_iterator it) {

        if (it != dq.cbegin()) {

            dq.emplace_front(*it);

            dq.erase(it);
        }

        return dq.cbegin();
    }
};

typedef pair<uint64_t, float> PAIR;

struct Cmp
{
    bool operator()(const PAIR& P1, const PAIR& P2){

        return P1.second > P2.second;

    }
};

class IRTs{

public:

    map<uint64_t,list<uint64_t>> Lams;

    uint64_t L = 10;

    void insert(uint64_t Id,uint64_t Irt){

        bool delpop = false;

        if(Lams.find(Id) == Lams.end()){

            delpop = true;

        }

        Lams[Id].push_back(Irt);

        if(Lams[Id].size() > L){

            Lams[Id].pop_front();

        }

        if(delpop){

            Lams[Id].pop_front();

        }
    }

    double getAvg(uint64_t Id){

        double avg = 0.0000000001;

        if(Lams[Id].size() >= 3){

            uint64_t Sum = std::accumulate(Lams[Id].begin(),Lams[Id].end(),0.0);

            avg = 1.0 / (Sum / (double)Lams[Id].size() / 1.0);
        }

        return avg;
    }

};

struct KeyMapEntry {

    unsigned int list_idx: 1;

    unsigned int list_pos: 31;

};


class ParallelLACache : public ParallelCache {

public:

    sparse_hash_map<uint64_t, KeyMapEntry> key_map;

    vector<ParallelInCacheMeta> in_cache_metas;

    vector<ParallelLAMeta> out_cache_metas;

    unordered_map<uint64_t, float> EvictRule;

    vector<uint64_t> IdList;

    map<uint64_t, uint64_t> All_Sizes;

    uint64_t DynHits = 0;

    double BaseDelay = 200.0;

    double Bwidth = 20971520000.0;

    map<uint64_t,double> Delays;

    map<uint64_t, uint64_t> DynLRTs;

    uint64_t TrainCounter = 1;

    float Beta = 0.2;

    IRTs GetLams;

    uint32_t timer = 0;

    ParallelInCacheLRUQueue in_cache_lru_queue;

    sparse_hash_map<uint64_t, uint64_t> negative_candidate_queue;

    std::mutex training_data_mutex;

    std::mutex LA_mutex;

    uint32_t t_counter = 0;

    std::thread training_thread;

    BoosterHandle booster = nullptr;

    std::mutex booster_mutex;

    bool if_trained = false;

    std::default_random_engine _generator = std::default_random_engine();

    std::uniform_int_distribution<std::size_t> _distribution =         std::uniform_int_distribution<std::size_t>();

    ~ParallelLACache() {

        keep_running = false;

        if (lookup_get_thread.joinable())
            lookup_get_thread.join();

        if (training_thread.joinable())
            training_thread.join();

        if (print_status_thread.joinable())
            print_status_thread.join();
    }

    void init_with_params(const map<string, string> &params) override {

        //set params
        for (auto &it: params) {
        
        }

        negative_candidate_queue.reserve(ParallelLA::memory_window);

        ParallelLA::max_n_past_distances = ParallelLA::max_n_past_timestamps - 1;

        //init
        ParallelLA::edc_windows = vector<uint32_t>(ParallelLA::n_edc_feature);

        for (uint8_t i = 0; i < ParallelLA::n_edc_feature; ++i) {

            ParallelLA::edc_windows[i] = pow(2, ParallelLA::base_edc_window + i);

        }

        ParallelLA::max_hash_edc_idx =
                (uint64_t) (ParallelLA::memory_window / pow(2, ParallelLA::base_edc_window)) - 1;

        ParallelLA::hash_edc = vector<double>(ParallelLA::max_hash_edc_idx + 1);

        for (int i = 0; i < ParallelLA::hash_edc.size(); ++i)
            ParallelLA::hash_edc[i] = pow(0.5, i);

        //interval, distances, size, extra_features, n_past_intervals, edwt
        ParallelLA::n_feature =
                ParallelLA::max_n_past_timestamps + ParallelLA::n_extra_fields + 2 + ParallelLA::n_edc_feature;

        if (ParallelLA::n_extra_fields) {

            if (ParallelLA::n_extra_fields > ParallelLA::max_n_extra_feature) {

                cerr << "error: only support <= " + to_string(ParallelLA::max_n_extra_feature)
                        + " extra fields because of static allocation" << endl;

                abort();
            }

            string categorical_feature = to_string(ParallelLA::max_n_past_timestamps + 1);

            for (uint i = 0; i < ParallelLA::n_extra_fields - 1; ++i) {
                categorical_feature += "," + to_string(ParallelLA::max_n_past_timestamps + 2 + i);
            }
        }

        training_thread = std::thread(&ParallelLACache::async_training, this);

        ParallelCache::init_with_params(params);
    }


    void print_stats() override {

        std::cerr << "cache size: " << _currentSize << "/" << _cacheSize << " (" << ((double) _currentSize) / _cacheSize
                  << ")" << std::endl
                  << "in/out metadata " << in_cache_metas.size() << " / " << out_cache_metas.size() << std::endl;
    }

    void async_training() {

        while (keep_running) {

            std::this_thread::sleep_for(std::chrono::milliseconds(200000));

        }
    }

    pair<uint64_t, uint32_t> rank();

    void async_lookup(const uint64_t &key) override;

    void async_admit(const uint64_t &key, const int64_t &size, const uint16_t extra_features[max_n_extra_feature]) override;

    void evict();


    bool has(const uint64_t &id) {

        auto it = key_map.find(id);

        if (it == key_map.end())

            return false;

        return !it->second.list_idx;
    }

    void update_stat(bsoncxx::v_noabi::builder::basic::document &doc) override {

        uint64_t feature_overhead = 0;

        uint64_t sample_overhead = 0;

        for (auto &m: in_cache_metas) {

            feature_overhead += m.feature_overhead();

            sample_overhead += m.sample_overhead();

        }

        for (auto &m: out_cache_metas) {

            feature_overhead += m.feature_overhead();

            sample_overhead += m.sample_overhead();
        }

        doc.append(kvp("n_metadata", to_string(key_map.size())));

        doc.append(kvp("feature_overhead", to_string(feature_overhead)));

        doc.append(kvp("sample ", to_string(sample_overhead)));

        int res;

        auto importances = vector<double>(ParallelLA::n_feature, 0);

        if (booster) {

            res = LGBM_BoosterFeatureImportance(booster,
                                                0,
                                                1,
                                                importances.data());
            if (res == -1) {

                cerr << "error: get model importance fail" << endl;

                abort();
            }
        }

        doc.append(kvp("model_importance", [importances](sub_array child) {

            for (const auto &element : importances)
                child.append(element);

        }));
    }

};

static Factory<ParallelLACache> factoryParallelLA("ParallelLA");

#endif //WEBCACHESIM_LA_H
