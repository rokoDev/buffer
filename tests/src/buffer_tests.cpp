#include <buffer/buffer.h>
#include <gtest/gtest.h>

#include <array>
#include <limits>
#include <type_traits>

using namespace std::literals::string_view_literals;

using buf_error = buffer::error;

using Pos = buffer::bit_pos;
using NBits = buffer::n_bits;
using NBytes = buffer::n_bytes;
using buffer_view_const = buffer::buffer_view_const<uint8_t>;
using buffer_view = buffer::buffer_view<uint8_t>;

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

template <typename DataT, std::size_t MaxSize>
class BufTest : public ::testing::Test
{
   public:
    BufTest() = default;
    BufTest(const BufTest &) = delete;
    BufTest &operator=(const BufTest &) = delete;
    BufTest(BufTest &&) = delete;
    BufTest &operator=(BufTest &&) = delete;

   protected:
    DataT rawBuf_[MaxSize]{};
};

using BufMaxLen64 = BufTest<uint8_t, 64>;
using BVConst = BufTest<uint8_t, 64>;

TEST(Comparison, PosPosEQ)
{
    constexpr Pos p1(NBits(80));
    constexpr Pos p2(NBytes(10));
    static_assert(p1 == p2, "p1 must be equal p2");
}

TEST(Comparison, PosPosNE)
{
    constexpr Pos p1(NBits(10));
    constexpr Pos p2(NBytes(10));
    static_assert(p1 != p2, "p1 must not be equal p2");
}

TEST(Comparison, PosPosLT)
{
    constexpr Pos p1(NBits(10));
    constexpr Pos p2(NBytes(2));
    static_assert(p1 < p2, "p1 must be less than p2");
}

TEST(Comparison, PosPosGT)
{
    constexpr Pos p1(NBits(10));
    constexpr Pos p2(NBytes(2));
    static_assert(p2 > p1, "p2 must be greater than p1");
}

TEST(Comparison, PosPosLE)
{
    constexpr Pos p1(NBits(16));
    constexpr Pos p2(NBytes(2));
    static_assert(p1 <= p2, "p1 must be less than or equal to p2");
}

TEST(Comparison, PosPosGE)
{
    constexpr Pos p1(NBits(16));
    constexpr Pos p2(NBytes(2));
    static_assert(p1 >= p2, "p1 must be greater than or equal to p2");
}

TEST(Comparison, PosBufSizeNE)
{
    constexpr Pos p1(NBits(10));
    constexpr Pos p2(NBytes(10));
    static_assert(p1 != p2, "p1 must not be equal p2");
}

TEST(Comparison, PosBufSizeLT)
{
    constexpr Pos p1(NBits(10));
    constexpr Pos p2(NBytes(2));
    static_assert(p1 < p2, "p1 must be less than p2");
}

TEST(Comparison, PosBufSizeGT)
{
    constexpr Pos p1(NBits(10));
    constexpr Pos p2(NBytes(2));
    static_assert(p2 > p1, "p2 must be greater than p1");
}

TEST(Comparison, PosBufSizeLE)
{
    constexpr Pos p1(NBits(16));
    constexpr Pos p2(NBytes(2));
    static_assert(p1 <= p2, "p1 must be less than or equal to p2");
}

TEST(Comparison, PosBufSizeGE)
{
    constexpr Pos p1(NBits(16));
    constexpr Pos p2(NBytes(2));
    static_assert(p1 >= p2, "p1 must be greater than or equal to p2");
}

TEST(BitPosSpecial, ByteIndex)
{
    constexpr Pos p(NBits(10));
    static_assert(p.byteIndex() == 1, "Invalid byte index.");
    static_assert(p.bytesUsed() == 2, "bytesUsed returned invalid value.");
}

TEST(BitPosSpecial, BitOffset)
{
    constexpr Pos p(NBits(11));
    static_assert(p.bitOffset() == 3, "Invalid bit offset.");
}

TEST(BitPosSpecial, Reset)
{
    constexpr Pos zeroPos(0);
    Pos p(NBits(11));
    p.reset();
    ASSERT_EQ(p, zeroPos);
}

TEST(BVConst, ConstructFromConstArray)
{
    constexpr std::size_t kSize = 10;
    const uint8_t someBuf[kSize]{};
    buffer_view_const buf(someBuf);
    ASSERT_EQ(buf.size(), NBytes(kSize));
}

TEST(BVConst, ConstructFromArray)
{
    constexpr std::size_t kSize = 10;
    uint8_t someBuf[kSize]{};
    buffer_view_const buf(someBuf);
    ASSERT_EQ(buf.size(), NBytes(kSize));
}

TEST(BV, ConstructFromArray)
{
    constexpr std::size_t kSize = 10;
    uint8_t someBuf[kSize]{};
    buffer_view buf(someBuf);
    ASSERT_EQ(buf.size(), NBytes(kSize));
}

TEST(BV, AccessViaSquareBrackets)
{
    using value_type = buffer_view::value_type;
    using pointer = buffer_view::pointer;

    constexpr std::size_t kSize = 10;
    uint8_t someBuf[kSize]{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    buffer_view buf(someBuf);
    for (std::size_t i = 0; i < kSize; ++i)
    {
        value_type val = buf[NBytes{i}];
        ASSERT_EQ(val, i + 1);

        pointer p = buf[NBytes{i}];
        ASSERT_EQ(p, someBuf + i);
    }
}

TEST(BV, AssignViaSquareBrackets)
{
    constexpr std::size_t kSize = 10;
    uint8_t someBuf[kSize]{};
    buffer_view buf(someBuf);
    for (std::size_t i = 0; i < kSize; ++i)
    {
        buf[NBytes{i}] = i;
    }

    for (std::size_t i = 0; i < kSize; ++i)
    {
        ASSERT_EQ(buf[NBytes{i}], i);
    }
}

TEST(BV, ConstAccessViaSquareBrackets)
{
    using value_type = buffer_view_const::value_type;
    using pointer = buffer_view_const::pointer;

    constexpr std::size_t kSize = 10;
    uint8_t someBuf[kSize]{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    buffer_view_const buf(someBuf);
    for (std::size_t i = 0; i < kSize; ++i)
    {
        value_type val = buf[NBytes{i}];
        ASSERT_EQ(val, i + 1);

        pointer p = buf[NBytes{i}];
        ASSERT_EQ(p, someBuf + i);
    }
}
