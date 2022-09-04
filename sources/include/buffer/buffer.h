#ifndef buffer_h
#define buffer_h

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
    null_data_pointer = 1,
    zero_size,
    null_data_and_zero_size,
    invalid_index,
    unknown
};

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
result<void> validate_args(T aData, std::size_t aSize) noexcept
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
        return leaf::new_error(error::null_data_pointer);
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

    static result<buffer_view_base<T, isConst>> create(
        pointer aDataPtr, std::size_t aSize) noexcept
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

    inline constexpr pointer data_unsafe(std::size_t aIndex) const noexcept
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

    inline result<pointer> data(std::size_t aIndex) const noexcept
    {
        if (aIndex < size_)
        {
            return data_unsafe(aIndex);
        }
        return leaf::new_error(error::invalid_index);
    }

    inline result<value_type> operator[](std::size_t aIndex) const noexcept
    {
        BOOST_LEAF_AUTO(dataPtr, data(aIndex));
        return *dataPtr;
    }

    inline constexpr pointer data() const noexcept { return data_; }

    inline constexpr std::size_t size() const noexcept { return size_; }

   private:
    inline constexpr buffer_view_base(pointer aData, std::size_t aSize) noexcept
        : data_(aData), size_(aSize)
    {
    }

    pointer data_{};
    std::size_t size_{};
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
inline result<buffer_view<DataT>> make_bv(T aDataPtr,
                                          std::size_t aSize) noexcept
{
    static_assert(not std::is_const_v<DataT>,
                  "aDataPtr parameter should be pointer to non const data.");
    return buffer_view<DataT>::create(aDataPtr, aSize);
}

template <typename T,
          typename DataT = std::remove_const_t<std::remove_pointer_t<T>>>
inline result<buffer_view_const<DataT>> make_bv_const(
    T aDataPtr, std::size_t aSize) noexcept
{
    using PointerT = std::add_pointer_t<std::add_const_t<DataT>>;
    return buffer_view_const<DataT>::create(static_cast<PointerT>(aDataPtr),
                                            aSize);
}

template <typename T, std::size_t N>
inline auto make_bv(T (&aData)[N]) noexcept
{
    return make_bv(aData, N);
}

template <typename T, std::size_t N>
inline auto make_bv_const(T (&aData)[N]) noexcept
{
    return make_bv_const(aData, N);
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
        return make_bv(aParam.data(), aParam.size());
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
        return make_bv_const(aParam.data(), aParam.size());
    }
}
}  // namespace buffer

#endif /* buffer_h */
