#ifndef LOCKFREE_HASHTABLE_H
#define LOCKFREE_HASHTABLE_H

// from https://github.com/bhhbazinga/LockFreeHashTable

#include <atomic>
#include <cassert>
#include <cmath>

#include "reclaimer.h"

// The maximum bucket size equals to kSegmentSize^kMaxLevel, in this case the
// maximum bucket size is 64^4. If the load factor is 0.5, the maximum number of
// items that Hash Table contains is 64^4 * 0.5 = 2^23. You can adjust the
// following two values according to your memory size.
const int kMaxLevel = 4;
const int kSegmentSize = 64;
const size_t kMaxBucketSize = pow(kSegmentSize, kMaxLevel);

// Hash Table can be stored 2^power_of_2_ * kLoadFactor items.
const float kLoadFactor = 0.5;

template <typename K, typename V>
class TableReclaimer;

template <typename K, typename V, typename Hash = std::hash<K>>
class LockFreeHashTable {
  static_assert(std::is_copy_constructible_v<K>, "K requires copy constructor");
  static_assert(std::is_copy_constructible_v<V>, "V requires copy constructor");
  friend TableReclaimer<K, V>;

  struct Node;
  struct DummyNode;
  struct RegularNode;
  struct Segment;

  typedef size_t HashKey;
  typedef size_t BucketIndex;
  typedef size_t SegmentIndex;
  typedef std::atomic<DummyNode*> Bucket;

 public:
  LockFreeHashTable() : power_of_2_(1), size_(0), hash_func_(Hash()) {
    // Initialize first bucket
    int level = 1;
    Segment* segments = segments_;  // Point to current segment.
    while (level++ <= kMaxLevel - 2) {
      Segment* sub_segments = NewSegments(level);
      segments[0].data.store(sub_segments, std::memory_order_release);
      segments = sub_segments;
    }

    Bucket* buckets = NewBuckets();
    segments[0].data.store(buckets, std::memory_order_release);

    DummyNode* head = new DummyNode(0);
    buckets[0].store(head, std::memory_order_release);
    head_ = head;
  }

  ~LockFreeHashTable() {
    Node* p = head_;
    while (p != nullptr) {
      Node* tmp = p;
      p = p->next.load(std::memory_order_acquire);
      tmp->Release();
    }
  }

  LockFreeHashTable(const LockFreeHashTable& other) = delete;
  LockFreeHashTable(LockFreeHashTable&& other) = delete;
  LockFreeHashTable& operator=(const LockFreeHashTable& other) = delete;
  LockFreeHashTable& operator=(LockFreeHashTable&& other) = delete;

  bool Insert(const K& key, const V& value) {
    RegularNode* new_node = new RegularNode(key, value, hash_func_);
    DummyNode* head = GetBucketHeadByHash(new_node->hash);
    return InsertRegularNode(head, new_node);
  }

  bool Insert(K&& key, const V& value) {
    RegularNode* new_node = new RegularNode(std::move(key), value, hash_func_);
    DummyNode* head = GetBucketHeadByHash(new_node->hash);
    return InsertRegularNode(head, new_node);
  }

  bool Insert(const K& key, V&& value) {
    RegularNode* new_node = new RegularNode(key, std::move(value), hash_func_);
    DummyNode* head = GetBucketHeadByHash(new_node->hash);
    return InsertRegularNode(head, new_node);
  }

  bool Insert(K&& key, V&& value) {
    RegularNode* new_node =
        new RegularNode(std::move(key), std::move(value), hash_func_);
    DummyNode* head = GetBucketHeadByHash(new_node->hash);
    return InsertRegularNode(head, new_node);
  }

  bool Delete(const K& key) {
    HashKey hash = hash_func_(key);
    DummyNode* head = GetBucketHeadByHash(hash);
    RegularNode delete_node(key, hash_func_);
    return DeleteNode(head, &delete_node);
  }

