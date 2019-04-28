#ifndef DYNAMICBITSET_HPP
#define DYNAMICBITSET_HPP

#include "test_sse_macro.hpp"
#ifdef HAS_SSE
    #include <immintrin.h>
#endif
#include <initializer_list>

template<typename Allocator = std::allocator<std::byte>>
class DynamicBitset
{
public:
    using value_type = bool;
    using allocator_type = Allocator;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    struct pointer;
    struct reference
    {
        reference(std::byte* ptr, uint8_t off) noexcept;
        reference& operator=(bool) noexcept;
        reference& operator=(reference const&) noexcept;

        void flip() noexcept;

        pointer operator&();

        operator bool() const noexcept;

    private:
        std::byte* const byte;
        uint8_t const offset;
    };
    using const_reference = bool;
    struct pointer
    {
        using iterator_category = std::random_access_iterator_tag;
        using value_type = bool;
        using reference = reference;
        using const_reference = bool;
        using pointer = pointer;
        using const_pointer = typename DynamicBitset<Allocator>::pointer const;
        using difference_type = difference_type;
        using size_type = size_type;

        using internal_pointer = typename DynamicBitset<Allocator>::pointer;

        pointer(std::byte* ptr, uint8_t off) noexcept;

        internal_pointer& operator++() { if(offset == 7) {++byte; offset = 0;} else ++offset; return *this; }
        internal_pointer operator++(int) { auto temp = *this; ++*this; return temp; }

        internal_pointer& operator--() { if(offset == 0) {--byte; offset = 7;} else --offset; return *this; }
        internal_pointer operator--(int) { auto temp = *this; --*this; return temp; }
        
        internal_pointer& operator+=(difference_type d)
        {
            if(d < 0)
                return *this -= -d;
            byte += d/8;
            if(offset + d%8 > 7)
            {
                ++byte;
                offset += d%8 - 8;
            }
			else
                offset += d%8;
            return *this;
        }
        internal_pointer& operator-=(difference_type d)
        {
            if(d < 0)
                return *this += -d;
            byte -= d/8;
            if(static_cast<ptrdiff_t>(offset) - d%8 < 0)
            {
                --byte;
                offset -= d%8 - 8;
            }
			else
                offset -= d%8;
            return *this;
        }

        reference operator*() { return reference(byte, offset); }
        const_reference operator*() const { return static_cast<bool>(reference(byte, offset)); }
        
        internal_pointer operator+(difference_type d)
        {
            if(d < 0)
                return *this - -d;
            auto temp = byte + d/8;
            if(offset + d%8 > 7)
                return internal_pointer(temp + 1, offset + d%8 - 8);
            return internal_pointer(temp, offset + d%8);
        }
        internal_pointer operator-(difference_type d)
        {
            if(d < 0)
                return *this + -d;
            auto temp = byte - d/8;
            if(static_cast<ptrdiff_t>(offset) - d%8 < 0)
                return internal_pointer(temp - 1, offset - d%8 + 8);
            return internal_pointer(temp, offset - d%8);
        }

        reference operator[](difference_type d) { return *(*this + d); }
        const_reference operator[](difference_type d) const { return *(*this + d); }
        
        bool operator==(internal_pointer const& other) const { return byte == other.byte && offset == other.offset; }
        bool operator!=(internal_pointer const& other) const { return !(*this == other); }

        bool operator<(internal_pointer const& other) const { if(byte < other.byte) return true; if(byte > other.byte) return false; return offset < other.offset; }
        bool operator>(internal_pointer const& other) const { return other < *this; }
        bool operator<=(internal_pointer const& other) const { return !(*this > other); }
        bool operator>=(internal_pointer const& other) const { return !(*this < other); }

        difference_type operator-(internal_pointer const& other) { return (byte - other.byte) * 8 + (offset - other.offset); }

    private:
        std::byte* byte;
        uint8_t offset;
    };
    using const_pointer = pointer; // TODO pas sur
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>();
    using const_reverse_iterator = std::reverse_iterator<const_iterator>();

    // Constructors
    DynamicBitset() noexcept(noexcept(Allocator()));
    explicit DynamicBitset(Allocator const& alloc) noexcept;
    explicit DynamicBitset(size_type count, Allocator const& alloc = Allocator());
    DynamicBitset(DynamicBitset const& other, Allocator const& alloc);
    DynamicBitset(const_iterator first, const_iterator last, Allocator const& alloc = Allocator());
    DynamicBitset(DynamicBitset&& other) noexcept;
    DynamicBitset(DynamicBitset&& other, Allocator const& alloc);
    DynamicBitset(std::initializer_list<bool> ilist, Allocator const& alloc = Allocator());

    // Copy and assignment
    DynamicBitset& operator=(DynamicBitset const& other);
    DynamicBitset& operator=(DynamicBitset&& other) noexcept;
    DynamicBitset& operator=(std::initializer_list<bool> ilist);

    void assign(const_iterator first, const_iterator last);
    void assign(std::initializer_list<bool> ilist);

    // Element access
    reference at(size_type pos);
    const_reference at(size_type pos) const;

    reference front();
    const_reference front() const;

    reference back();
    const_reference back() const;

