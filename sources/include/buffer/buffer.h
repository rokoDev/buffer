#ifndef buffer_h
#define buffer_h

#include <cstddef>
#include <type_traits>

namespace buffer
{
namespace details
{
template <typename DataT, bool kConstFlag>
class buffer_view_base
{
   public:
    using pointer = std::add_pointer_t<
        std::conditional_t<kConstFlag, std::add_const_t<DataT>, DataT>>;
    using value_type = DataT;
    static constexpr bool kIsConst = kConstFlag;

    ~buffer_view_base() = default;

    constexpr buffer_view_base() noexcept = delete;

    constexpr buffer_view_base(pointer aData, std::size_t aSize) noexcept
        : data_(aData), size_(aSize)
    {
    }

    template <std::size_t N, typename T>
    constexpr explicit buffer_view_base(T (&aData)[N]) noexcept
        : buffer_view_base(aData, N)
    {
    }

    inline constexpr pointer data_at_unsafe(std::size_t aIndex) const noexcept
    {
        if constexpr (std::is_same_v<value_type, void>)
        {
            using CharPtrT =
                std::conditional_t<kConstFlag, char const *, char *>;
            return static_cast<CharPtrT>(data_) + aIndex;
        }
        else
        {
            return data_ + aIndex;
        }
    }

    inline constexpr pointer data() const noexcept { return data_; }

    inline constexpr std::size_t size() const noexcept { return size_; }

   private:
    pointer data_{};
    std::size_t size_{};
};
}  // namespace details

template <typename T>
using buffer_view_const = details::buffer_view_base<T, true>;

template <typename T>
using buffer_view = details::buffer_view_base<T, false>;
}  // namespace buffer

#endif /* buffer_h */
