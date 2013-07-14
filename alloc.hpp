// alloc.hpp - part of krystal
// (c) 2013 by Arthur Langereie

#ifndef __krystal_alloc__
#define __krystal_alloc__

#include <cstddef>
#include <memory>

namespace krystal {

	class lake {
		const size_t block_size_;
		mutable std::vector<std::unique_ptr<uint8_t[]>> blocks_;
		mutable uint8_t *arena_, *pos_;

		static constexpr const size_t DefaultBlockSize = 48 * 1024;

		void add_block(size_t of_size) const {
			blocks_.emplace_back(new uint8_t[of_size]);
			pos_ = arena_ = blocks_.back().get();
		}
		
		inline void add_block() const { add_block(block_size_); }

	public:
		lake(const size_t block_size)
		: block_size_ {block_size}
		{
			add_block();
		}
		lake() : lake(DefaultBlockSize) {}
		
		void* allocate(size_t n) const {
			if (pos_ + n - arena_ > block_size_) {
				if (n > block_size_)
					add_block(n); // single-use large block
				else
					add_block();
			}

			auto result = pos_;
			pos_ += n;
			return result;
		}
		
		void deallocate(void* p, size_t n) const {
		}
	};


	template <typename T, typename Alloc>
	class krystal_alloc {
		const Alloc* allocator_;

	public:
		template <typename U, typename A>
		friend class krystal_alloc;
	
		using value_type = T;
		using size_type = size_t;
		using pointer = T*;
		
		using propagate_on_container_move_assignment = std::true_type;
		
		template <typename U>
		struct rebind { typedef krystal_alloc<U, Alloc> other; };
		
		krystal_alloc(const Alloc* alloc) noexcept : allocator_{ alloc } {}

		template <class U, class UAlloc>
		krystal_alloc(const krystal_alloc<U, UAlloc>& rhs) noexcept
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
	inline bool operator==(const krystal_alloc<T, Alloc>&, const krystal_alloc<U, Alloc>) noexcept { return true; }

	template <typename T, typename Alloc, typename U>
	inline bool operator!=(const krystal_alloc<T, Alloc>&, const krystal_alloc<U, Alloc>) noexcept { return false; }

	// -- standard usages
	template <typename T>
	using lake_alloc = krystal_alloc<T, lake>;
}

#endif
