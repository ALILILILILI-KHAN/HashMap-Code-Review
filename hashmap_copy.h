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
    using myPair = std::pair<KeyType, ValueType>;

    std::list<myPair> data[MAX_SIZE];
    typename std::list<myPair>::iterator endIterator;
    std::vector<size_t> nonemptyBuckets;
    size_t pos[MAX_SIZE];
    Hash hasher;
    size_t sz = 0;

    size_t getHash(const KeyType& x) const {
        return hasher(x) % MAX_SIZE;
    }
    void addBucket(size_t id) {
        pos[id] = nonemptyBuckets.size();
        nonemptyBuckets.push_back(id);
    }
    void delBucket(size_t id) {
        size_t tmp = pos[id];
        std::swap(pos[id], pos[nonemptyBuckets.back()]);
        std::swap(nonemptyBuckets[tmp], nonemptyBuckets.back());
        pos[id] = 0;
        nonemptyBuckets.pop_back();
    }

public:
    HashMap(const Hash& _hasher = Hash()) : hasher(_hasher) {}
    template <typename Iter>
    HashMap(Iter first, Iter last, const Hash& _hasher = Hash()) : hasher(_hasher) {
        for (; first != last; ++first) {
            insert(*first);
        }
    }
    HashMap(const std::initializer_list<myPair>& l, const Hash& _hasher = Hash()) : hasher(_hasher) {
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

    void insert(const myPair& cur) {
        size_t hsh = getHash(cur.first);
        for (auto it = data[hsh].begin(); it != data[hsh].end(); ++it) {
            if (it->first == cur.first) {
                return;
            }
        }
        if (data[hsh].empty()) {
            addBucket(hsh);
        }
        data[hsh].push_front(cur);
        ++sz;
    }
    void erase(const KeyType& key) {
        size_t hsh = getHash(key);
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
            delBucket(hsh);
        }
    }

    class const_iterator;

    class iterator {
    private:
        HashMap * map;
        size_t bucketId;
        typename std::list<myPair>::iterator posBucket;

        std::pair<const KeyType, ValueType>* getPtr() {
            std::pair<KeyType, ValueType>* ptr = &(*posBucket);
            return (std::pair<const KeyType, ValueType>*)(void*)ptr;
        }

    public:
        friend class const_iterator;
        iterator() : map(nullptr), bucketId(0), posBucket() {}
        iterator(HashMap* _map, size_t _bucketId, typename std::list<myPair>::iterator _posBucket) : map(_map) {
            bucketId = _bucketId;
            posBucket = _posBucket;
        }
        iterator& operator++() {
            ++posBucket;
            std::list<myPair>& tmp = map->data[map->nonemptyBuckets[bucketId]];
            if (posBucket == tmp.end()) {
                ++bucketId;
                if (bucketId == map->nonemptyBuckets.size()) {
                    posBucket = map->endIterator;
                } else {
                    posBucket = map->data[map->nonemptyBuckets[bucketId]].begin();
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
            return *getPtr();
        }
        std::pair<const KeyType, ValueType>* operator->() {
            return getPtr();
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
        typename std::list<myPair>::const_iterator posBucket;

        std::pair<const KeyType, ValueType>* getPtr() const {
            const std::pair<KeyType, ValueType>* ptr = &(*posBucket);
            return (std::pair<const KeyType, ValueType>*)(void*)ptr;
        }

    public:
        const_iterator() : map(nullptr), bucketId(0), posBucket() {}
        const_iterator(HashMap const * _map, size_t _bucketId, typename std::list<myPair>::const_iterator _posBucket) : map(_map) {
            bucketId = _bucketId;
            posBucket = _posBucket;
        }
        const_iterator(iterator it) : map(it.map) {
            bucketId = it.bucketId;
            posBucket = it.posBucket;
        }
        const_iterator& operator++() {
            ++posBucket;
            const std::list<myPair>& tmp = map->data[map->nonemptyBuckets[bucketId]];
            if (posBucket == tmp.end()) {
                ++bucketId;
                if (bucketId == map->nonemptyBuckets.size()) {
                    posBucket = map->endIterator;
                } else {
                    posBucket = map->data[map->nonemptyBuckets[bucketId]].begin();
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
            return *getPtr();
        }
        const std::pair<const KeyType, ValueType>* operator->() const {
            return getPtr();
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
            return iterator(this, 0, data[nonemptyBuckets[0]].begin());
        }
    }
    iterator end() {
        return iterator(this, nonemptyBuckets.size(), endIterator);
    }
    const_iterator begin() const {
        if (empty()) {
            return const_iterator(this, 0, endIterator);
        } else {
            return const_iterator(this, 0, data[nonemptyBuckets[0]].begin());
        }
    }
    const_iterator end() const {
        return const_iterator(this, nonemptyBuckets.size(), endIterator);
    }
    iterator find(const KeyType& key) {
        size_t hsh = getHash(key);
        for (auto it = data[hsh].begin(); it != data[hsh].end(); ++it) {
            if (it->first == key) {
                return iterator(this, pos[hsh], it);
            }
        }
        return end();
    }
    const_iterator find(const KeyType& key) const {
        size_t hsh = getHash(key);
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
        for (size_t i = 0; i != nonemptyBuckets.size(); ++i) {
            size_t idx = nonemptyBuckets[i];
            pos[idx] = 0;
            data[idx].clear();
        }
        nonemptyBuckets.clear();
    }
};
