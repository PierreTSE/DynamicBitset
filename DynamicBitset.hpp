#ifndef DYNAMICBITSET_HPP
#define DYNAMICBITSET_HPP

#if defined(_WIN32) || defined(__WIN32__) || defined(__MINGW32__) || defined(__TOS_WIN__) || defined(__WINDOWS__)
    #define DB_OS_WINDOWS
    #include <intrin.h>
#endif

#if defined(__linux__)
    #define DB_OS_LINUX
#endif

#include <climits>
#include <cstddef>
#include <cstring>
#include <initializer_list>
#include <memory>
#include <type_traits>



namespace ok
{

template<size_t N>
size_t ceil_div(size_t n)
{
    return (n % N == 0 ? n/N : n/N + 1);
}


namespace detail
{
    template<typename Iter>
    struct is_input_iterator 
    {
        static constexpr bool value = std::is_base_of_v<std::input_iterator_tag, typename std::iterator_traits<Iter>::iterator_category>;
    };
    
    template<typename Iter>
    static constexpr bool is_input_iterator_v = is_input_iterator<Iter>::value;
};


template<typename Allocator = std::allocator<std::byte>>
class DynamicBitset : private Allocator
{
    template<bool is_const>
    struct internal_pointer;
public:
    using value_type = bool;
    using allocator_type = Allocator;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    struct reference
    {
        reference(std::byte* ptr, uint8_t off) noexcept;
        reference& operator=(bool) noexcept;
        reference& operator=(reference const&) noexcept;

        void flip() noexcept;

        internal_pointer<false> operator&();

        operator bool() const noexcept;

        bool operator==(reference const& other) const { return bool(*this) == bool(other); }

        bool operator<(reference const& other) const { return !bool(*this) && bool(other); }
        
        friend void swap(reference a, reference b) {
            bool temp = a;
            a = b;
            b = temp;
        }

    private:
        std::byte* const byte;
        uint8_t const offset;
    };

    using const_reference = bool;
    
private:
    template<bool is_const>
    struct internal_pointer
    {
        friend DynamicBitset;
        
        using iterator_category = std::random_access_iterator_tag;
        using value_type = bool;
        using reference = std::conditional_t<is_const, typename DynamicBitset<Allocator>::reference const, typename DynamicBitset<Allocator>::reference>;
        using const_reference = bool;
        using pointer = internal_pointer;
        using const_pointer = typename DynamicBitset<Allocator>::template internal_pointer<true>;
        using difference_type = typename DynamicBitset<Allocator>::difference_type;
        using size_type = typename DynamicBitset<Allocator>::size_type;
        
        internal_pointer() = default;
        
        internal_pointer(std::byte* ptr, uint8_t off) noexcept;
        
        internal_pointer(internal_pointer<false> const& other);

        internal_pointer& operator++() {
            if (offset == 7) {
                ++byte;
                offset = 0;
            } else ++offset;
            return *this;
        }

        internal_pointer operator++(int) {
            auto temp = *this;
            ++*this;
            return temp;
        }

        internal_pointer& operator--() {
            if (offset == 0) {
                --byte;
                offset = 7;
            } else --offset;
            return *this;
        }

        internal_pointer operator--(int) {
            auto temp = *this;
            --*this;
            return temp;
        }

        internal_pointer& operator+=(difference_type d);
        internal_pointer& operator-=(difference_type d);

        internal_pointer operator+(difference_type d) const;
        internal_pointer operator-(difference_type d) const;

        reference operator*() { return reference(byte, offset); }

        const_reference operator*() const { return static_cast<bool>(reference(byte, offset)); }

        reference operator[](difference_type d) const { return *(*this + d); }

        //const_reference operator[](difference_type d) const { return *(*this + d); }
        
        bool operator==(internal_pointer<true> const& other) const { return byte == other.byte && offset == other.offset; }
        bool operator!=(internal_pointer<true> const& other) const { return !(*this == other); }

        bool operator<(internal_pointer<true> const& other) const { if(byte < other.byte) return true; if(byte > other.byte) return false; return offset < other.offset; }
        bool operator>(internal_pointer<true> const& other) const { return other < *this; }
        bool operator<=(internal_pointer<true> const& other) const { return !(*this > other); }
        bool operator>=(internal_pointer<true> const& other) const { return !(*this < other); }

