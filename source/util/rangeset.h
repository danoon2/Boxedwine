#include <iterator>
#include <set>
#include <utility>

/**
 * Range set ot type T.
 *
 * A range set is a set comprising zero or more nonempty, disconnected ranges of type T.
 *
 * This class supports adding and removing ranges from the set, and testing if a given object or range is contained in the set.
 *
 * @tparam T type of the contained range end points (anything with an absolute order defined)
 *
 */

// MIT license from https://github.com/hl037/rangeset.hpp/blob/master/rangeset.hpp

template <typename T, typename V>
class RangeSet {
private:
    /** \internal
     *  Representation of an end_point (lower/upper bound) of a range.
     *  RangeSet should alternate lower and upper bounds.
     */
    struct end_point_t {
        T v;        
        enum {
            BEFORE = 0,
            LOWER = 1,
            UPPER = 2,
            AFTER = 3
        } dir;
        V value;
        bool operator<(const end_point_t& oth) const {
            return v == oth.v ? dir < oth.dir : v < oth.v;
        }
        bool operator ==(const end_point_t& oth) const {
            return v == oth.v && dir == oth.dir;
        }
    };

    std::set<end_point_t> data;

public:
    /**
     *  The iterator is bidirectionnal. Its dereferenced value is a V
     */
    struct const_iterator {
        using difference_type = long;
        using value_type = std::pair<T, T>;
        using pointer = const value_type*;
        using reference = const value_type&;
        using iterator_category = std::bidirectional_iterator_tag;

        using _sub = typename std::set<end_point_t>::const_iterator;
        
        value_type val;
        V value;
        _sub lower;
        _sub end;
    protected:
        inline void update() {
            if (lower != end) {
                val = { lower->v, std::next(lower)->v };
                value = lower->value;
            }
        }
    public:
        inline const_iterator() : lower{} {}
        inline const_iterator(const _sub& lower, const _sub& end) : lower{ lower }, end{ end } { update(); }

        inline reference operator*() const { return value; }
        inline pointer operator->() const { return &value; }
        inline const_iterator& operator++() { ++++lower; update(); return *this; }
        inline const_iterator operator++(int) { const_iterator res{ *this }; ++*this; return res; }
        inline const_iterator& operator--() { ----lower; update(); return *this; }
        inline const_iterator operator--(int) { const_iterator res{ *this }; --*this; return res; }

        inline bool operator==(const const_iterator& oth) const { return lower == oth.lower; }
        inline bool operator!=(const const_iterator& oth) const { return !(*this == oth); }
    };

    void insert(T start, T end, V value) {
        if (end < start) {
            return;
        }
        auto&& upper = data.upper_bound({ end, end_point_t::UPPER }); // end) < upper OR upper == end() 
        // At the container begining
        if (upper == data.begin()) {  //    [start , end) < [ upper=begin(), end() )
            data.insert(data.begin(), { end, end_point_t::UPPER, value });
            data.insert(data.begin(), { start, end_point_t::LOWER, value });
            return;
        }

        if (upper == data.end() || upper->dir == end_point_t::LOWER) {  // ')' < end < '['
            if (std::prev(upper)->v != end) { // if not same value, insert, else just skip and take upper's precedent
                data.insert(upper, { end, end_point_t::UPPER, value });
            } else {
                int ii = 0;
            }
            --upper;
        }

        auto&& lower = data.upper_bound({ start, end_point_t::LOWER });//    [start < lower

        if ((lower == upper || lower->dir == end_point_t::LOWER) && (lower == data.begin() || std::prev(lower)->dir == end_point_t::UPPER)) {
            data.insert(lower, { start, end_point_t::LOWER, value });
        } else {
            int ii = 0;
        }

        if (lower != upper) {
            data.erase(lower, upper);
        }
    }

    inline void insert(const std::pair<T, T>& range, V value) {
        insert(range.first, range.second, value);
    }


    void remove(const T& start, const T& end) {        
        auto&& lower = data.lower_bound({ start, end_point_t::LOWER });
        // At the container end
        if (lower == data.end()) {
            return; //nothing to do...
        }

        bool lower_inserted = false;
        if (lower->dir == end_point_t::UPPER) {
            if (lower->v == start) {
                ++lower;
            } else {
                data.insert(lower, { start, end_point_t::UPPER });
                --lower;
                lower_inserted = true;
            }
        }

        auto&& upper = data.lower_bound({ end, end_point_t::LOWER });

        if (upper != data.end() && upper->dir == end_point_t::UPPER) {
            if (upper->v == end) {
                ++upper;
            } else {
                data.insert(upper, { end, end_point_t::LOWER });
                --upper;
            }
        }

        if (lower_inserted) {
            ++lower;
        }
        if (lower != upper) {
            data.erase(lower, upper);
        }
    }

    inline void remove(const std::pair<T, T>& range) {
        remove(range.first, range.second);
    }

    /**
     * Remove unit ranges from the set (could be faster than remove)
     */
    inline void erase(const_iterator it_begin, const_iterator it_end) {
        if (it_begin == cend()) {
            return;
        }
        data.erase(it_begin.lower, it_end.lower);
    }

    inline void erase(const_iterator it) {
        if (it == cend()) {
            return;
        }
        auto it2 = it.lower;
        ++++it2;
        data.erase(it.lower, it2);
    }

    /**
     * Find the unit range that contains a specific value.
     * Returns cend() if not v is not in the set.
     */
    const_iterator find(const T& v) const {
        return find(v, v);
    }

    const_iterator find(const T& start, const T& end) const {
        auto&& upper = data.upper_bound({ start, end_point_t::BEFORE });
        if (upper == data.end()) {
            return cend();
        }
        // start is in the range (if it was less than the LOWER then dir would be LOWER)
        if (upper->dir == end_point_t::UPPER) {
            return const_iterator(--upper, data.end());
        }
        // start is <= the beginning of the found range, so if end is >= start of range then we have a match
        if (end >= upper->v) {
            return const_iterator(upper, data.end());
        }
        return cend();
    }

    inline const_iterator find(const std::pair<T, T>& range) const {
        return find(range.first, range.second);
    }

    /**
     * Return the number of unit range in the set (The number of iterator beetwin cbegin() and cend())
     */
    inline size_t size() const { return data.size() / 2; }

    /**
     * Return an iterator to the first unit range. When dereferencing an iterator, the value is a std::pair<T,T> describing the interval [ res.first, res.end )
     */
    inline const_iterator cbegin() const { return const_iterator{ data.begin(), data.end() }; }
    inline const_iterator begin() { return const_iterator{ data.begin(), data.end() }; }

    /**
     * Return a past-the-end iterator of this set.
     */
    inline const_iterator cend() const { return const_iterator{ data.end(), data.end() }; }
    inline const_iterator end() { return const_iterator{ data.end(), data.end() }; }

public:
    RangeSet() = default;
    ~RangeSet() = default;

};