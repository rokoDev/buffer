#include <buffer/buffer.h>
#include <gtest/gtest.h>
#include <type_name/type_name.h>

#include <array>

using namespace std::literals::string_view_literals;

TEST(BufferViewConst, ConstructorTest1)
{
    using DataT = char;
    using BufDataT = char;
    constexpr std::size_t kSize = 10;
    std::array<DataT, kSize> array{};
    using Buf = buffer::buffer_view_const<BufDataT>;
    auto buf = Buf(array.data(), array.size());
    static_assert(type_name::type_name<Buf::value_type>() == "char"sv,
                  "Invalid value_type.");
    static_assert(type_name::type_name<Buf::pointer>() == "const char *"sv,
                  "Invalid value_type.");
    ASSERT_EQ(buf.size(), kSize);
    ASSERT_EQ(buf.data(), array.data());
}

TEST(BufferViewConst, ConstructorTest2)
{
    using DataT = char;
    using BufDataT = void;
    constexpr std::size_t kSize = 10;
    const std::array<DataT, kSize> array{};
    using Buf = buffer::buffer_view_const<BufDataT>;
    auto buf = Buf(array.data(), array.size());
    static_assert(type_name::type_name<Buf::value_type>() == "void"sv,
                  "Invalid value_type.");
    static_assert(type_name::type_name<Buf::pointer>() == "const void *"sv,
                  "Invalid value_type.");
    ASSERT_EQ(buf.size(), kSize);
    ASSERT_EQ(buf.data(), array.data());
}

TEST(BufferViewConst, ConstructorTest3)
{
    using DataT = int16_t;
    using BufDataT = int16_t;
    constexpr std::size_t kSize = 10;
    const std::array<DataT, kSize> array{};
    using Buf = buffer::buffer_view_const<BufDataT>;
    auto buf = Buf(array.data(), array.size());
    static_assert(type_name::type_name<Buf::value_type>() == "short"sv,
                  "Invalid value_type.");
    static_assert(type_name::type_name<Buf::pointer>() == "const short *"sv,
                  "Invalid value_type.");
    ASSERT_EQ(buf.size(), kSize);
    ASSERT_EQ(buf.data(), array.data());
}

TEST(BufferViewConst, ConstructorWithCArrayTest1)
{
    using DataT = unsigned char;
    using BufDataT = unsigned char;
    constexpr std::size_t kSize = 10;
    DataT array[kSize]{};
    using Buf = buffer::buffer_view_const<BufDataT>;
    auto buf = Buf(array);
    static_assert(type_name::type_name<Buf::value_type>() == "unsigned char"sv,
                  "Invalid value_type.");
    static_assert(
        type_name::type_name<Buf::pointer>() == "const unsigned char *"sv,
        "Invalid value_type.");
    ASSERT_EQ(buf.size(), kSize);
    ASSERT_EQ(buf.data(), array);
}
