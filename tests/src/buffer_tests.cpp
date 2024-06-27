#include <buffer/buffer.h>
#include <gtest/gtest.h>

#include <array>
#include <limits>
#include <type_traits>

using namespace std::literals::string_view_literals;

namespace leaf = boost::leaf;
using buf_error = buffer::error;

template <class T>
using result = leaf::result<T>;

using Pos = buffer::bit_pos;
using NBits = buffer::n_bits;
using NBytes = buffer::n_bytes;
using simple_bv_const = buffer::simple_buffer_view_const<uint8_t>;
using simple_bv = buffer::simple_buffer_view<uint8_t>;

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
        [](leaf::match<buf_error, buf_error::null_data_and_zero_size>)
        { return to_underlying(buf_error::null_data_and_zero_size); },
        [](leaf::match<buf_error, buf_error::null_data>)
        { return to_underlying(buf_error::null_data); },
        [](leaf::match<buf_error, buf_error::zero_size>)
        { return to_underlying(buf_error::zero_size); },
        [](leaf::match<buf_error, buf_error::invalid_index>)
        { return to_underlying(buf_error::invalid_index); },
        [](leaf::error_info const &unmatched)
        {
            std::cerr << "Unknown failure detected" << std::endl
                      << "Cryptic diagnostic information follows" << std::endl
                      << unmatched;
            return std::numeric_limits<int>::max();
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

template <typename DataT, std::size_t MaxSize>
class BufTest : public ::testing::Test
{
   public:
    using Buf = buffer::buffer_view<DataT>;
    using BufConst = buffer::buffer_view_const<DataT>;

    BufTest() = default;
    BufTest(const BufTest &) = delete;
    BufTest &operator=(const BufTest &) = delete;
    BufTest(BufTest &&) = delete;
    BufTest &operator=(BufTest &&) = delete;

    Buf make_buf(NBytes aSize) noexcept
    {
        assert((aSize <= MaxSize) && "aSize is too big.");
        auto r = buffer::make_bv(rawBuf_, aSize);
        assert(r.has_value() && "fatal error while creating buffer.");
        Buf buf = r.value();
        return buf;
    }

    BufConst make_buf_const(NBytes aSize) noexcept
    {
        assert((aSize <= MaxSize) && "aSize is too big.");
        auto r = buffer::make_bv_const(rawBuf_, aSize);
        assert(r.has_value() && "fatal error while creating buffer.");
        BufConst buf = r.value();
        return buf;
    }

   protected:
    DataT rawBuf_[MaxSize]{};
};

using BufMaxLen64 = BufTest<uint8_t, 64>;
using SimpleBVConst = BufTest<uint8_t, 64>;

TEST(BufferViewConst, ConstructorFromPointer)
{
    using DataT = char;
    using BufDataT = char;
    constexpr std::size_t kSize = 10;
    std::array<DataT, kSize> array{};
    using Buf = buffer::buffer_view_const<BufDataT>;
    result<Buf> buf = buffer::make_bv_const(array.data(), NBytes(array.size()));
    ASSERT_EQ(buf.value().size(), NBytes(kSize));
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
    ASSERT_EQ(buf.value().size(), NBytes(kSize));
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
    ASSERT_EQ(buf.value().size(), NBytes(kSize));
    ASSERT_EQ(buf.value().data(), array.data());
}

TEST(BufferView, ConstructFromPointer)
{
    using DataT = char;
    using BufDataT = char;
    constexpr std::size_t kSize = 10;
    const std::array<DataT, kSize> array{};
    using Buf = buffer::buffer_view_const<BufDataT>;
    result<Buf> buf = buffer::make_bv_const(array.data(), NBytes(array.size()));
    ASSERT_EQ(buf.value().size(), NBytes(kSize));
    ASSERT_EQ(buf.value().bit_size(), kSize * CHAR_BIT);
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
    ASSERT_EQ(buf.value().size(), NBytes(kSize));
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
    ASSERT_EQ(buf.value().size(), NBytes(kSize));
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
            result<BufT> buf = buffer::make_bv(dataPtr, NBytes(kSize));
            return buf.has_error() ? buf.error() : result<int>{0};
        });

    ASSERT_EQ(r, to_underlying(buf_error::null_data_and_zero_size));
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
            result<BufT> buf = buffer::make_bv(dataPtr, NBytes(kSize));
            return buf.has_error() ? buf.error() : result<int>{0};
        });

    ASSERT_EQ(r, to_underlying(buf_error::null_data));
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
            result<BufT> buf = buffer::make_bv(rawPtr, NBytes(0));
            return buf.has_error() ? buf.error() : result<int>{0};
        });

    ASSERT_EQ(r, to_underlying(buf_error::zero_size));
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
                auto value = buf.value()[NBytes(kSize)];
                return value.has_error() ? value.error() : result<int>{};
            }
            return buf.error();
        });

    ASSERT_EQ(r, to_underlying(buf_error::invalid_index));
}

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

