#ifndef AVAKAR_ATOMIC_REF_ATOMIC_REF_GCC_h
#define AVAKAR_ATOMIC_REF_ATOMIC_REF_GCC_h

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
#include <type_traits>

namespace _avakar {
namespace atomic_ref {

template <typename T>
struct is_always_lock_free
	: std::integral_constant<bool, __atomic_always_lock_free(sizeof(T), 0)>
{
};

template <typename T>
struct is_always_wait_free
	: std::false_type
{
};

template <typename T>
std::enable_if_t<std::is_enum<T>::value, T> load(T const & obj, std::memory_order order) noexcept
{
	return (T)__atomic_load_n((std::underlying_type_t<T> const *)&obj, order);
}

template <typename T>
std::enable_if_t<!std::is_enum<T>::value, T> load(T const & obj, std::memory_order order) noexcept
{
	return __atomic_load_n(&obj, order);
}

template <typename T>
void store(T & obj, T desired, std::memory_order order) noexcept
{
	__atomic_store(&obj, &desired, order);
}

template <typename T>
T exchange(T & obj, T desired, std::memory_order order) noexcept
{
	return __atomic_exchange_n(&obj, desired, order);
}

template <typename T>
bool compare_exchange_weak(T & obj, T & expected, T desired, std::memory_order success, std::memory_order failure) noexcept
{
	return __atomic_compare_exchange(&obj, &expected, &desired, true, (int)success, (int)failure);
}

template <typename T>
bool compare_exchange_strong(T & obj, T & expected, T desired, std::memory_order success, std::memory_order failure) noexcept
{
	return __atomic_compare_exchange(&obj, &expected, &desired, false, (int)success, (int)failure);
}

template <typename T>
T fetch_add(T & obj, T arg, std::memory_order order) noexcept
{
	return __atomic_fetch_add(&obj, arg, order);
}

template <typename T>
T fetch_sub(T & obj, T arg, std::memory_order order) noexcept
{
	return __atomic_fetch_sub(&obj, arg, order);
}

template <typename T>
T fetch_and(T & obj, T arg, std::memory_order order) noexcept
{
	return __atomic_fetch_and(&obj, arg, order);
}

template <typename T>
T fetch_or(T & obj, T arg, std::memory_order order) noexcept
{
	return __atomic_fetch_or(&obj, arg, order);
}

template <typename T>
T fetch_xor(T & obj, T arg, std::memory_order order) noexcept
{
	return __atomic_fetch_xor(&obj, arg, order);
}

template <typename T>
auto fetch_add(T * & obj, std::ptrdiff_t arg, std::memory_order order) noexcept
{
	return __atomic_fetch_add(&obj, arg * sizeof(T), order);
}

template <typename T>
auto fetch_sub(T * & obj, std::ptrdiff_t arg, std::memory_order order) noexcept
{
	return __atomic_fetch_sub(&obj, arg * sizeof(T), order);
}

}
}

#endif // _h