        difference_type operator-(internal_pointer<true> const& other) const { return (byte - other.byte) * 8 + (offset - other.offset); }

    private:
        std::byte* byte;
        uint8_t offset;
    };
    
public:
    using pointer = internal_pointer<false>;
    using const_pointer = internal_pointer<true>;
    using iterator = pointer;
    using const_iterator = const_pointer;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_reverse_iterator = std::reverse_iterator<const_iterator>;

    // Constructors
    DynamicBitset() noexcept(std::is_nothrow_default_constructible_v<Allocator>);
    explicit DynamicBitset(Allocator const& alloc) noexcept(std::is_nothrow_copy_constructible_v<Allocator>);
    explicit DynamicBitset(size_type count, Allocator const& alloc = Allocator());
    explicit DynamicBitset(size_type count, bool value, Allocator const& alloc = Allocator());
    template<size_t N>
    DynamicBitset(bool const (&bools)[N], Allocator const& alloc = Allocator());
    template<typename Iter, typename = std::enable_if_t<detail::is_input_iterator_v<Iter>>>
    DynamicBitset(Iter first, Iter last, Allocator const& alloc = Allocator());
    DynamicBitset(DynamicBitset const& other);
    DynamicBitset(DynamicBitset const& other, Allocator const& alloc);
    DynamicBitset(DynamicBitset&& other) noexcept;
    DynamicBitset(DynamicBitset&& other, Allocator const& alloc) noexcept(std::is_nothrow_copy_constructible_v<Allocator>);
    DynamicBitset(std::initializer_list<bool> ilist, Allocator const& alloc = Allocator());
    
    ~DynamicBitset();

    // Copy and assignment
    DynamicBitset& operator=(DynamicBitset const& other);
    DynamicBitset& operator=(DynamicBitset&& other) noexcept;
    DynamicBitset& operator=(std::initializer_list<bool> ilist);

    template<typename Iter>
    void assign(Iter first, Iter last);
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
    void shrink_to_fit() noexcept;
    size_type capacity() const noexcept;

    // Modifiers
    void clear() noexcept;

    iterator insert(const_iterator pos, bool value);
    iterator insert(const_iterator pos, size_type count, bool value);
    iterator insert(const_iterator pos, const_iterator first, const_iterator last);
    iterator insert(const_iterator pos, std::initializer_list<bool> ilist);

    template<class... Args>
    iterator emplace(const_iterator pos, Args&& ... args);

    iterator erase(const_iterator pos);
    iterator erase(const_iterator first, const_iterator last);

    void push_back(bool&& value);
    void pop_back();

    template<class... Args>
    reference emplace_back(Args&& ... args);

    void resize(size_type count);
    void resize(size_type count, value_type value);

    void swap(DynamicBitset& other) noexcept;

    void flip();
    void flip(size_type n);
    void flip(const_iterator it);

    static void swap(reference x, reference y);

    // Operators
    DynamicBitset& operator&=(DynamicBitset const& b);
    DynamicBitset& operator|=(DynamicBitset const& b);
    DynamicBitset& operator^=(DynamicBitset const& b);
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