    // Iterators
    iterator begin() noexcept;
    const_iterator begin() const noexcept;
    const_iterator cbegin() const noexcept;
        
    iterator end() noexcept;
    const_iterator end() const noexcept;
    const_iterator cend() const noexcept;

    reverse_iterator rbegin() noexcept;
    const_reverse_iterator rbegin() const noexcept;
    const_reverse_iterator crbegin() const noexcept;

    reverse_iterator rend() noexcept;
    const_reverse_iterator rend() const noexcept;
    const_reverse_iterator crend() const noexcept;

    // Capacity
    bool empty() const noexcept;

    size_type size() const noexcept;

    size_type max_size() const noexcept;

    void reserve(size_type new_cap);

    size_type capacity() const noexcept;

    // Modifiers
    void clear() noexcept;

    iterator insert(const_iterator pos, bool value);
    iterator insert(const_iterator pos, bool&& value);
    iterator insert(const_iterator pos, size_type count, const T& value);
    iterator insert(const_iterator pos, const_iterator first, const_iterator last);
    iterator insert(const_iterator pos, std::initializer_list<T> ilist);

    template< class... Args > 
    iterator emplace(const_iterator pos, Args&&... args);

    iterator erase(const_iterator pos);
    iterator erase(const_iterator first, const_iterator last);

    void push_back(bool&& value);
    void pop_back();

    template< class... Args >
    reference emplace_back(Args&&... args);

    void resize(size_type count);
    void resize(size_type count, const value_type& value);

    void swap(DynamicBitset& other) noexcept;

    void flip();
    void flip(size_type n);
    void flip(const_iterator it);

    static void swap(reference x, reference y);

    // Operators
    DynamicBitset& operator&=(DynamicBitset const& b);
    DynamicBitset& operator|=(DynamicBitset const& b);
    DynamicBitset& operator^=(DynamicBitset const& b);
    DynamicBitset& operator-=(DynamicBitset const& b);
    DynamicBitset& operator<<=(size_type n);
    DynamicBitset& operator>>=(size_type n);
    DynamicBitset operator<<(size_type n) const;
    DynamicBitset operator>>(size_type n) const;
    DynamicBitset operator~() const;
    reference operator[](size_type pos);
    bool operator[](size_type pos) const;

    // Member functions
    allocator_type get_allocator() const;

    bool any() const;

    bool none() const;

    size_type find_first() const;
    size_type find_next(size_type pos) const;
    size_type find_next(const_iterator it) const;

    size_type popcount(const_iterator pos, size_type n) const;
    size_type popcount(const_iterator first, const_iterator last) const;

private:
    struct data
    {
        pointer start = nullptr;
        pointer finish = nullptr;
        std::byte* end = nullptr;
    };
};

template<typename Allocator>
DynamicBitset::size_type DynamicBitset<Allocator>::popcount(DynamicBitset::const_iterator pos, DynamicBitset::size_type n) const {
    DynamicBitset::size_type sum = 0;
    const auto end = pos + n;
#ifdef HAS_SSE4_1
    while(pos != end){
        if(end - pos >= 64){
            sum += _mm_popcnt_u64(reinterpret_cast<__int64>(pos));
            pos += 64;
        } else if(end - pos >= 32){
            sum += _mm_popcnt_u64(reinterpret_cast<__int32>(pos));
            pos += 32;
        } else{
            while(pos != end){
                sum += *pos++;
            }
        }
    }
#else
    while(pos != end){
        sum += *pos++;
    }
#endif
    return sum;
}


template<typename Allocator>
typename DynamicBitset<Allocator>::pointer operator+(typename DynamicBitset<Allocator>::pointer::difference_type lhs, typename DynamicBitset<Allocator>::pointer rhs)
{
    return rhs + lhs;
}

template<typename Allocator>
DynamicBitset<Allocator>::reference::reference(std::byte* ptr, uint8_t off) noexcept :
byte{ptr}, offset{off}
{}

template<typename Allocator>
typename DynamicBitset<Allocator>::reference& DynamicBitset<Allocator>::reference::operator=(bool b) noexcept
{
    if(b)
        *byte |= (std::byte(1) << 7-offset);
	else
        *byte &= ~(std::byte(1) << 7-offset);
    return *this;
}

template<typename Allocator>
typename DynamicBitset<Allocator>::reference& DynamicBitset<Allocator>::reference::operator=(reference const& other) noexcept
{
    return *this = static_cast<bool>(other);
}

template<typename Allocator>
void DynamicBitset<Allocator>::reference::flip() noexcept
{
    *byte ^= (std::byte(1) << 7-offset);
}

template<typename Allocator>
typename DynamicBitset<Allocator>::pointer DynamicBitset<Allocator>::reference::operator&()
{
    return pointer(byte, offset);
}

template<typename Allocator>
DynamicBitset<Allocator>::reference::operator bool() const noexcept 
{
    return std::to_integer<bool>(*byte & (std::byte(1) << 7-offset));
}

template<typename Allocator>
DynamicBitset<Allocator>::pointer::pointer(std::byte* ptr, uint8_t off) noexcept :
byte{ptr}, offset{off}
{}


#endif // DYNAMICBITSET_HPP
