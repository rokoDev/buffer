#include <buffer/buffer.h>
#include <gtest/gtest.h>
#include <type_name/type_name.h>

#include <array>
#include <type_traits>

using namespace std::literals::string_view_literals;

namespace leaf = boost::leaf;

template <class T>
using result = leaf::result<T>;

template <typename E>
constexpr
    typename std::enable_if_t<std::is_enum_v<E>, std::underlying_type_t<E>>
    to_underlying(E e) noexcept
{
    return static_cast<typename std::underlying_type_t<E>>(e);
}

template <typename BufT>
constexpr void validateGeneralBufferTypeProperties() noexcept
{
    static_assert(not std::is_default_constructible_v<BufT>,
                  "BufT supposed to be not default constructible.");
    static_assert(not std::is_trivially_default_constructible_v<BufT>,
                  "BufT supposed to be not trivially default constructible.");
    static_assert(not std::is_nothrow_default_constructible_v<BufT>,
                  "BufT supposed to be not nothrow default constructible");

    static_assert(std::is_trivially_copy_constructible_v<BufT>,
                  "BufT must be trivially copy constructible.");
    static_assert(std::is_nothrow_copy_constructible_v<BufT>,
                  "BufT must be nothrow copy constructible.");

    static_assert(std::is_trivially_move_constructible_v<BufT>,
                  "BufT must be trivially move constructible.");
    static_assert(std::is_nothrow_move_constructible_v<BufT>,
                  "BufT must be nothrow move constructible.");

    static_assert(std::is_trivially_assignable_v<BufT, BufT>,
                  "BufT must be trivially assignable to BufT.");
    static_assert(std::is_nothrow_assignable_v<BufT, BufT>,
                  "BufT must be nothrow assignable to BufT.");

    static_assert(std::is_trivially_copy_assignable_v<BufT>,
                  "BufT must be trivially copy assignable.");
    static_assert(std::is_nothrow_copy_assignable_v<BufT>,
                  "BufT must be nothrow copy assignable.");

    static_assert(std::is_trivially_move_assignable_v<BufT>,
                  "BufT must be trivially move assignable.");
    static_assert(std::is_nothrow_move_assignable_v<BufT>,
                  "BufT must be nothrow move assignable.");

    static_assert(std::is_trivially_destructible_v<BufT>,
                  "BufT must be trivially destructible.");
    static_assert(std::is_nothrow_destructible_v<BufT>,
                  "BufT must be nothrow destructible.");

    static_assert(not std::has_virtual_destructor_v<BufT>,
                  "BufT must have no virtual destructor.");
}

namespace test_utils
{
template <typename CallableT>
int execute(CallableT &&aArg) noexcept
{
    const auto r = leaf::try_handle_all(
        aArg,
        [](leaf::match<buffer::error, buffer::error::null_data_and_zero_size>)
        { return to_underlying(buffer::error::null_data_and_zero_size); },
        [](leaf::match<buffer::error, buffer::error::null_data_pointer>)
        { return to_underlying(buffer::error::null_data_pointer); },
        [](leaf::match<buffer::error, buffer::error::zero_size>)
        { return to_underlying(buffer::error::zero_size); },
        [](leaf::match<buffer::error, buffer::error::invalid_index>)
        { return to_underlying(buffer::error::invalid_index); },
        [](leaf::error_info const &unmatched)
        {
            std::cerr << "Unknown failure detected" << std::endl
                      << "Cryptic diagnostic information follows" << std::endl
                      << unmatched;
            return to_underlying(buffer::error::unknown);
        });
    return r;
}
}  // namespace test_utils

#ifdef BOOST_LEAF_NO_EXCEPTIONS

namespace boost
{
[[noreturn]] void throw_exception(std::exception const &e)
{
    std::cerr
        << "Terminating due to a C++ exception under BOOST_LEAF_NO_EXCEPTIONS: "
        << e.what();
    std::terminate();
}

struct source_location;
[[noreturn]] void throw_exception(std::exception const &e,
                                  boost::source_location const &)
{
    throw_exception(e);
}
}  // namespace boost

#endif

