#ifndef __VECTOR_UTILS_H__
#define __VECTOR_UTILS_H__


template <class vType>
int vectorIndexOf(const std::vector<vType>& v, const vType& o) {
	auto itr = std::find(v.begin(), v.end(), o);

	if (itr != v.cend()) {
		return (int)std::distance(v.begin(), itr);
	}
	return -1;
}

#endif