#ifndef AVAKAR_ATOMIC_REF_h
#define AVAKAR_ATOMIC_REF_h

// downloaded from https://github.com/avakar/atomic_ref/

/*
 MIT License

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

#include <atomic>
#include <cstddef>

#if defined(__GNUC__)
#include "atomic_ref.gcc.h"
#else
#error Unsupported platform
#endif

namespace std {

template <typename T, typename = void>
struct _atomic_ref;

template <typename T>
using safe_atomic_ref = _atomic_ref<T>;

template <typename T>
struct atomic_ref
	: _atomic_ref<T>
{
	using difference_type = typename _atomic_ref<T>::difference_type;

	explicit atomic_ref(T & obj)
		: _atomic_ref<T>(obj)
	{
	}

	operator T() const noexcept
	{
		return this->load();
	}

	T operator=(T desired) const noexcept
	{
		this->store(desired);
		return desired;
	}

	T operator++() const noexcept
	{
		return this->fetch_add(1) + T(1);
	}

	T operator++(int) const noexcept
	{
		return this->fetch_add(1);
	}

	T operator--() const noexcept
	{
		return this->fetch_sub(1) - T(1);
	}

	T operator--(int) const noexcept
	{
		return this->fetch_sub(1);
	}

	T operator+=(difference_type arg) const noexcept
	{
		return this->fetch_add(arg) + arg;
	}

	T operator-=(difference_type arg) const noexcept
	{
		return this->fetch_sub(arg) - arg;
	}

	T operator&=(T arg) const noexcept
	{
		return this->fetch_and(arg) & arg;
	}

	T operator|=(T arg) const noexcept
	{
		return this->fetch_or(arg) | arg;
	}

	T operator^=(T arg) const noexcept
	{
		return this->fetch_xor(arg) ^ arg;
	}
};


template <typename T, typename>
struct _atomic_ref
{
	static_assert(std::is_trivially_copyable<T>::value, "T must be TriviallyCopyable");

	static constexpr bool is_always_lock_free = _avakar::atomic_ref::is_always_lock_free<T>::value;
	static constexpr bool is_always_wait_free = _avakar::atomic_ref::is_always_wait_free<T>::value;
	static constexpr std::size_t required_alignment = alignof(T);

	using value_type = T;

	explicit _atomic_ref(value_type & obj)
		: _obj(obj)
	{
	}

	value_type load(std::memory_order order = std::memory_order_seq_cst) const noexcept
	{
		return _avakar::atomic_ref::load(_obj, order);
	}

	void store(value_type desired, std::memory_order order = std::memory_order_seq_cst) noexcept
	{
		_avakar::atomic_ref::store(_obj, desired, order);
	}

	value_type exchange(value_type desired, std::memory_order order = std::memory_order_seq_cst) const noexcept
	{
		return _avakar::atomic_ref::exchange(_obj, desired, order);
	}

	bool compare_exchange_weak(value_type & expected, value_type desired, std::memory_order order = std::memory_order_seq_cst) const noexcept
	{
		return this->compare_exchange_weak(expected, desired, order, order);
	}

	bool compare_exchange_weak(
		value_type & expected, value_type desired,
		std::memory_order success,
		std::memory_order failure) const noexcept
	{
		return _avakar::atomic_ref::compare_exchange_weak(_obj, expected, desired, success, failure);
	}

	bool compare_exchange_strong(value_type & expected, value_type desired, std::memory_order order = std::memory_order_seq_cst) const noexcept
	{
		return this->compare_exchange_strong(expected, desired, order, order);
	}

	bool compare_exchange_strong(
		value_type & expected, value_type desired,
		std::memory_order success,
		std::memory_order failure) const noexcept
	{
		return _avakar::atomic_ref::compare_exchange_strong(_obj, expected, desired, success, failure);
	}

	_atomic_ref & operator=(_atomic_ref const &) = delete;

private:
	value_type & _obj;
};

template <typename T>
struct _atomic_ref<T *>
{
	static_assert(std::is_trivially_copyable<T>::value, "T must be TriviallyCopyable");

	static constexpr bool is_always_lock_free = _avakar::atomic_ref::is_always_lock_free<T>::value;
	static constexpr bool is_always_wait_free = _avakar::atomic_ref::is_always_wait_free<T>::value;
	static constexpr std::size_t required_alignment = alignof(T);

	using value_type = T *;
	using difference_type = std::ptrdiff_t;

	explicit _atomic_ref(value_type & obj)
		: _obj(obj)
	{
	}

	value_type load(std::memory_order order = std::memory_order_seq_cst) const noexcept
	{
		return _avakar::atomic_ref::load(_obj, order);
	}

	void store(value_type desired, std::memory_order order = std::memory_order_seq_cst) noexcept
	{
		_avakar::atomic_ref::store(_obj, desired, order);
	}

	value_type exchange(value_type desired, std::memory_order order = std::memory_order_seq_cst) const noexcept
	{
		return _avakar::atomic_ref::exchange(_obj, desired, order);
	}

	bool compare_exchange_weak(value_type & expected, value_type desired, std::memory_order order = std::memory_order_seq_cst) const noexcept
	{
		return this->compare_exchange_weak(expected, desired, order, order);
	}

	bool compare_exchange_weak(
		value_type & expected, value_type desired,
		std::memory_order success,
		std::memory_order failure) const noexcept
	{
		return _avakar::atomic_ref::compare_exchange_weak(_obj, expected, desired, success, failure);
	}

	bool compare_exchange_strong(value_type & expected, value_type desired, std::memory_order order = std::memory_order_seq_cst) const noexcept
	{
		return this->compare_exchange_strong(expected, desired, order, order);
	}

	bool compare_exchange_strong(
		value_type & expected, value_type desired,
		std::memory_order success,
		std::memory_order failure) const noexcept
	{
		return _avakar::atomic_ref::compare_exchange_strong(_obj, expected, desired, success, failure);
	}

	value_type fetch_add(difference_type arg, std::memory_order order = std::memory_order_seq_cst) const noexcept
	{
		return _avakar::atomic_ref::fetch_add(_obj, arg, order);
	}

	value_type fetch_sub(difference_type arg, std::memory_order order = std::memory_order_seq_cst) const noexcept
	{
		return _avakar::atomic_ref::fetch_sub(_obj, arg, order);
	}

	_atomic_ref & operator=(_atomic_ref const &) = delete;

private:
	value_type & _obj;
};

template <typename T>
struct _atomic_ref<T, std::enable_if_t<std::is_integral<T>::value>>
{
	static_assert(std::is_trivially_copyable<T>::value, "T must be TriviallyCopyable");

	static constexpr bool is_always_lock_free = _avakar::atomic_ref::is_always_lock_free<T>::value;
	static constexpr bool is_always_wait_free = _avakar::atomic_ref::is_always_wait_free<T>::value;
	static constexpr std::size_t required_alignment = alignof(T);

	using value_type = T;
	using difference_type = T;

	explicit _atomic_ref(value_type & obj)
		: _obj(obj)
	{
	}

	value_type load(std::memory_order order = std::memory_order_seq_cst) const noexcept
	{
		return _avakar::atomic_ref::load(_obj, order);
	}

	void store(value_type desired, std::memory_order order = std::memory_order_seq_cst) noexcept
	{
		_avakar::atomic_ref::store(_obj, desired, order);
	}

	value_type exchange(value_type desired, std::memory_order order = std::memory_order_seq_cst) const noexcept
	{
		return _avakar::atomic_ref::exchange(_obj, desired, order);
	}

	bool compare_exchange_weak(value_type & expected, value_type desired, std::memory_order order = std::memory_order_seq_cst) const noexcept
	{
		return this->compare_exchange_weak(expected, desired, order, order);
	}

	bool compare_exchange_weak(
		value_type & expected, value_type desired,
		std::memory_order success,
		std::memory_order failure) const noexcept
	{
		return _avakar::atomic_ref::compare_exchange_weak(_obj, expected, desired, success, failure);
	}

	bool compare_exchange_strong(value_type & expected, value_type desired, std::memory_order order = std::memory_order_seq_cst) const noexcept
	{
		return this->compare_exchange_strong(expected, desired, order, order);
	}

	bool compare_exchange_strong(
		value_type & expected, value_type desired,
		std::memory_order success,
		std::memory_order failure) const noexcept
	{
		return _avakar::atomic_ref::compare_exchange_strong(_obj, expected, desired, success, failure);
	}

	value_type fetch_add(difference_type arg, std::memory_order order = std::memory_order_seq_cst) const noexcept
	{
		return _avakar::atomic_ref::fetch_add(_obj, arg, order);
	}

	value_type fetch_sub(difference_type arg, std::memory_order order = std::memory_order_seq_cst) const noexcept
	{
		return _avakar::atomic_ref::fetch_sub(_obj, arg, order);
	}

	value_type fetch_and(difference_type arg, std::memory_order order = std::memory_order_seq_cst) const noexcept
	{
		return _avakar::atomic_ref::fetch_and(_obj, arg, order);
	}

	value_type fetch_or(difference_type arg, std::memory_order order = std::memory_order_seq_cst) const noexcept
	{
		return _avakar::atomic_ref::fetch_or(_obj, arg, order);
	}

	value_type fetch_xor(difference_type arg, std::memory_order order = std::memory_order_seq_cst) const noexcept
	{
		return _avakar::atomic_ref::fetch_xor(_obj, arg, order);
	}

	_atomic_ref & operator=(_atomic_ref const &) = delete;

private:
	value_type & _obj;
};

}

#endif // _h
