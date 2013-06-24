// alloc.hpp - part of krystal
// (c) 2013 by Arthur Langereie

#ifndef __krystal_alloc__
#define __krystal_alloc__

#include <memory>
#include <scoped_allocator>

namespace krystal {

class lake {
	size_t block_size_;
	uint8_t *arena_, *pos_;
public:
	lake(const size_t block_size) {
		block_size_ = block_size;
		arena_ = new uint8_t[block_size_];
		pos_ = arena_;
	}
	
	~lake() {
		delete [] arena_;
	}
	
	void* allocate(size_t n) {
		auto result = pos_;
		pos_ += n;
		return result;
	}
	
	void deallocate(void* p, size_t n) {
	}
};


template <typename T, typename Alloc>
class krystal_alloc {
	Alloc* allocator_;
public:
	using value_type = T;
	using size_type = size_t;
	using difference_type = ptrdiff_t;
	using pointer = T*;
	using const_pointer = const T*;
	using reference = T&;
	using const_reference = const T&;
	
	template <typename U>
	struct rebind { typedef krystal_alloc<U, Alloc> other; };
	
	krystal_alloc() : allocator_{ new Alloc() } {}
	krystal_alloc(Alloc& alloc) : allocator_{ alloc } {}

	pointer address(reference v) {
		return 0;
	}

	const_pointer address(const_reference v) {
		return 0;
	}

	size_type max_size() const {
		return static_cast<size_type>(-1) / sizeof(value_type);
	}

	pointer allocate(size_type n) {
		return static_cast<pointer>(allocator_->allocate(sizeof(T) * n));
	}

	void deallocate(pointer p, size_type n) {
		allocator_->deallocate(p, n);
	}

	void construct(pointer p, const_reference v) {
		new (p) T{v};
	}

	void construct(pointer p) {
		new (p) T{};
	}

	void destroy(pointer p) {
		p->~T();
	}
};

template <typename T, typename Alloc, typename U>
inline bool operator==(const krystal_alloc<T, Alloc>&, const krystal_alloc<U, Alloc>) { return true; }

template <typename T, typename Alloc, typename U>
inline bool operator!=(const krystal_alloc<T, Alloc>&, const krystal_alloc<U, Alloc>) { return false; }

}

#endif