    size_type popcount() const;
    size_type popcount(const_iterator pos, size_type n) const;
    size_type popcount(const_iterator first, const_iterator last) const;
    
private:
    void destroy() noexcept;
    allocator_type* alloc() noexcept { return static_cast<allocator_type*>(this); }

private:
    struct data
    {
        std::byte* start = nullptr;
        std::byte* capacity = nullptr;
        uintptr_t size = 0;
    };
    data d;
};

template<typename Allocator>
typename DynamicBitset<Allocator>::size_type DynamicBitset<Allocator>::popcount(const_iterator pos, size_type n) const {
    size_type  sum = 0;
    const auto end = pos + n;

#ifdef DB_OS_WINDOWS
    while (pos != end) {
        if (end - pos >= 64) {
            sum += __popcnt64(reinterpret_cast<__int64>(pos));
            pos += 64;
        } else if (end - pos >= 32) {
            sum += __popcnt(reinterpret_cast<__int32>(pos));
            pos += 32;
        } else if (end - pos >= 16) {
            sum += __popcnt16(reinterpret_cast<__int16>(pos));
            pos += 16;
        } else {
            while (pos != end) {
                sum += *pos++;
            }
        }
    }
#elif defined(DB_OS_LINUX)
    while(pos != end){
        if(end - pos >= sizeof(unsigned long long) * CHAR_BIT){
            sum += __builtin_popcountll(*reinterpret_cast<uint64_t*>(pos.byte));
            pos += 64;
        } else if(end - pos >= sizeof(unsigned long) * CHAR_BIT){
            sum += __builtin_popcountl(*reinterpret_cast<uint32_t*>(pos.byte));
            pos += 32;
        } else if(end - pos >= sizeof(unsigned int) * CHAR_BIT){
            sum += __builtin_popcount(*reinterpret_cast<uint16_t*>(pos.byte));
            pos += 16;
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
DynamicBitset<Allocator>::DynamicBitset() noexcept(std::is_nothrow_default_constructible_v<Allocator>)
{}

template<typename Allocator>
DynamicBitset<Allocator>::DynamicBitset(const Allocator& alloc) noexcept(std::is_nothrow_copy_constructible_v<Allocator>) :
    Allocator{alloc}
{}

template<typename Allocator>
DynamicBitset<Allocator>::DynamicBitset(DynamicBitset::size_type count, const Allocator& alloc) :
    Allocator{alloc}
{
    reserve(count);
    d.size = count;
    memset(d.start, 0, ceil_div<8>(d.size));
}

template<typename Allocator>
void DynamicBitset<Allocator>::reserve(DynamicBitset::size_type new_cap)
{
    if(new_cap > (d.capacity - d.start)*8)
    {
        size_t num_byte = ceil_div<8>(new_cap);
        std::byte* temp = Allocator::allocate(num_byte);
        memcpy(temp, d.start, ceil_div<8>(d.size));
        destroy();
        d.start = temp;
        d.capacity = temp+num_byte;
    }
}

template<typename Allocator>
void DynamicBitset<Allocator>::destroy() noexcept
{
    if(d.start)
        Allocator::deallocate(d.start, d.capacity-d.start);
}

template<typename Allocator>
typename DynamicBitset<Allocator>::size_type DynamicBitset<Allocator>::popcount() const
{
    return popcount(cbegin(), d.size);
}

template<typename Allocator>
typename DynamicBitset<Allocator>::const_iterator DynamicBitset<Allocator>::cbegin() const noexcept
{
    return const_iterator(d.start, 0);
}

template<typename Allocator>
template<size_t N>
DynamicBitset<Allocator>::DynamicBitset(bool const (&bools)[N], Allocator const& alloc) :
    Allocator{alloc}
{
    reserve(N);
    d.size = N;
    auto it = begin();
    for(bool b : bools)
        *it++ = b;        
}

template<typename Allocator>
typename DynamicBitset<Allocator>::iterator DynamicBitset<Allocator>::begin() noexcept
{
    return iterator(d.start, 0);
}

template<typename Allocator>
typename DynamicBitset<Allocator>::const_iterator DynamicBitset<Allocator>::begin() const noexcept
{
    return cbegin();
}

template<typename Allocator>
DynamicBitset<Allocator>::DynamicBitset(DynamicBitset::size_type count, bool value, const Allocator& alloc) :
    Allocator(alloc)
{
    reserve(count);
    d.size = count;
    memset(d.start, value ? 0xFF : 0, ceil_div<8>(d.size));
}

template<typename Allocator>
template<typename Iter>
DynamicBitset<Allocator>::DynamicBitset(Iter first, Iter last, Allocator const& alloc) :
    Allocator(alloc)
{
    auto size = std::distance(first, last);
    reserve(size);
    d.size = size;
    std::copy(first, last, begin());
}

template<typename Allocator>
DynamicBitset<Allocator>::DynamicBitset(DynamicBitset const& other) :
    Allocator(other)
{
    reserve(other.d.size);
    d.size = other.d.size;
    memcpy(d.start, other.d.start, ceil_div<8>(d.size));
}

template<typename Allocator>
DynamicBitset<Allocator>::DynamicBitset(DynamicBitset const& other, const Allocator& alloc):
    Allocator(other)
{
    reserve(other.d.size);
    d.size = other.d.size;
    memcpy(d.start, other.d.start, ceil_div<8>(d.size));
}

template<typename Allocator>
DynamicBitset<Allocator>::DynamicBitset(DynamicBitset&& other) noexcept :
    Allocator(std::move(other))
{
    d = other.d;
    other.d.start = other.d.capacity = nullptr;
    other.d.size = 0;
}

template<typename Allocator>
DynamicBitset<Allocator>::DynamicBitset(DynamicBitset&& other, const Allocator& alloc) noexcept(std::is_nothrow_copy_constructible_v<Allocator>) :
    Allocator(std::move(other))
{
    d = other.d;
    other.d.start = other.d.capacity = nullptr;
    other.d.size = 0;
}

template<typename Allocator>
DynamicBitset<Allocator>::DynamicBitset(std::initializer_list<bool> ilist, const Allocator& alloc) :
    DynamicBitset(ilist.begin(), ilist.end(), alloc)
{}

template<typename Allocator>
DynamicBitset<Allocator>::~DynamicBitset()
{
    destroy();
}

template<typename Allocator>
DynamicBitset<Allocator>& DynamicBitset<Allocator>::operator=(DynamicBitset const& other)
{
    if(std::allocator_traits<Allocator>::propagate_on_container_copy_assignment::value)
    {
        if(get_allocator() != other.get_allocator())
            destroy();
        *alloc() = *other.alloc();
        reserve(other.d.size);
        d.size = other.d.size;
        memcpy(d.start, other.d.start, ceil_div<8>(d.size));
    }
    else
    {
        reserve(other.d.size);
        d.size = other.d.size;
        memcpy(d.start, other.d.start, ceil_div<8>(d.size));
    }
    
    return *this;
}

template<typename Allocator>
DynamicBitset<Allocator>& DynamicBitset<Allocator>::operator=(DynamicBitset&& other) noexcept
{
    if(std::allocator_traits<Allocator>::propagate_on_container_move_assignment::value)
    {
        if(get_allocator() != other.get_allocator())
            destroy();
        *alloc() = *other.alloc();
        reserve(other.d.size);
        d.size = other.d.size;
        memcpy(d.start, other.d.start, ceil_div<8>(d.size));
    }
    else
    {
        d = other.d;
        other.d.start = other.d.capacity = nullptr;
        other.d.size = 0;
    }
    
    return *this;
}

template<typename Allocator>
DynamicBitset<Allocator>& DynamicBitset<Allocator>::operator=(std::initializer_list<bool> ilist)
{
    reserve(ilist.size());
    d.size = ilist.size();
    std::copy(ilist.begin(), ilist.end(), begin());
    
    return *this;
}

template<typename Allocator>
template<typename Iter>
void DynamicBitset<Allocator>::assign(Iter first, Iter last)
{
    
}


template<typename Allocator>
typename DynamicBitset<Allocator>::internal_pointer operator+(typename DynamicBitset<Allocator>::internal_pointer::difference_type lhs, typename DynamicBitset<Allocator>::internal_pointer rhs)
{
    return rhs + lhs;
}

template<typename Allocator>
DynamicBitset<Allocator>::reference::reference(std::byte* ptr, uint8_t off) noexcept :
        byte{ptr}, offset{off} {}

template<typename Allocator>
typename DynamicBitset<Allocator>::reference& DynamicBitset<Allocator>::reference::operator=(bool b) noexcept {
    if (b)
        *byte |= (std::byte(1) << 7 - offset);
    else
        *byte &= ~(std::byte(1) << 7 - offset);
    return *this;
}

template<typename Allocator>
typename DynamicBitset<Allocator>::reference& DynamicBitset<Allocator>::reference::operator=(reference const& other) noexcept {
    return *this = static_cast<bool>(other);
}

template<typename Allocator>
void DynamicBitset<Allocator>::reference::flip() noexcept {
    *byte ^= (std::byte(1) << 7 - offset);
}

template<typename Allocator>
typename DynamicBitset<Allocator>::template internal_pointer<false> DynamicBitset<Allocator>::reference::operator&()
{
    return internal_pointer(byte, offset);
}

template<typename Allocator>
DynamicBitset<Allocator>::reference::operator bool() const noexcept {
    return std::to_integer<bool>(*byte & (std::byte(1) << 7 - offset));
}

template<typename Allocator>
template<bool is_const>
DynamicBitset<Allocator>::internal_pointer<is_const>::internal_pointer(std::byte* ptr, uint8_t off) noexcept :
byte{ptr}, offset{off}
{}

template<typename Allocator>
template<bool is_const>
typename DynamicBitset<Allocator>::template internal_pointer<is_const>::pointer& DynamicBitset<Allocator>::internal_pointer<is_const>::operator
+=(difference_type d) {
    if (d < 0)
        return *this -= -d;
    byte += d / 8;
    if (offset + d % 8 > 7) {
        ++byte;
        offset += d % 8 - 8;
    } else
        offset += d % 8;
    return *this;
}

template<typename Allocator>
template<bool is_const>
typename DynamicBitset<Allocator>::template internal_pointer<is_const>::pointer& DynamicBitset<Allocator>::internal_pointer<is_const>::operator
-=(difference_type d) {
    if (d < 0)
        return *this += -d;
    byte -= d / 8;
    if (static_cast<ptrdiff_t>(offset) - d % 8 < 0) {
        --byte;
        offset -= d % 8 - 8;
    } else
        offset -= d % 8;
    return *this;
}

template<typename Allocator>
template<bool is_const>
typename DynamicBitset<Allocator>::template internal_pointer<is_const>::pointer DynamicBitset<Allocator>::internal_pointer<is_const>::operator+(
        difference_type d) const {
    if (d < 0)
        return *this - -d;
    auto temp = byte + d / 8;
    if (offset + d % 8 > 7)
        return internal_pointer(temp + 1, offset + d % 8 - 8);
    return internal_pointer(temp, offset + d % 8);
}

template<typename Allocator>
template<bool is_const>
typename DynamicBitset<Allocator>::template internal_pointer<is_const>::pointer DynamicBitset<Allocator>::internal_pointer<is_const>::operator-(
        difference_type d) const {
    if (d < 0)
        return *this + -d;
    auto temp = byte - d / 8;
    if (static_cast<ptrdiff_t>(offset) - d % 8 < 0)
        return internal_pointer(temp - 1, offset - d % 8 + 8);
    return internal_pointer(temp, offset - d % 8);
}

template<typename Allocator>
template<bool is_const>
DynamicBitset<Allocator>::internal_pointer<is_const>::internal_pointer(DynamicBitset::internal_pointer<false> const& other) :
    byte{other.byte}, offset{other.offset}
{}

// Non member operators

template<typename Allocator>
bool operator==(DynamicBitset<Allocator> const& lhs, DynamicBitset<Allocator> const& rhs) {
    return (lhs.size() == rhs.size() && std::equal(lhs.begin(), lhs.end(), rhs.begin()));
}

template<typename Allocator>
bool operator!=(DynamicBitset<Allocator> const& lhs, DynamicBitset<Allocator> const& rhs) {
    return !(lhs == rhs);
}

template<typename Allocator>
bool operator<(DynamicBitset<Allocator> const& lhs, DynamicBitset<Allocator> const& rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
}

template<typename Allocator>
bool operator<=(DynamicBitset<Allocator> const& lhs, DynamicBitset<Allocator> const& rhs) {
    return !(lhs < rhs);
}

template<typename Allocator>
bool operator>(DynamicBitset<Allocator> const& lhs, DynamicBitset<Allocator> const& rhs) {
    return rhs > lhs;
}

template<typename Allocator>
bool operator>=(DynamicBitset<Allocator> const& lhs, DynamicBitset<Allocator> const& rhs) {
    return !(lhs < rhs);
}

template<typename Allocator>
void swap(DynamicBitset<Allocator>& lhs, DynamicBitset<Allocator>& rhs) noexcept;

};
#endif // DYNAMICBITSET_HPP