  bool Find(const K& key, V& value) {
    HashKey hash = hash_func_(key);
    DummyNode* head = GetBucketHeadByHash(hash);
    RegularNode find_node(key, hash_func_);
    return FindNode(head, &find_node, value);
  };

  size_t size() const { return size_.load(std::memory_order_relaxed); }

 private:
  size_t bucket_size() const {
    return 1 << power_of_2_.load(std::memory_order_relaxed);
  }

  Segment* NewSegments(int level) {
    Segment* segments = new Segment[kSegmentSize];
    for (int i = 0; i < kSegmentSize; ++i) {
      segments[i].level = level;
      segments[i].data.store(nullptr, std::memory_order_release);
    }
    return segments;
  }

  Bucket* NewBuckets() {
    Bucket* buckets = new Bucket[kSegmentSize];
    for (int i = 0; i < kSegmentSize; ++i) {
      buckets[i].store(nullptr, std::memory_order_release);
    }
    return buckets;
  }

  // Initialize bucket recursively.
  DummyNode* InitializeBucket(BucketIndex bucket_index);

  // When the table size is 2^i , a logical table bucket b contains items whose
  // keys k maintain k mod 2^i = b. When the size becomes 2^i+1, the items of
  // this bucket are split into two buckets: some remain in the bucket b, and
  // others, for which k mod 2^(i+1) == b + 2^i.
  BucketIndex GetBucketParent(BucketIndex bucket_index) const {
    //__builtin_clzl: Get number of leading zero bits.
    // Unset the MSB(most significant bit) of bucket_index;
    return (~(0x8000000000000000 >> (__builtin_clzl(bucket_index))) &
            bucket_index);
  };

  // Get the head node of bucket, if bucket not exist then return nullptr or
  // return head.
  DummyNode* GetBucketHeadByIndex(BucketIndex bucket_index);

  // Get the head node of bucket, if bucket not exist then initialize it and
  // return head.
  DummyNode* GetBucketHeadByHash(HashKey hash) {
    BucketIndex bucket_index = (hash & (bucket_size() - 1));
    DummyNode* head = GetBucketHeadByIndex(bucket_index);
    if (nullptr == head) {
      head = InitializeBucket(bucket_index);
    }
    return head;
  }

  // Harris' OrderedListBasedset with Michael's hazard pointer to manage memory,
  // See also https://github.com/bhhbazinga/LockFreeLinkedList.
  bool InsertRegularNode(DummyNode* head, RegularNode* new_node);
  bool InsertDummyNode(DummyNode* head, DummyNode* new_node,
                       DummyNode** real_head);
  bool DeleteNode(DummyNode* head, Node* delete_node);
  bool FindNode(DummyNode* head, RegularNode* find_node, V& value) {
    Node* prev;
    Node* cur;
    HazardPointer prev_hp, cur_hp;
    bool found = SearchNode(head, find_node, &prev, &cur, prev_hp, cur_hp);
    auto& reclaimer = TableReclaimer<K, V>::GetInstance();
    if (found) {
      V* value_ptr;
      V* temp;
      do {
        // When find and insert concurrently value may be deleted,
        // see InsertRegularNode, so value must be marked as hazard.
        value_ptr = static_cast<RegularNode*>(cur)->value.load(
            std::memory_order_consume);
        temp = value_ptr;
        value_ptr = static_cast<RegularNode*>(cur)->value.load(
            std::memory_order_consume);
      } while (temp != value_ptr);
      reclaimer.ReclaimNoHazardPointer();
      value = *value_ptr;
    }
    return found;
  }

  // Traverse list begin with head until encounter nullptr or the first node
  // which is greater than or equals to the given search_node.
  bool SearchNode(DummyNode* head, Node* search_node, Node** prev_ptr,
                  Node** cur_ptr, HazardPointer& prev_hp,
                  HazardPointer& cur_hp);

