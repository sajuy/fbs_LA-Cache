#include "parallel_la.h"
    
void ParallelLACache::async_lookup(const uint64_t &key) {

    auto InCache = key_map.find(key);

    if (InCache != key_map.end()) {
	
	uint64_t size = All_Sizes[key];

        training_data_mutex.lock();

        uint64_t Irt = timer - DynLRTs[key];

        GetLams.insert(key,Irt);

    	DynLRTs[key] = timer;

         float get_size = (float)size;

         float eval = GetLams.getAvg(key) * Delays[key];

         float update_val = eval * (1 + eval) / (2 + eval) / get_size / 1.0;

         EvictRule[key] = update_val;
        
	 ++timer;

	 training_data_mutex.unlock();
    }
}

void ParallelLACache::async_admit(const uint64_t &key, const int64_t &size, const uint16_t *extra_features) {

     auto InCache = key_map.find(key);

     if (InCache == key_map.end()) {  
	 
         uint64_t Size = uint64_t(size);

    	 All_Sizes[key] = Size;

         training_data_mutex.lock();

        uint64_t Irt = 100000000;

        if(DynLRTs.find(key) != DynLRTs.end()){

            Irt = timer - DynLRTs[key];

        }
        
        GetLams.insert(key,Irt);

        Delays[key] = BaseDelay + size / Bwidth * 1000 / 1.0; 

    	DynLRTs[key] = timer;

        float get_size = (float)Size;

        float eval = GetLams.getAvg(key) * Delays[key];

        float update_val = eval * (1 + eval) / (2 + eval) / get_size / 1.0;

        EvictRule[key] = update_val;
         
	training_data_mutex.unlock();

    	if (true) {

              key_map.insert({key, KeyMapEntry{.list_idx=0, .list_pos = (uint32_t) in_cache_metas.size()}});

              auto shard_id = key%n_shard; 

              size_map_mutex[shard_id].lock();

              size_map[shard_id].insert({key, size});

              size_map_mutex[shard_id].unlock();

              auto lru_it = in_cache_lru_queue.request(key);

              in_cache_metas.emplace_back(key, size, timer, extra_features, lru_it);

              _currentSize += size;

              IdList.push_back(key);
        }

    	while (_currentSize > _cacheSize) {

        	evict();

    	}

        ++timer;
        
    }
}

pair<uint64_t, uint32_t> ParallelLACache::rank() {

    uint64_t find_key = -1;

    float find_val = 10000000000.0;

    uint64_t vlen = 512;

    uint64_t slen = IdList.size();

    uint32_t fix_timer = timer;

    unsigned seed = time(0);

    srand(seed);

    map<uint64_t, uint64_t> GLRTs = DynLRTs;

    for (uint64_t i = 0; i < vlen; i++) {

        uint64_t gid = rand() % slen;

        uint64_t key = IdList[gid];

        float val = EvictRule[key];

        if (val < find_val) {

            find_val = val;

            find_key = key;
        }
    }

    uint64_t delete_id = find_key;

    uint32_t find_pos = -1;

    auto it = key_map.find(delete_id);

    find_pos = it->second.list_pos;

    auto &meta = in_cache_metas[find_pos];

    find_key = meta._key;	

    EvictRule.erase(find_key);

    unordered_map<uint64_t,float>(EvictRule).swap(EvictRule);

    std::remove(IdList.begin(), IdList.end(), find_key);

    IdList.pop_back();

    vector<uint64_t>(IdList).swap(IdList);

    return {find_key, find_pos};
}

void ParallelLACache::evict() {

    auto epair = rank();

    uint64_t &key = epair.first;

    uint32_t &old_pos = epair.second;

    auto &meta = in_cache_metas[old_pos];

    if(true){
        if (!meta._sample_times.empty()) {

            meta._sample_times.clear();

            meta._sample_times.shrink_to_fit();
        }

        in_cache_lru_queue.dq.erase(meta.p_last_request);

        meta.p_last_request = in_cache_lru_queue.dq.end();

        meta.free();

        _currentSize -= meta._size;

        key_map.erase(key);
        
        auto shard_id = key%n_shard;

        size_map_mutex[shard_id].lock();

        size_map[shard_id].erase(key);

        size_map_mutex[shard_id].unlock();

        uint32_t activate_tail_idx = in_cache_metas.size() - 1;

        if (old_pos != activate_tail_idx) {

            in_cache_metas[old_pos] = in_cache_metas[activate_tail_idx];

            key_map.find(in_cache_metas[activate_tail_idx]._key)->second.list_pos = old_pos;

        }

        in_cache_metas.pop_back();

    }
}