TEST_F(BufMaxLen64, PosBufSizeEQ)
{
    constexpr Pos p1(NBits(80));
    Buf buf = make_buf(NBytes(10));

    ASSERT_EQ(p1, Pos(buf.size()));
    static_assert(p1.bytesUsed() == 10, "bytesUsed returned invalid value.");
}

TEST_F(BufMaxLen64, PosBufBitSizeEQ)
{
    constexpr Pos p1(NBits(80));
    Buf buf = make_buf(NBytes(10));

    ASSERT_EQ(p1, buf.bit_size());
}

TEST_F(BufMaxLen64, PosBufBitSizeEQ2)
{
    constexpr Pos p1(15);
    Buf buf = make_buf(NBytes(2));

    ASSERT_EQ(p1 + 1, buf.bit_size());
    static_assert(p1.bytesUsed() == 2, "bytesUsed returned invalid value.");
}

TEST_F(BufMaxLen64, PosBufSizeLT)
{
    constexpr Pos p1(NBits(80));
    Buf buf = make_buf(NBytes(11));
    ASSERT_LT(p1, Pos(buf.size()));
}

TEST_F(BufMaxLen64, PosBufBitSizeLT)
{
    constexpr Pos p1(NBits(80));
    Buf buf = make_buf(NBytes(11));
    ASSERT_LT(p1, buf.bit_size());
}

TEST(SimpleBVConst, ConstructFromConstArray)
{
    constexpr std::size_t kSize = 10;
    const uint8_t someBuf[kSize]{};
    simple_bv_const buf(someBuf);
    ASSERT_EQ(buf.size(), NBytes(kSize));
}

TEST(SimpleBVConst, ConstructFromArray)
{
    constexpr std::size_t kSize = 10;
    uint8_t someBuf[kSize]{};
    simple_bv_const buf(someBuf);
    ASSERT_EQ(buf.size(), NBytes(kSize));
}

TEST(SimpleBV, ConstructFromArray)
{
    constexpr std::size_t kSize = 10;
    uint8_t someBuf[kSize]{};
    simple_bv buf(someBuf);
    ASSERT_EQ(buf.size(), NBytes(kSize));
}

TEST(SimpleBV, AccessViaSquareBrackets)
{
    using value_type = simple_bv::value_type;
    using pointer = simple_bv::pointer;

    constexpr std::size_t kSize = 10;
    uint8_t someBuf[kSize]{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    simple_bv buf(someBuf);
    for (std::size_t i = 0; i < kSize; ++i)
    {
        value_type val = buf[NBytes{i}];
        ASSERT_EQ(val, i + 1);

        pointer p = buf[NBytes{i}];
        ASSERT_EQ(p, someBuf + i);
    }
}

TEST(SimpleBV, AssignViaSquareBrackets)
{
    constexpr std::size_t kSize = 10;
    uint8_t someBuf[kSize]{};
    simple_bv buf(someBuf);
    for (std::size_t i = 0; i < kSize; ++i)
    {
        buf[NBytes{i}] = i;
    }

    for (std::size_t i = 0; i < kSize; ++i)
    {
        ASSERT_EQ(buf[NBytes{i}], i);
    }
}

TEST(SimpleBV, ConstAccessViaSquareBrackets)
{
    using value_type = simple_bv_const::value_type;
    using pointer = simple_bv_const::pointer;

    constexpr std::size_t kSize = 10;
    uint8_t someBuf[kSize]{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    simple_bv_const buf(someBuf);
    for (std::size_t i = 0; i < kSize; ++i)
    {
        value_type val = buf[NBytes{i}];
        ASSERT_EQ(val, i + 1);

        pointer p = buf[NBytes{i}];
        ASSERT_EQ(p, someBuf + i);
    }
}
