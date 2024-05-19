//
// hash_table.hh
//
// Created by Arpad Goretity (H2CO3)
// on 02/06/2015
//
// Licensed under the 2-clause BSD License
//

// taken from https://github.com/H2CO3/hash_table/blob/master/hash_table.hh
// 
// I removed [].  This is just a personal preference, I believe the fact that the c++ map which returns 
// a reference, thus is required to create an empty version if it doesn't exist seems wrong.  This version
// returns a copy using get.
//
// I intend to use this for only pointers types and BString for the values
#ifndef H2CO3_HASH_TABLE_HH
#define H2CO3_HASH_TABLE_HH

#include <vector>
#include <functional>
#include <type_traits>
#include <cstdlib>
#include <cassert>
#include <cstdio>
#include <concepts>

namespace concepts
{
	//concept to find out of if a type is a pointer
	template<typename T>
	concept is_pointer = std::is_pointer_v<T>;

	//find out if the type overloads the arrow operator
	template<class T>
	concept has_arrow_operator = requires (T t)
	{
		{ t.operator ->() } -> is_pointer;
	};
}

template<typename Key, typename Value, typename Hash = std::hash<Key>, typename Equal = std::equal_to<Key>>
struct BHashTable {
public:

	// "pointed-to" type of iterators; a Key + Value pair
	struct KeyValue {
		Key key;     // must not be modified by the user! (const omitted because reasons)
		Value value;
	};

	// stlib-traits-friendly typedefs
	using key_type = Key;
	using mapped_type = Value;
	using value_type = KeyValue;
	using size_type = std::size_t;
	using difference_type = std::ptrdiff_t;
	using hasher = Hash;
	using key_equal = Equal;
	using reference = value_type&;
	using const_reference = const value_type&;
	using pointer = value_type*;
	using const_pointer = const value_type*;

private:

	struct Slot {
		KeyValue kv;
		bool used;

		Slot() noexcept : kv{}, used{ false } {}

		Slot(Key key, Value value) :
			kv{ std::move(key), std::move(value) },
			used{ true }
		{}

		Slot(const Slot& other) = default;

		Slot(Slot&& other) noexcept :
			kv{ std::move(other.kv) },
			used{ other.used }
		{
			other.used = false;
		}

		friend void swap(Slot& lhs, Slot& rhs) noexcept {
			using std::swap;
			swap(lhs.kv, rhs.kv);
			swap(lhs.used, rhs.used);
		}

		Slot& operator=(Slot other) {
			swap(*this, other);
			return *this;
		}

		// intentionally not operator==
		bool equals(const Key& key) const {
			assert(used);
			return Equal{}(kv.key, key);
		}
	};

	std::vector<Slot> slots;
	std::size_t count;
	std::size_t max_hash_offset;

	// the sole purpose of this function is that we can
	// explicitly call const member functions on 'this'.
	auto cthis() const { return this; }

	std::size_t key_index(const Key& key) const {
		return Hash{}(key)&mask();
	}

	std::size_t mask() const {
		assert(
			slots.size() && !(slots.size() & (slots.size() - 1)) &&
			"table size must be a power of two"
		);

		return slots.size() - 1;
	}

	bool should_rehash() const {
		// keep load factor below 0.75
		// this ratio is chosen carefully so that it can be optimized well:
		// it is equivalent with ((size << 1) + size) >> 2.
		return slots.empty() || count >= slots.size() * 3 / 4;
	}

	const Slot* get_slot(const Key& key) const {
		// do not try to modulo by 0. An empty table has no values.
		if (slots.empty()) {
			return nullptr;
		}

		std::size_t i = key_index(key);
		std::size_t hash_offset = 0;

		// linear probing using a cached maximal probe sequence length.
		// This avoids the need to mark deleted slots as special and
		// fixes the performance problem whereby searching for a key after
		// having performed lots of deletions results in O(n) running time.
		// (max_hash_offset is one less than the length of the longest sequence.)
		do {
			if (slots[i].used && slots[i].equals(key)) {
				return &slots[i];
			}

			i = (i + 1) & mask();
			hash_offset++;
		} while (hash_offset <= max_hash_offset);

		return nullptr;
	}

	Slot* get_slot(const Key& key) {
		return const_cast<Slot*>(cthis()->get_slot(key));
	}