TEST(BufferViewConst, ConstructorTest1)
{
    using DataT = char;
    using BufDataT = char;
    constexpr std::size_t kSize = 10;
    std::array<DataT, kSize> array{};
    using Buf = buffer::buffer_view_const<BufDataT>;
    result<Buf> buf = buffer::make_bv_const(array.data(), array.size());
    static_assert(type_name::type_name<Buf::value_type>() == "char"sv,
                  "Invalid value_type.");
    static_assert(type_name::type_name<Buf::pointer>() == "const char *"sv,
                  "Invalid value_type.");
    ASSERT_EQ(buf.value().size(), kSize);
    ASSERT_EQ(buf.value().data(), array.data());
}

TEST(BufferViewConst, ConstructorTest2)
{
    using DataT = char;
    using BufDataT = void;
    constexpr std::size_t kSize = 10;
    const std::array<DataT, kSize> array{};
    using Buf = buffer::buffer_view_const<BufDataT>;
    result<Buf> buf = buffer::make_bv_const(
        static_cast<void const *>(array.data()), array.size());
    static_assert(type_name::type_name<Buf::value_type>() == "void"sv,
                  "Invalid value_type.");
    static_assert(type_name::type_name<Buf::pointer>() == "const void *"sv,
                  "Invalid value_type.");
    ASSERT_EQ(buf.value().size(), kSize);
    ASSERT_EQ(buf.value().data(), array.data());
}

TEST(BufferViewConst, ConstructorTest3)
{
    using DataT = int16_t;
    using BufDataT = int16_t;
    constexpr std::size_t kSize = 10;
    const std::array<DataT, kSize> array{};
    using Buf = buffer::buffer_view_const<BufDataT>;
    result<Buf> buf = buffer::make_bv_const(array.data(), array.size());
    static_assert(type_name::type_name<Buf::value_type>() == "short"sv,
                  "Invalid value_type.");
    static_assert(type_name::type_name<Buf::pointer>() == "const short *"sv,
                  "Invalid value_type.");
    ASSERT_EQ(buf.value().size(), kSize);
    ASSERT_EQ(buf.value().data(), array.data());
}

TEST(BufferViewConst, ConstructFromCArrayTest1)
{
    using DataT = unsigned char;
    using BufDataT = unsigned char;
    constexpr std::size_t kSize = 10;
    const DataT array[kSize]{};
    using Buf = buffer::buffer_view_const<BufDataT>;
    result<Buf> buf = buffer::make_bv_const(array);
    static_assert(type_name::type_name<Buf::value_type>() == "unsigned char"sv,
                  "Invalid value_type.");
    static_assert(
        type_name::type_name<Buf::pointer>() == "const unsigned char *"sv,
        "Invalid value_type.");
    ASSERT_EQ(buf.value().size(), kSize);
    ASSERT_EQ(buf.value().data(), array);
}

TEST(BufferViewConst, ConstructFromSTDArrayTest)
{
    using DataT = unsigned char;
    using BufDataT = unsigned char;
    constexpr std::size_t kSize = 10;
    const std::array<DataT, kSize> array{};
    using BufT = buffer::buffer_view_const<BufDataT>;
    result<BufT> buf = buffer::make_bv_const(array);
    static_assert(type_name::type_name<BufT::value_type>() == "unsigned char"sv,
                  "Invalid value_type.");
    static_assert(
        type_name::type_name<BufT::pointer>() == "const unsigned char *"sv,
        "Invalid value_type.");
    ASSERT_EQ(buf.value().size(), kSize);
    ASSERT_EQ(buf.value().data(), array.data());
}

TEST(BufferView, ConstructorTest1)
{
    using DataT = int16_t;
    using BufDataT = int16_t;
    constexpr std::size_t kSize = 10;
    std::array<DataT, kSize> array{};
    using Buf = buffer::buffer_view<BufDataT>;
    result<Buf> buf = buffer::make_bv(array.data(), array.size());
    static_assert(type_name::type_name<Buf::value_type>() == "short"sv,
                  "Invalid value_type.");
    static_assert(type_name::type_name<Buf::pointer>() == "short *"sv,
                  "Invalid value_type.");
    ASSERT_EQ(buf.value().size(), kSize);
    ASSERT_EQ(buf.value().data(), array.data());
}