  // Compare two nodes according to their reverse_hash and the key.
  bool Less(Node* node1, Node* node2) const {
    if (node1->reverse_hash != node2->reverse_hash) {
      return node1->reverse_hash < node2->reverse_hash;
    }

    if (node1->IsDummy() || node2->IsDummy()) {
      // When initialize bucket concurrently, that could happen.
      return false;
    }

    return static_cast<RegularNode*>(node1)->key <
           static_cast<RegularNode*>(node2)->key;
  }

  bool GreaterOrEquals(Node* node1, Node* node2) const {
    return !(Less(node1, node2));
  }

  bool Equals(Node* node1, Node* node2) const {
    return !Less(node1, node2) && !Less(node2, node1);
  }

  bool is_marked_reference(Node* next) const {
    return (reinterpret_cast<unsigned long>(next) & 0x1) == 0x1;
  }

  Node* get_marked_reference(Node* next) const {
    return reinterpret_cast<Node*>(reinterpret_cast<unsigned long>(next) | 0x1);
  }

  Node* get_unmarked_reference(Node* next) const {
    return reinterpret_cast<Node*>(reinterpret_cast<unsigned long>(next) &
                                   ~0x1);
  }

  static void OnDeleteNode(void* ptr) { delete static_cast<Node*>(ptr); }

  struct Node {
    Node(HashKey hash_, bool dummy)
        : hash(hash_),
          reverse_hash(dummy ? DummyKey(hash) : RegularKey(hash)),
          next(nullptr) {}

    virtual void Release() = 0;

    virtual ~Node() {}

    HashKey Reverse(HashKey hash) const {
      return reverse8bits_[hash & 0xff] << 56 |
             reverse8bits_[(hash >> 8) & 0xff] << 48 |
             reverse8bits_[(hash >> 16) & 0xff] << 40 |
             reverse8bits_[(hash >> 24) & 0xff] << 32 |
             reverse8bits_[(hash >> 32) & 0xff] << 24 |
             reverse8bits_[(hash >> 40) & 0xff] << 16 |
             reverse8bits_[(hash >> 48) & 0xff] << 8 |
             reverse8bits_[(hash >> 56) & 0xff];
    }
    HashKey RegularKey(HashKey hash) const {
      return Reverse(hash | 0x8000000000000000);
    }
    HashKey DummyKey(HashKey hash) const { return Reverse(hash); }

    virtual bool IsDummy() const { return (reverse_hash & 0x1) == 0; }
    Node* get_next() const { return next.load(std::memory_order_acquire); }

    const HashKey hash;
    const HashKey reverse_hash;
    std::atomic<Node*> next;
  };

  // Head node of bucket
  struct DummyNode : Node {
    DummyNode(BucketIndex bucket_index) : Node(bucket_index, true) {}
    ~DummyNode() override {}

    void Release() override { delete this; }

    bool IsDummy() const override { return true; }
  };

  struct RegularNode : Node {
    RegularNode(const K& key_, const V& value_, const Hash& hash_func)
        : Node(hash_func(key_), false), key(key_), value(new V(value_)) {}
    RegularNode(const K& key_, V&& value_, const Hash& hash_func)
        : Node(hash_func(key_), false),
          key(key_),
          value(new V(std::move(value_))) {}
    RegularNode(K&& key_, const V& value_, const Hash& hash_func)
        : Node(hash_func(key_), false),
          key(std::move(key_)),
          value(new V(value_)) {}
    RegularNode(K&& key_, V&& value_, const Hash& hash_func)
        : Node(hash_func(key_), false),
          key(std::move(key_)),
          value(new V(std::move(value_))) {}

    RegularNode(const K& key_, const Hash& hash_func)
        : Node(hash_func(key_), false), key(key_), value(nullptr) {}

    ~RegularNode() override {
      V* ptr = value.load(std::memory_order_consume);
      if (ptr != nullptr)
        delete ptr;  // If update a node, value of this node is nullptr.
    }

