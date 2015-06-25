// alloc.hpp - part of krystal
// (c) 2013-4 by Arthur Langereis (@zenmumbler)

#ifndef KRYSTAL_ALLOC_H
#define KRYSTAL_ALLOC_H

#include <cstddef>
#include <memory>

namespace krystal {


class Lake {
	const size_t blockSize_;
	mutable std::vector<std::unique_ptr<uint8_t[]>> blocks_;
	mutable uint8_t *arena_, *pos_;

	static constexpr size_t DefaultBlockSize = 48 * 1024;

	void addBlock(size_t sizeInBytes) const {
		blocks_.emplace_back(new uint8_t[sizeInBytes]);
		pos_ = arena_ = blocks_.back().get();
	}
	
	inline void addBlock() const { addBlock(blockSize_); }

public:
	Lake(const size_t blockSize)
	: blockSize_ {blockSize}
	{
		addBlock();
	}
	Lake() : Lake(DefaultBlockSize) {}
	
	void* allocate(size_t n) const {
		if (pos_ + n - arena_ > blockSize_) {
			if (n > blockSize_)
				addBlock(n + blockSize_); // single-use large block
			else
				addBlock();
		}

		auto result = pos_;
		pos_ += (n + 3) & ~3;
		return result;
	}
	
	void deallocate(void* p, size_t n) const {
	}
};


template <typename T, typename Alloc>
class AllocAdapter {
	const Alloc* allocator_;

public:
	template <typename U, typename A>
	friend class AllocAdapter;

	using value_type = T;
	using size_type = size_t;
	using pointer = T*;
	
	using propagate_on_container_move_assignment = std::true_type;
	
	template <typename U>
	struct rebind { typedef AllocAdapter<U, Alloc> other; };
	
	AllocAdapter(const Alloc* alloc) noexcept : allocator_{ alloc } {}

	template <class U, class UAlloc>
	AllocAdapter(const AllocAdapter<U, UAlloc>& rhs) noexcept
	: allocator_{rhs.allocator_}
	{}

	pointer allocate(size_type n) {
		return static_cast<pointer>(allocator_->allocate(sizeof(T) * n));
	}

	void deallocate(pointer p, size_type n) {
		allocator_->deallocate(p, sizeof(T) * n);
	}
};

template <typename T, typename Alloc, typename U>
inline bool operator==(const AllocAdapter<T, Alloc>&, const AllocAdapter<U, Alloc>) noexcept { return true; }

template <typename T, typename Alloc, typename U>
inline bool operator!=(const AllocAdapter<T, Alloc>&, const AllocAdapter<U, Alloc>) noexcept { return false; }

// -- standard usages
template <typename T>
using LakeAllocator = AllocAdapter<T, Lake>;


} // ns krystal

#endif