TEST(BufferView, ConstructFromCArrayTest1)
{
    using DataT = unsigned char;
    using BufDataT = unsigned char;
    constexpr std::size_t kSize = 10;
    DataT array[kSize]{};
    using Buf = buffer::buffer_view<BufDataT>;
    result<Buf> buf = buffer::make_bv(array);
    static_assert(type_name::type_name<Buf::value_type>() == "unsigned char"sv,
                  "Invalid value_type.");
    static_assert(type_name::type_name<Buf::pointer>() == "unsigned char *"sv,
                  "Invalid value_type.");
    ASSERT_EQ(buf.value().size(), kSize);
    ASSERT_EQ(buf.value().data(), array);
}

TEST(BufferView, ConstructFromSTDArrayTest)
{
    using DataT = unsigned char;
    using BufDataT = unsigned char;
    constexpr std::size_t kSize = 10;
    std::array<DataT, kSize> array{};
    using BufT = buffer::buffer_view<BufDataT>;
    result<BufT> buf = buffer::make_bv(array);
    static_assert(type_name::type_name<BufT::value_type>() == "unsigned char"sv,
                  "Invalid value_type.");
    static_assert(type_name::type_name<BufT::pointer>() == "unsigned char *"sv,
                  "Invalid value_type.");
    ASSERT_EQ(buf.value().size(), kSize);
    ASSERT_EQ(buf.value().data(), array.data());
}

TEST(BufferView, ValidateTypeProperties)
{
    using DataT = unsigned char;
    using BufT = buffer::buffer_view<DataT>;
    validateGeneralBufferTypeProperties<BufT>();
}

TEST(BufferViewConst, ValidateTypeProperties)
{
    using DataT = unsigned char;
    using BufT = buffer::buffer_view<DataT>;
    using ConstBufT = buffer::buffer_view_const<DataT>;
    validateGeneralBufferTypeProperties<BufT>();
    static_assert(std::is_convertible_v<BufT, ConstBufT>,
                  "BufT must be convertible to ConstBufT.");
}

TEST(BufferView, ConstructFromInvalidDataAndSize)
{
    const auto r = test_utils::execute(
        [&]() -> result<int>
        {
            using DataT = unsigned char;
            using DataPtrT = DataT *;
            DataPtrT dataPtr = nullptr;
            constexpr std::size_t kSize = 0;
            using BufT = buffer::buffer_view<DataT>;
            result<BufT> buf = buffer::make_bv(dataPtr, kSize);
            return buf.has_error() ? buf.error() : result<int>{0};
        });

    ASSERT_EQ(r, to_underlying(buffer::error::null_data_and_zero_size));
}

TEST(BufferView, ConstructFromNullDataPointer)
{
    const auto r = test_utils::execute(
        [&]() -> result<int>
        {
            using DataT = unsigned char;
            using DataPtrT = DataT *;
            DataPtrT dataPtr = nullptr;
            constexpr std::size_t kSize = 10;
            using BufT = buffer::buffer_view<DataT>;
            result<BufT> buf = buffer::make_bv(dataPtr, kSize);
            return buf.has_error() ? buf.error() : result<int>{0};
        });

    ASSERT_EQ(r, to_underlying(buffer::error::null_data_pointer));
}

TEST(BufferView, ConstructFromZeroSize)
{
    const auto r = test_utils::execute(
        [&]() -> result<int>
        {
            using DataT = unsigned char;
            constexpr std::size_t kSize = 10;
            DataT rawPtr[kSize]{};
            using BufT = buffer::buffer_view<DataT>;
            result<BufT> buf = buffer::make_bv(rawPtr, 0);
            return buf.has_error() ? buf.error() : result<int>{0};
        });

    ASSERT_EQ(r, to_underlying(buffer::error::zero_size));
}

TEST(BufferView, AccessByInvalidIndex)
{
    const auto r = test_utils::execute(
        [&]() -> result<int>
        {
            using DataT = unsigned char;
            constexpr std::size_t kSize = 10;
            DataT rawBuf[kSize]{};
            using BufT = buffer::buffer_view<DataT>;
            result<BufT> buf = buffer::make_bv(rawBuf);
            if (buf)
            {
                auto value = buf.value()[kSize];
                return value.has_error() ? value.error() : result<int>{};
            }
            return buf.error();
        });

    ASSERT_EQ(r, to_underlying(buffer::error::invalid_index));
}