    void Release() override { delete this; }

    bool IsDummy() const override { return false; }

    const K key;
    std::atomic<V*> value;
  };

  struct Segment {
    Segment() : level(1), data(nullptr) {}
    explicit Segment(int level_) : level(level_), data(nullptr) {}

    Bucket* get_sub_buckets() const {
      return static_cast<Bucket*>(data.load(std::memory_order_consume));
    }

    Segment* get_sub_segments() const {
      return static_cast<Segment*>(data.load(std::memory_order_consume));
    }

    ~Segment() {
      void* ptr = data.load(std::memory_order_consume);
      if (nullptr == ptr) return;

      if (level == kMaxLevel - 1) {
        Bucket* buckets = static_cast<Bucket*>(ptr);
        delete[] buckets;
      } else {
        Segment* sub_segments = static_cast<Segment*>(ptr);
        delete[] sub_segments;
      }
    }

    int level;                // Level of segment.
    std::atomic<void*> data;  // If level == kMaxLevel then data point to
                              // buckets else data point to segments.
  };

  std::atomic<size_t> power_of_2_;   // Bucket size == 2^power_of_2_.
  std::atomic<size_t> size_;         // Item size.
  Hash hash_func_;                   // Hash function.
  Segment segments_[kSegmentSize];   // Top level sengments.
  static size_t reverse8bits_[256];  // Lookup table for reverse bits quickly.
  DummyNode* head_;                  // Head of linkedlist.
  static Reclaimer::HazardPointerList global_hp_list_;
};

template <typename K, typename V, typename Hash>
Reclaimer::HazardPointerList LockFreeHashTable<K, V, Hash>::global_hp_list_;

template <typename K, typename V>
class TableReclaimer : public Reclaimer {
  friend LockFreeHashTable<K, V>;

 private:
  TableReclaimer(HazardPointerList& hp_list) : Reclaimer(hp_list) {}
  ~TableReclaimer() override = default;

  static TableReclaimer<K, V>& GetInstance() {
    thread_local static TableReclaimer reclaimer(
        LockFreeHashTable<K, V>::global_hp_list_);
    return reclaimer;
  }
};

// Fast reverse bits using Lookup Table.
#define R2(n) n, n + 2 * 64, n + 1 * 64, n + 3 * 64
#define R4(n) R2(n), R2(n + 2 * 16), R2(n + 1 * 16), R2(n + 3 * 16)
#define R6(n) R4(n), R4(n + 2 * 4), R4(n + 1 * 4), R4(n + 3 * 4)
// Lookup Table that store the reverse of each 8bit number.
template <typename K, typename V, typename Hash>
size_t LockFreeHashTable<K, V, Hash>::reverse8bits_[256] = {R6(0), R6(2), R6(1),
                                                            R6(3)};