	KeyValue* insert_nonexistent_norehash(Key key, Value value) {
		assert(should_rehash() == false);
		assert(size() < slots.size()); // requires empty slots
		assert(cthis()->get_slot(key) == nullptr);

		std::size_t i = key_index(key);
		std::size_t hash_offset = 0;

		// first, find an empty (unused) slot
		while (slots[i].used) {
			i = (i + 1) & mask();
			hash_offset++;
		}

		// then, perform the actual insertion.
		// this also marks the slot as used.
		slots[i] = { std::move(key), std::move(value) };
		assert(slots[i].used);

		// unconditionally increment the size because
		// we know that the key didn't exist before.
		count++;

		// finally, update maximal length of probe sequences (minus one)
		if (hash_offset > max_hash_offset) {
			max_hash_offset = hash_offset;
		}

		return &slots[i].kv;
	}

	void rehash() {
		// compute new size. Must be a power of two.
		const std::size_t new_size = slots.empty() ? 8 : slots.size() * 2;

		// move original slot array out of *this and reset internal state
		auto old_slots = std::move(slots);

		// language lawyer: move() need not clear std::vector.
		// this->clear() takes care of that, however
		// (as well as zeroing out count and max_hash_offset.)
		clear();

		// make room for new slots (need to default-construct
		// in order for them to be in an 'unused'/free state)
		slots.resize(new_size);

		// re-insert each key-value pair
		for (auto& slot : old_slots) {
			if (slot.used) {
				insert_nonexistent_norehash(std::move(slot.kv.key), std::move(slot.kv.value));
			}
		}
	}

public:

	//////////////////
	// Constructors //
	//////////////////
	BHashTable() noexcept : slots{}, count{ 0 }, max_hash_offset{ 0 } {}

	BHashTable(std::size_t capacity) noexcept : BHashTable() {
		// Make sure the real capacity is a power of two >= 8.
		// We should also keep in mind that the number of elements
		// is at most 3/4 of the number of slots!
		std::size_t min_num_slots = (capacity * 4 + 2) / 3; // round up
		std::size_t real_cap = 8;

		while (real_cap < min_num_slots) {
			real_cap *= 2;
		}

		slots.resize(real_cap);
	}

	BHashTable(const BHashTable&) = default;

	BHashTable(BHashTable&& other) noexcept :
		slots{ std::move(other.slots) },
		count{ other.count },
		max_hash_offset{ other.max_hash_offset }
	{
		other.clear();
	}

	// naive implementation, may be improved. not sure if worth the effort.
	BHashTable(std::initializer_list<KeyValue> elems) : BHashTable(elems.size()) {
		for (auto& elem : elems) {
			// cannot move from an initializer_list
			set(elem.key, elem.value);
		}
	}

	/////////////////////////
	// Resource management //
	/////////////////////////

	friend void swap(BHashTable& lhs, BHashTable& rhs) noexcept {
		using std::swap;
		swap(lhs.slots, rhs.slots);
		swap(lhs.count, rhs.count);
		swap(lhs.max_hash_offset, rhs.max_hash_offset);
	}

	BHashTable& operator=(BHashTable other) {
		swap(*this, other);
		return *this;
	}

	void clear() noexcept {
		slots.clear();
		count = 0;
		max_hash_offset = 0;
	}

	///////////////////////////////////////////////////////////////
	// Actual hash table operations: Get, Insert/Replace, Delete //
	///////////////////////////////////////////////////////////////

	bool contains(const Key& key) const {
		return get_slot(key) != nullptr;
	}
		
	Value get(const Key& key) const requires concepts::has_arrow_operator<Value> || std::is_pointer_v<Value> {
		if (const Slot* slot = get_slot(key)) {
			return slot->kv.value;
		}
		return nullptr;
	}

	bool get(const Key& key, Value& value) const {
		if (const Slot* slot = get_slot(key)) {
			value = slot->kv.value;
			return true;
		}
		return false;
	}

	Value set(const Key& key, const Value& value) requires (concepts::has_arrow_operator<Value> || std::is_pointer_v<Value>) {
		// if the key is already in the table, just replace it and move on
		if (Slot* slot = get_slot(key)) {
			Value oldValue = slot->kv.value;
			slot->kv.value = value;
			return oldValue;
		}

		// else we need to insert it. First, check if we need to expand.
		if (should_rehash()) {
			rehash();
		}

		// then we actually insert the key.
		insert_nonexistent_norehash(key, std::move(value));
		return nullptr;
	}

