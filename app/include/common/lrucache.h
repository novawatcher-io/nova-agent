//
// Created by zhanglei on 2025/2/19.
//

#pragma once

#include <unordered_map>
#include <list>
#include <cstddef>
#include <stdexcept>
#include <mutex>

namespace App {
namespace Common {

template<typename key_t, typename value_t>
class LruCache {
public:
	typedef typename std::pair<key_t, value_t> key_value_pair_t;
	typedef typename std::list<key_value_pair_t>::iterator list_iterator_t;

	LruCache(size_t max_size) :
		_max_size(max_size) {
	}

	void put(const key_t& key, const value_t& value) {
        std::lock_guard guard(mtx);
		auto it = _cache_items_map.find(key);
		_cache_items_list.push_front(key_value_pair_t(key, value));
		if (it != _cache_items_map.end()) {
			_cache_items_list.erase(it->second);
			_cache_items_map.erase(it);
		}
		_cache_items_map[key] = _cache_items_list.begin();

		if (_cache_items_map.size() > _max_size) {
			auto last = _cache_items_list.end();
			last--;
			_cache_items_map.erase(last->first);
			_cache_items_list.pop_back();
		}
	}

	const value_t& get(const key_t& key) {
        std::lock_guard guard(mtx);
		auto it = _cache_items_map.find(key);
		if (it == _cache_items_map.end()) {
			throw std::range_error("There is no such key in cache");
		} else {
			_cache_items_list.splice(_cache_items_list.begin(), _cache_items_list, it->second);
			return it->second->second;
		}
	}

	bool exists(const key_t& key) {
        std::lock_guard guard(mtx);
		return _cache_items_map.find(key) != _cache_items_map.end();
	}

	size_t size() const {
        std::lock_guard guard(mtx);
		return _cache_items_map.size();
	}


	void clear() {
        std::lock_guard guard(mtx);
		_cache_items_list.clear();
        _cache_items_map.clear();
	}

private:
	std::list<key_value_pair_t> _cache_items_list;
	std::unordered_map<key_t, list_iterator_t> _cache_items_map;
	size_t _max_size;
    std::mutex mtx;
};

}
}