template <typename K, typename V, typename Hash>
typename LockFreeHashTable<K, V, Hash>::DummyNode*
LockFreeHashTable<K, V, Hash>::InitializeBucket(BucketIndex bucket_index) {
  BucketIndex parent_index = GetBucketParent(bucket_index);
  DummyNode* parent_head = GetBucketHeadByIndex(parent_index);
  if (nullptr == parent_head) {
    parent_head = InitializeBucket(parent_index);
  }

  int level = 1;
  Segment* segments = segments_;  // Point to current segment.
  while (level++ <= kMaxLevel - 2) {
    Segment& cur_segment =
        segments[(bucket_index / static_cast<SegmentIndex>(pow(
                                     kSegmentSize, kMaxLevel - level + 1))) %
                 kSegmentSize];
    Segment* sub_segments = cur_segment.get_sub_segments();
    if (nullptr == sub_segments) {
      // Try allocate segments.
      sub_segments = NewSegments(level);
      void* expected = nullptr;
      if (!cur_segment.data.compare_exchange_strong(
              expected, sub_segments, std::memory_order_release)) {
        delete[] sub_segments;
        sub_segments = static_cast<Segment*>(expected);
      }
    }
    segments = sub_segments;
  }

  Segment& cur_segment = segments[(bucket_index / kSegmentSize) % kSegmentSize];
  Bucket* buckets = cur_segment.get_sub_buckets();
  if (nullptr == buckets) {
    // Try allocate buckets.
    void* expected = nullptr;
    buckets = NewBuckets();
    if (!cur_segment.data.compare_exchange_strong(expected, buckets,
                                                  std::memory_order_release)) {
      delete[] buckets;
      buckets = static_cast<Bucket*>(expected);
    }
  }

  Bucket& bucket = buckets[bucket_index % kSegmentSize];
  DummyNode* head = bucket.load(std::memory_order_consume);
  if (nullptr == head) {
    // Try allocate dummy head.
    head = new DummyNode(bucket_index);
    DummyNode* real_head;  // If insert failed, real_head is the head of bucket.
    if (InsertDummyNode(parent_head, head, &real_head)) {
      // Dummy head must be inserted into the list before storing into bucket.
      bucket.store(head, std::memory_order_release);
    } else {
      delete head;
      head = real_head;
    }
  }
  return head;
}

template <typename K, typename V, typename Hash>
typename LockFreeHashTable<K, V, Hash>::DummyNode*
LockFreeHashTable<K, V, Hash>::GetBucketHeadByIndex(BucketIndex bucket_index) {
  int level = 1;
  const Segment* segments = segments_;
  while (level++ <= kMaxLevel - 2) {
    segments =
        segments[(bucket_index / static_cast<SegmentIndex>(pow(
                                     kSegmentSize, kMaxLevel - level + 1))) %
                 kSegmentSize]
            .get_sub_segments();
    if (nullptr == segments) return nullptr;
  }

  Bucket* buckets =
      segments[(bucket_index / kSegmentSize) % kSegmentSize].get_sub_buckets();
  if (nullptr == buckets) return nullptr;

  Bucket& bucket = buckets[bucket_index % kSegmentSize];
  return bucket.load(std::memory_order_consume);
}

template <typename K, typename V, typename Hash>
bool LockFreeHashTable<K, V, Hash>::InsertDummyNode(DummyNode* parent_head,
                                                    DummyNode* new_head,
                                                    DummyNode** real_head) {
  Node* prev;
  Node* cur;
  HazardPointer prev_hp, cur_hp;
  do {
    if (SearchNode(parent_head, new_head, &prev, &cur, prev_hp, cur_hp)) {
      // The head of bucket already insert into list.
      *real_head = static_cast<DummyNode*>(cur);
      return false;
    }
    new_head->next.store(cur, std::memory_order_release);
  } while (!prev->next.compare_exchange_weak(
      cur, new_head, std::memory_order_release, std::memory_order_relaxed));
  return true;
}

// Insert regular node into hash table, if its key is already exists in
// hash table then update it and return false else return true.
template <typename K, typename V, typename Hash>
bool LockFreeHashTable<K, V, Hash>::InsertRegularNode(DummyNode* head,
                                                      RegularNode* new_node) {
  Node* prev;
  Node* cur;
  HazardPointer prev_hp, cur_hp;
  auto& reclaimer = TableReclaimer<K, V>::GetInstance();
  do {
    if (SearchNode(head, new_node, &prev, &cur, prev_hp, cur_hp)) {
      V* new_value = new_node->value.load(std::memory_order_consume);
      V* old_value = static_cast<RegularNode*>(cur)->value.exchange(
          new_value, std::memory_order_release);
      reclaimer.ReclaimLater(old_value,
                             [](void* ptr) { delete static_cast<V*>(ptr); });
      new_node->value.store(nullptr, std::memory_order_release);
      delete new_node;
      return false;
    }
    new_node->next.store(cur, std::memory_order_release);
  } while (!prev->next.compare_exchange_weak(
      cur, new_node, std::memory_order_release, std::memory_order_relaxed));

  size_t size = size_.fetch_add(1, std::memory_order_relaxed) + 1;
  size_t power = power_of_2_.load(std::memory_order_relaxed);
  if ((1 << power) * kLoadFactor < size) {
    if (power_of_2_.compare_exchange_strong(power, power + 1,
                                            std::memory_order_release)) {
      assert(bucket_size() <=
             kMaxBucketSize);  // Out of memory or you can change the kMaxLevel
                               // and kSegmentSize.
    }
  }
  return true;
}

