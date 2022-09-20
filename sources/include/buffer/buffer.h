#ifndef buffer_h
#define buffer_h

#include <strong_type/strong_type.h>

#include <array>
#include <boost/leaf.hpp>
#include <cstddef>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace buffer
{
namespace leaf = boost::leaf;

template <class T>
using result = leaf::result<T>;

enum class error
{
    null_data = 1,
    zero_size,
    null_data_and_zero_size,
    invalid_index,
    unknown
};

namespace details
{
template <typename StrongT>
struct NecessaryOps
    : strong::plus<StrongT>
    , strong::plus_assignment<StrongT>
    , strong::convertible_to_bool<StrongT>
    , strong::modulo<StrongT>
    , strong::division<StrongT>
    , strong::comparisons<StrongT>
    , strong::implicitly_convertible_to_underlying<StrongT>
{
};
}  // namespace details

using NBits =
    strong::strong_type<struct NBitsTag, std::size_t, details::NecessaryOps>;
using NBytes =
    strong::strong_type<struct NBytesTag, std::size_t, details::NecessaryOps>;

namespace details
{
template <typename T>
struct has_contiguous_storage : public std::false_type
{
};

template <typename T>
struct has_contiguous_storage<std::vector<T>> : public std::true_type
{
};

template <>
struct has_contiguous_storage<std::vector<bool>> : public std::false_type
{
};

template <typename T, std::size_t N>
struct has_contiguous_storage<std::array<T, N>> : public std::true_type
{
};

template <typename T>
struct has_contiguous_storage<
    std::basic_string<T, std::char_traits<T>, std::allocator<T>>>
    : public std::true_type
{
};

template <typename T>
struct has_contiguous_storage<std::basic_string_view<T, std::char_traits<T>>>
    : public std::true_type
{
};

template <typename T, bool isConst>
class buffer_view_base;

template <typename T, bool isConst>
struct has_contiguous_storage<buffer_view_base<T, isConst>>
    : public std::true_type
{
};

template <typename T>
inline constexpr bool has_contiguous_storage_v =
    has_contiguous_storage<T>::value;

template <typename T>
result<void> validate_args(T aData, NBytes aSize) noexcept
{
    if (aData && aSize)
    {
        return {};
    }
    if (!aData && !aSize)
    {
        return leaf::new_error(error::null_data_and_zero_size);
    }
    if (!aData)
    {
        return leaf::new_error(error::null_data);
    }
    else
    {
        return leaf::new_error(error::zero_size);
    }
}

template <typename T, bool isConst>
class buffer_view_base
{
    friend buffer_view_base<T, not isConst>;

   public:
    using pointer =
        std::add_pointer_t<std::conditional_t<isConst, std::add_const_t<T>, T>>;
    using value_type = T;

    static result<buffer_view_base<T, isConst>> create(pointer aDataPtr,
                                                       NBytes aSize) noexcept
    {
        BOOST_LEAF_CHECK(validate_args(aDataPtr, aSize));
        return buffer_view_base<T, isConst>(aDataPtr, aSize);
    }

    ~buffer_view_base() = default;

    buffer_view_base() noexcept = delete;

    constexpr operator buffer_view_base<T, true>() const
    {
        return buffer_view_base<T, true>(data_, size_);
    }

    inline constexpr pointer data_unsafe(NBytes aIndex) const noexcept
    {
        if constexpr (std::is_same_v<value_type, void>)
        {
            using CharPtrT = std::conditional_t<isConst, char const *, char *>;
            return static_cast<CharPtrT>(data_) + aIndex;
        }
        else
        {
            return data_ + aIndex;
        }
    }

    inline result<pointer> data(NBytes aIndex) const noexcept
    {
        if (aIndex < size_)
        {
            return data_unsafe(aIndex);
        }
        return leaf::new_error(error::invalid_index);
    }

    inline result<value_type> operator[](NBytes aIndex) const noexcept
    {
        BOOST_LEAF_AUTO(dataPtr, data(aIndex));
        return *dataPtr;
    }

    inline constexpr pointer data() const noexcept { return data_; }

    inline constexpr NBytes size() const noexcept { return size_; }

   private:
    inline constexpr buffer_view_base(pointer aData, NBytes aSize) noexcept
        : data_(aData), size_(aSize)
    {
    }

    pointer data_{};
    NBytes size_{};
};
}  // namespace details

template <typename T>
using buffer_view = details::buffer_view_base<T, false>;

template <typename T>
using buffer_view_const = details::buffer_view_base<T, true>;

template <typename T>
struct is_bv : public std::false_type
{
};

template <typename T>
struct is_bv<buffer_view<T>> : public std::true_type
{
};

template <typename T>
inline constexpr bool is_bv_v = is_bv<T>::value;

template <typename T>
struct is_bv_const : public std::false_type
{
};

template <typename T>
struct is_bv_const<buffer_view_const<T>> : public std::true_type
{
};

template <typename T>
inline constexpr bool is_bv_const_v = is_bv_const<T>::value;

template <typename T, typename DataT = std::remove_pointer_t<T>>
inline result<buffer_view<DataT>> make_bv(T aDataPtr, NBytes aSize) noexcept
{
    static_assert(not std::is_const_v<DataT>,
                  "aDataPtr parameter should be pointer to non const data.");
    return buffer_view<DataT>::create(aDataPtr, aSize);
}

template <typename T,
          typename DataT = std::remove_const_t<std::remove_pointer_t<T>>>
inline result<buffer_view_const<DataT>> make_bv_const(T aDataPtr,
                                                      NBytes aSize) noexcept
{
    using PointerT = std::add_pointer_t<std::add_const_t<DataT>>;
    return buffer_view_const<DataT>::create(static_cast<PointerT>(aDataPtr),
                                            aSize);
}

template <typename T, std::size_t N>
inline auto make_bv(T (&aData)[N]) noexcept
{
    return make_bv(aData, NBytes(N));
}

template <typename T, std::size_t N>
inline auto make_bv_const(T (&aData)[N]) noexcept
{
    return make_bv_const(aData, NBytes(N));
}

template <typename T>
inline auto make_bv(T &&aParam) noexcept
{
    using U = std::remove_cv_t<std::remove_reference_t<T>>;
    if constexpr (is_bv_v<U>)
    {
        return make_bv(aParam.data(), aParam.size());
    }
    else
    {
        static_assert(std::is_lvalue_reference_v<T>,
                      "aContainer's parameter type must be lvalue reference.");
        static_assert(
            details::has_contiguous_storage_v<std::remove_reference_t<T>>,
            "aContainer must have contiguous storage.");
        static_assert(not std::is_const_v<T>,
                      "aContainer parameter must be non const.");
        return make_bv(aParam.data(), NBytes(aParam.size()));
    }
}

template <typename T>
inline auto make_bv_const(T &&aParam) noexcept
{
    using U = std::remove_cv_t<std::remove_reference_t<T>>;
    if constexpr (is_bv_v<U> || is_bv_const_v<U>)
    {
        return make_bv_const(aParam.data(), aParam.size());
    }
    else
    {
        static_assert(std::is_lvalue_reference_v<T>,
                      "aContainer's parameter type must be lvalue reference.");
        static_assert(details::has_contiguous_storage_v<
                          std::remove_cv_t<std::remove_reference_t<T>>>,
                      "aContainer must have contiguous storage.");
        return make_bv_const(aParam.data(), NBytes(aParam.size()));
    }
}

class buf_pos
{
   public:
    ~buf_pos() = default;
    buf_pos() = default;
    buf_pos(const buf_pos &) = default;
    buf_pos &operator=(const buf_pos &) = default;
    buf_pos(buf_pos &&) noexcept = default;
    buf_pos &operator=(buf_pos &&) noexcept = default;

    explicit constexpr buf_pos(NBits aNBits) noexcept : bitIndex_(aNBits) {}

    constexpr buf_pos(NBytes aNBytes, NBits aNBits) noexcept
        : bitIndex_(NBits(aNBytes * CHAR_BIT) + aNBits)
    {
    }

    explicit constexpr buf_pos(NBytes aNBytes) noexcept
        : bitIndex_(aNBytes * CHAR_BIT)
    {
    }

    inline constexpr buf_pos &operator=(const NBits &aNBits) noexcept
    {
        bitIndex_ = aNBits.get();
        return *this;
    }

    inline constexpr buf_pos &operator=(const NBytes &aNBytes) noexcept
    {
        bitIndex_ = aNBytes * CHAR_BIT;
        return *this;
    }

    inline constexpr std::size_t bitIndex() const noexcept { return bitIndex_; }

    inline constexpr std::size_t byteIndex() const noexcept
    {
        return bitIndex_ / CHAR_BIT;
    }

    inline constexpr uint_fast8_t bitOffset() const noexcept
    {
        return bitIndex_ % CHAR_BIT;
    }

    inline constexpr void reset() noexcept { bitIndex_ = 0; }

   private:
    std::size_t bitIndex_{};
};
inline constexpr buf_pos operator+(const buf_pos &aBufPos,
                                   const NBits &aNBits) noexcept
{
    return buf_pos(NBits(aBufPos.bitIndex() + aNBits.get()));
}

inline constexpr buf_pos operator+(const NBits &aNBits,
                                   const buf_pos &aBufPos) noexcept
{
    return aBufPos + aNBits;
}

inline constexpr buf_pos &operator+=(buf_pos &aBufPos,
                                     const NBits &aNBits) noexcept
{
    aBufPos = NBits(aBufPos.bitIndex() + aNBits);
    return aBufPos;
}

inline constexpr buf_pos operator+(const buf_pos &aBufPos,
                                   const NBytes &aNBytes) noexcept
{
    return buf_pos(NBits(aBufPos.bitIndex() + aNBytes * CHAR_BIT));
}

inline constexpr buf_pos operator+(const NBytes &aNBytes,
                                   const buf_pos &aBufPos) noexcept
{
    return aBufPos + aNBytes;
}

inline constexpr buf_pos &operator+=(buf_pos &aBufPos,
                                     const NBytes &aNBytes) noexcept
{
    aBufPos = NBits(aBufPos.bitIndex() + aNBytes * CHAR_BIT);
    return aBufPos;
}

inline constexpr bool operator==(const buf_pos &aLhs,
                                 const buf_pos &aRhs) noexcept
{
    return aLhs.bitIndex() == aRhs.bitIndex();
}

inline constexpr bool operator!=(const buf_pos &aLhs,
                                 const buf_pos &aRhs) noexcept
{
    return aLhs.bitIndex() != aRhs.bitIndex();
}

inline constexpr bool operator<(const buf_pos &aLhs,
                                const buf_pos &aRhs) noexcept
{
    return aLhs.bitIndex() < aRhs.bitIndex();
}

inline constexpr bool operator>(const buf_pos &aLhs,
                                const buf_pos &aRhs) noexcept
{
    return aLhs.bitIndex() > aRhs.bitIndex();
}

inline constexpr bool operator<=(const buf_pos &aLhs,
                                 const buf_pos &aRhs) noexcept
{
    return aLhs.bitIndex() <= aRhs.bitIndex();
}

inline constexpr bool operator>=(const buf_pos &aLhs,
                                 const buf_pos &aRhs) noexcept
{
    return aLhs.bitIndex() >= aRhs.bitIndex();
}
}  // namespace buffer

#endif /* buffer_h */
