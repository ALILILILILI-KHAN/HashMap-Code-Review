#include <functional>
#include <initializer_list>
#include <iostream>
#include <list>
#include <stdexcept>
#include <utility>
#include <vector>

template <class KeyType, class ValueType, class Hash = std::hash<KeyType>>
class HashMap {
private:
    static const size_t MAX_SIZE = 10000;
    using my_pair = std::pair<KeyType, ValueType>;

    std::list<my_pair> data[MAX_SIZE];
    typename std::list<my_pair>::iterator endIterator;
    std::vector<size_t> nonempty_buckets;
    size_t pos[MAX_SIZE];
    Hash hasher;
    size_t sz = 0;

    size_t get_hash(const KeyType& x) const {
        return hasher(x) % MAX_SIZE;
    }
    void add_bucket(size_t id) {
        pos[id] = nonempty_buckets.size();
        nonempty_buckets.push_back(id);
    }
    void del_bucket(size_t id) {
        size_t tmp = pos[id];
        std::swap(pos[id], pos[nonempty_buckets.back()]);
        std::swap(nonempty_buckets[tmp], nonempty_buckets.back());
        pos[id] = 0;
        nonempty_buckets.pop_back();
    }

public:
    HashMap(const Hash& _hasher = Hash()) : hasher(_hasher) {}
    template <typename Iter>
    HashMap(Iter first, Iter last, const Hash& _hasher = Hash()) : hasher(_hasher) {
        for (; first != last; ++first) {
            insert(*first);
        }
    }
    HashMap(const std::initializer_list<my_pair>& l, const Hash& _hasher = Hash()) : hasher(_hasher) {
        for (auto it = l.begin(); it != l.end(); ++it) {
            insert(*it);
        }
    }

    const size_t size() const {
        return sz;
    }
    const bool empty() const {
        return sz == 0;
    }
    const Hash hash_function() const {
        return hasher;
    }

    void insert(const my_pair& cur) {
        size_t hsh = get_hash(cur.first);
        for (auto it = data[hsh].begin(); it != data[hsh].end(); ++it) {
            if (it->first == cur.first) {
                return;
            }
        }
        if (data[hsh].empty()) {
            add_bucket(hsh);
        }
        data[hsh].push_front(cur);
        ++sz;
    }
    void erase(const KeyType& key) {
        size_t hsh = get_hash(key);
        bool is_deleted = false;
        for (auto it = data[hsh].begin(); it != data[hsh].end(); ++it) {
            if (it->first == key) {
                data[hsh].erase(it);
                --sz;
                is_deleted = true;
                break;
            }
        }
        if (is_deleted && data[hsh].empty()) {
            del_bucket(hsh);
        }
    }

    class const_iterator;

    class iterator {
    private:
        HashMap * map;
        size_t bucketId;
        typename std::list<my_pair>::iterator posBucket;

        std::pair<const KeyType, ValueType>* get_ptr() {
            std::pair<KeyType, ValueType>* ptr = &(*posBucket);
            return (std::pair<const KeyType, ValueType>*)(void*)ptr;
        }

    public:
        friend class const_iterator;
        iterator() : map(nullptr), bucketId(0), posBucket() {}
        iterator(HashMap* _map, size_t _bucketId, typename std::list<my_pair>::iterator _posBucket) : map(_map) {
            bucketId = _bucketId;
            posBucket = _posBucket;
        }
        iterator& operator++() {
            ++posBucket;
            std::list<my_pair>& tmp = map->data[map->nonempty_buckets[bucketId]];
            if (posBucket == tmp.end()) {
                ++bucketId;
                if (bucketId == map->nonempty_buckets.size()) {
                    posBucket = map->endIterator;
                } else {
                    posBucket = map->data[map->nonempty_buckets[bucketId]].begin();
                }
            }
            return *this;
        }
        iterator operator++(int) {
            iterator tmp = *this;
            ++*this;
            return tmp;
        }
        std::pair<const KeyType, ValueType>& operator*() {
            return *get_ptr();
        }
        std::pair<const KeyType, ValueType>* operator->() {
            return get_ptr();
        }
        bool operator==(const iterator& ot) const {
            return bucketId == ot.bucketId && posBucket == ot.posBucket;
        }
        bool operator!=(const iterator& ot) const {
            return bucketId != ot.bucketId || posBucket != ot.posBucket;
        }
    };