template <typename K, typename V, typename Hash>
bool LockFreeHashTable<K, V, Hash>::SearchNode(DummyNode* head,
                                               Node* search_node,
                                               Node** prev_ptr, Node** cur_ptr,
                                               HazardPointer& prev_hp,
                                               HazardPointer& cur_hp) {
  auto& reclaimer = TableReclaimer<K, V>::GetInstance();
try_again:
  Node* prev = head;
  Node* cur = prev->get_next();
  Node* next;
  while (true) {
    cur_hp.UnMark();
    cur_hp = HazardPointer(&reclaimer, cur);
    // Make sure prev is the predecessor of cur,
    // so that cur is properly marked as hazard.
    if (prev->get_next() != cur) goto try_again;

    if (nullptr == cur) {
      *prev_ptr = prev;
      *cur_ptr = cur;
      return false;
    }

    next = cur->get_next();
    if (is_marked_reference(next)) {
      if (!prev->next.compare_exchange_strong(cur,
                                              get_unmarked_reference(next)))
        goto try_again;

      reclaimer.ReclaimLater(cur, LockFreeHashTable<K, V, Hash>::OnDeleteNode);
      reclaimer.ReclaimNoHazardPointer();
      size_.fetch_sub(1, std::memory_order_relaxed);
      cur = get_unmarked_reference(next);
    } else {
      if (prev->get_next() != cur) goto try_again;

      // Can not get copy_cur after above invocation,
      // because prev may not be the predecessor of cur at this point.
      if (GreaterOrEquals(cur, search_node)) {
        *prev_ptr = prev;
        *cur_ptr = cur;
        return Equals(cur, search_node);
      }

      // Swap cur_hp and prev_hp.
      HazardPointer tmp = std::move(cur_hp);
      cur_hp = std::move(prev_hp);
      prev_hp = std::move(tmp);

      prev = cur;
      cur = next;
    }
  };

  assert(false);
  return false;
}

template <typename K, typename V, typename Hash>
bool LockFreeHashTable<K, V, Hash>::DeleteNode(DummyNode* head,
                                               Node* delete_node) {
  Node* prev;
  Node* cur;
  Node* next;
  HazardPointer prev_hp, cur_hp;
  do {
    do {
      if (!SearchNode(head, delete_node, &prev, &cur, prev_hp, cur_hp)) {
        return false;
      }
      next = cur->get_next();
    } while (is_marked_reference(next));
    // Logically delete cur by marking cur->next.
  } while (!cur->next.compare_exchange_weak(next, get_marked_reference(next),
                                            std::memory_order_release,
                                            std::memory_order_relaxed));

  if (prev->next.compare_exchange_strong(cur, next,
                                         std::memory_order_release)) {
    size_.fetch_sub(1, std::memory_order_relaxed);
    auto& reclaimer = TableReclaimer<K, V>::GetInstance();
    reclaimer.ReclaimLater(cur, LockFreeHashTable<K, V, Hash>::OnDeleteNode);
    reclaimer.ReclaimNoHazardPointer();
  } else {
    prev_hp.UnMark();
    cur_hp.UnMark();
    SearchNode(head, delete_node, &prev, &cur, prev_hp, cur_hp);
  }

  return true;
}
#endif  // LOCKFREE_HASHTABLE_H