	bool set(const Key& key, const Value& value) requires (!concepts::has_arrow_operator<Value> && !std::is_pointer_v<Value>) {
		// if the key is already in the table, just replace it and move on
		if (Slot* slot = get_slot(key)) {
			slot->kv.value = value;
			return true;
		}

		// else we need to insert it. First, check if we need to expand.
		if (should_rehash()) {
			rehash();
		}

		// then we actually insert the key.
		insert_nonexistent_norehash(key, std::move(value));
		return false;
	}

	void remove(const Key& key) {
		if (Slot* slot = get_slot(key)) {
			// destroy key and value (we don't want to surprise users of RAII)
			// This also marks the slot as unused.
			*slot = {};
			assert(slot->used == false);

			// removing an existing key means we need to decrease the table size.
			count--;
		}
	}

	std::size_t size() const {
		return count;
	}

	bool isEmpty() const {
		return size() == 0;
	}

	const Value operator[](const Key& key) const requires concepts::has_arrow_operator<Value> || std::is_pointer_v<Value> {
		if (const Slot* slot = get_slot(key)) {
			return slot->kv.value;
		}
		return nullptr;
	}

	//////////////////
	// Iterator API //
	//////////////////

	struct const_iterator {
	protected:
		friend struct BHashTable;

		const BHashTable* owner;
		std::size_t slot_index;

		const_iterator(const BHashTable* p_owner, std::size_t p_slot_index) :
			owner(p_owner),
			slot_index(p_slot_index)
		{}

	public:

		const_iterator(const const_iterator& other) = default;

		const KeyValue* operator->() const {
			assert(slot_index < owner->slots.size() && "cannot dereference end iterator");
			return &owner->slots[slot_index].kv;
		}

		const KeyValue& operator*() const {
			return *operator->();
		}

		const_iterator& operator++() {
			assert(slot_index < owner->slots.size() && "cannot increment end iterator");
			do {
				slot_index++;
			} while (slot_index < owner->slots.size() && not owner->slots[slot_index].used);
			return *this;
		}

		const_iterator operator++(int) {
			auto prev(*this);
			++*this;
			return prev;
		}

		bool operator==(const const_iterator& other) const {
			return owner == other.owner && slot_index == other.slot_index;
		}

		bool operator!=(const const_iterator& other) const {
			return !operator==(other);
		}
	};

	struct iterator : public const_iterator {
	private:
		friend struct BHashTable;

		iterator(const const_iterator& other) : const_iterator(other) {}

	public:
		iterator(const iterator& other) : const_iterator(other) {}

		KeyValue& operator*() const {
			return *operator->();
		}

		KeyValue* operator->() const {
			return const_cast<KeyValue*>(
				static_cast<const const_iterator*>(this)->operator->()
				);
		}

		iterator& operator++() {
			assert(this->slot_index < this->owner->slots.size() && "cannot increment end iterator");
			do {
				this->slot_index++;
			} while (this->slot_index < this->owner->slots.size() && not this->owner->slots[this->slot_index].used);
			return *this;
		}

		iterator operator++(int) {
			auto prev(*this);
			++*this;
			return prev;
		}
	};

	const_iterator begin() const {
		auto it = const_iterator(this, 0);
		while (it.slot_index < slots.size() && not slots[it.slot_index].used) {
			it.slot_index++;
		}
		return it;
	}

	const_iterator end() const {
		return const_iterator(this, slots.size());
	}

	iterator begin() {
		return iterator(cthis()->begin());
	}

	iterator end() {
		return iterator(cthis()->end());
	}

	const_iterator cbegin() const {
		return begin();
	}

	const_iterator cend() const {
		return end();
	}

	const_iterator find(const Key& key) const {
		if (const Slot* slot = get_slot(key)) {
			return const_iterator(this, slot - slots.data());
		}
		return end();
	}

	iterator find(const Key& key) {
		return iterator(cthis()->find(key));
	}

	void erase(const const_iterator& it) {
		assert(it.owner == this && "cannot erase an element of another instance");
		remove(it->key);
	}
};
#endif 