    class const_iterator {
    private:
        HashMap const * map;
        size_t bucketId;
        typename std::list<my_pair>::const_iterator posBucket;

        std::pair<const KeyType, ValueType>* get_ptr() const {
            const std::pair<KeyType, ValueType>* ptr = &(*posBucket);
            return (std::pair<const KeyType, ValueType>*)(void*)ptr;
        }

    public:
        const_iterator() : map(nullptr), bucketId(0), posBucket() {}
        const_iterator(HashMap const * _map, size_t _bucketId, typename std::list<my_pair>::const_iterator _posBucket) : map(_map) {
            bucketId = _bucketId;
            posBucket = _posBucket;
        }
        const_iterator(iterator it) : map(it.map) {
            bucketId = it.bucketId;
            posBucket = it.posBucket;
        }
        const_iterator& operator++() {
            ++posBucket;
            const std::list<my_pair>& tmp = map->data[map->nonempty_buckets[bucketId]];
            if (posBucket == tmp.end()) {
                ++bucketId;
                if (bucketId == map->nonempty_buckets.size()) {
                    posBucket = map->endIterator;
                } else {
                    posBucket = map->data[map->nonempty_buckets[bucketId]].begin();
                }
            }
            return *this;
        }
        const_iterator operator++(int) {
            const_iterator tmp = *this;
            ++*this;
            return tmp;
        }
        const std::pair<const KeyType, ValueType>& operator*() const {
            return *get_ptr();
        }
        const std::pair<const KeyType, ValueType>* operator->() const {
            return get_ptr();
        }
        bool operator==(const const_iterator& ot) const {
            return bucketId == ot.bucketId && posBucket == ot.posBucket;
        }
        bool operator!=(const const_iterator& ot) const {
            return bucketId != ot.bucketId || posBucket != ot.posBucket;
        }
    };

    iterator begin() {
        if (empty()) {
            return iterator(this, 0, endIterator);
        } else {
            return iterator(this, 0, data[nonempty_buckets[0]].begin());
        }
    }
    iterator end() {
        return iterator(this, nonempty_buckets.size(), endIterator);
    }
    const_iterator begin() const {
        if (empty()) {
            return const_iterator(this, 0, endIterator);
        } else {
            return const_iterator(this, 0, data[nonempty_buckets[0]].begin());
        }
    }
    const_iterator end() const {
        return const_iterator(this, nonempty_buckets.size(), endIterator);
    }
    iterator find(const KeyType& key) {
        size_t hsh = get_hash(key);
        for (auto it = data[hsh].begin(); it != data[hsh].end(); ++it) {
            if (it->first == key) {
                return iterator(this, pos[hsh], it);
            }
        }
        return end();
    }
    const_iterator find(const KeyType& key) const {
        size_t hsh = get_hash(key);
        for (auto it = data[hsh].begin(); it != data[hsh].end(); ++it) {
            if (it->first == key) {
                return const_iterator(this, pos[hsh], it);
            }
        }
        return end();
    }
    ValueType& operator[](const KeyType& key) {
        iterator it = find(key);
        if (it == end()) {
            insert({key, ValueType()});
        }
        return find(key)->second;
    }
    const ValueType& at(const KeyType& key) const {
        const_iterator it = find(key);
        if (it == end()) {
            throw std::out_of_range("key is not contained in the hash table");
        } else {
            return it->second;
        }
    }
    void clear() {
        sz = 0;
        for (size_t i = 0; i != nonempty_buckets.size(); ++i) {
            size_t idx = nonempty_buckets[i];
            pos[idx] = 0;
            data[idx].clear();
        }
        nonempty_buckets.clear();
    }
};
