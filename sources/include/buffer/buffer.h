#ifndef buffer_h
#define buffer_h

#include <strong_type/strong_type.h>

#include <array>
#include <climits>
#include <cstddef>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace buffer
{
enum class error
{
    null_data = 1,
    zero_size,
    null_data_and_zero_size,
    invalid_index
};

namespace details
{
template <typename StrongT>
struct NecessaryOps
    : strong::plus<StrongT>
    , strong::plus_assignment<StrongT>
    , strong::minus<StrongT>
    , strong::minus_assignment<StrongT>
    , strong::pre_increment<StrongT>
    , strong::post_increment<StrongT>
    , strong::convertible_to_bool<StrongT>
    , strong::modulo<StrongT>
    , strong::division<StrongT>
    , strong::comparisons<StrongT>
    , strong::implicitly_convertible_to_underlying<StrongT>
    , strong::multiplication_assignment<StrongT>
    , strong::multiplication<StrongT>
{
};

template <typename StrongT>
struct bit_pos_special
{
    inline constexpr std::size_t byteIndex() const noexcept
    {
        return static_cast<const StrongT &>(*this).get() / CHAR_BIT;
    }
    inline constexpr uint_fast8_t bitOffset() const noexcept
    {
        return static_cast<const StrongT &>(*this).get() % CHAR_BIT;
    }
    inline constexpr std::size_t bytesUsed() const noexcept
    {
        const std::size_t kBytesUsed =
            byteIndex() + (bitOffset() ? static_cast<std::size_t>(1)
                                       : static_cast<std::size_t>(0));
        return kBytesUsed;
    }
    inline constexpr void reset() noexcept
    {
        static_cast<StrongT &>(*this).get() = 0;
    }
};
}  // namespace details

using bit_pos =
    strong::strong_type<struct bit_posTag, std::size_t, details::NecessaryOps,
                        details::bit_pos_special>;

namespace details
{
template <typename StrongT>
struct nbits_special
{
    explicit constexpr operator bit_pos() const noexcept
    {
        static_assert(strong::is_strong_v<StrongT>, "Invalid StrongT.");
        using T = typename StrongT::value_type;
        const T kNumBits = static_cast<const StrongT &>(*this).get();
        return bit_pos(kNumBits);
    }
};

template <typename StrongT>
struct nbytes_special
{
    explicit constexpr operator bit_pos() const noexcept
    {
        static_assert(strong::is_strong_v<StrongT>, "Invalid StrongT.");
        using T = typename StrongT::value_type;
        const T kNumBytes = static_cast<const StrongT &>(*this).get();
        return bit_pos(kNumBytes * CHAR_BIT);
    }
};
}  // namespace details

using n_bits =
    strong::strong_type<struct n_bitsTag, std::size_t, details::NecessaryOps,
                        details::nbits_special>;
using n_bytes =
    strong::strong_type<struct n_bytesTag, std::size_t, details::NecessaryOps,
                        details::nbytes_special>;

namespace details
{
template <typename T, bool isConst>
class buffer_view_base
{
   public:
    template <std::size_t N>
    using array_t = std::conditional_t<isConst, std::add_const_t<T>, T> (&)[N];
    using pointer =
        std::add_pointer_t<std::conditional_t<isConst, std::add_const_t<T>, T>>;
    using value_type = T;

    class reference final
    {
       public:
        using buffer_type = buffer_view_base<T, isConst>;
        inline constexpr reference(std::size_t aPos, buffer_type &aBuf) noexcept
            : pos_(aPos), buf_(&aBuf)
        {
        }

        inline constexpr reference &operator=(value_type aValue) noexcept
        {
            *buf_->data(pos_) = aValue;
            return *this;
        }

        inline constexpr reference &operator=(const reference &aOther) noexcept
        {
            *buf_->data(pos_) = value_type(aOther);
            return *this;
        }

        operator value_type() const { return *buf_->data(pos_); }
        operator pointer() const { return buf_->data(pos_); }

       private:
        const n_bytes pos_{};
        buffer_type *buf_{};
    };

    ~buffer_view_base() = default;

    buffer_view_base() noexcept = delete;

    constexpr operator buffer_view_base<T, true>() const noexcept
    {
        return buffer_view_base<T, true>(data_, size_);
    }

    inline constexpr pointer data(n_bytes aIndex) const noexcept
    {
        assert(aIndex < size_ && "Invalid aIndex");
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

    inline constexpr pointer data() const noexcept { return data_; }

    inline constexpr reference operator[](n_bytes aIndex) noexcept
    {
        assert(aIndex < size_ && "Invalid aIndex");
        return {aIndex, *this};
    }

    inline constexpr n_bytes size() const noexcept { return size_; }

    inline constexpr n_bits bit_size() const noexcept
    {
        return n_bits(size_ * static_cast<std::size_t>(CHAR_BIT));
    }

    inline constexpr buffer_view_base(pointer aData, n_bytes aSize) noexcept
        : data_(aData), size_(aSize)
    {
        assert(aData && "Invalid aData");
        assert(aSize && "Invalid aSize");
    }

    template <std::size_t NBytes>
    constexpr explicit buffer_view_base(array_t<NBytes> aData) noexcept
        : buffer_view_base(aData, n_bytes(NBytes))
    {
    }

   protected:
    pointer data_{};
    n_bytes size_{};
};
}  // namespace details

template <typename T>
using buffer_view = details::buffer_view_base<T, false>;

template <typename T>
using buffer_view_const = details::buffer_view_base<T, true>;

template <typename T>
struct is_buffer_view : public std::false_type
{
};

template <typename T>
struct is_buffer_view<buffer_view<T>> : public std::true_type
{
};

template <typename T>
inline constexpr bool is_buffer_view_v = is_buffer_view<T>::value;

template <typename T>
struct is_buffer_view_const : public std::false_type
{
};

template <typename T>
struct is_buffer_view_const<buffer_view_const<T>> : public std::true_type
{
};

template <typename T>
inline constexpr bool is_buffer_view_const_v = is_buffer_view_const<T>::value;
}  // namespace buffer

#endif /* buffer_h */
