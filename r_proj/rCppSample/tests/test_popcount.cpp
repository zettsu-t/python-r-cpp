#include "test_popcount.h"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <gtest/gtest.h>
#include <limits>
#define R_INTERFACE_PTRS
#include <Rembedded.h>
#include <Rinterface.h>

#ifndef UNIT_TEST_CPP
class TestMatrix : public ::testing::Test {};

TEST_F(TestMatrix, Shape) {
    using Element = double;
    constexpr int nrow = 63;
    constexpr int ncol = 3;
    Rcpp::NumericMatrix m(nrow, ncol);

    Element &left_top = m(0, 0);
    Element &right = m(0, 1);
    Element &down = m(1, 0);

    const auto ptrdiff_right = std::addressof(right) - std::addressof(left_top);
    const auto ptrdiff_down = std::addressof(down) - std::addressof(left_top);
    ASSERT_LE(nrow, ptrdiff_right);
    ASSERT_EQ(1, ptrdiff_down);

    const auto addrdiff_right =
        reinterpret_cast<uintptr_t>(std::addressof(right)) -
        reinterpret_cast<uintptr_t>(std::addressof(left_top));
    const auto addrdiff_down =
        reinterpret_cast<uintptr_t>(std::addressof(down)) -
        reinterpret_cast<uintptr_t>(std::addressof(left_top));
    ASSERT_LE(sizeof(Element) * nrow, addrdiff_right);
    ASSERT_EQ(sizeof(Element), addrdiff_down);
}
#endif // UNIT_TEST_CPP

class TestNaInteger : public ::testing::Test {};

TEST_F(TestNaInteger, IsNaInteger) {
    constexpr uint8_t zero_uint8 = 0;
    constexpr auto max_uint8 = std::numeric_limits<uint8_t>::max();
    constexpr int zero_int = 0;
    constexpr auto max_int = std::numeric_limits<int>::max();
    constexpr auto min_int = std::numeric_limits<int>::min();
#ifdef UNIT_TEST_CPP
    EXPECT_FALSE(is_na_integer(zero_uint8));
    EXPECT_FALSE(is_na_integer(max_uint8));
    EXPECT_FALSE(is_na_integer(zero_int));
    EXPECT_FALSE(is_na_integer(max_int));
    EXPECT_TRUE(is_na_integer(min_int));
#else // UNIT_TEST_CPP
    EXPECT_FALSE(is_na_integer<rCppSample::RawVector>(zero_uint8));
    EXPECT_FALSE(is_na_integer<rCppSample::RawVector>(max_uint8));
    EXPECT_FALSE(is_na_integer<rCppSample::IntegerVector>(zero_int));
    EXPECT_FALSE(is_na_integer<rCppSample::IntegerVector>(max_int));
    EXPECT_TRUE(is_na_integer<rCppSample::IntegerVector>(min_int));
#endif // UNIT_TEST_CPP
}

TEST_F(TestNaInteger, GetNaIntValue) {
#ifdef UNIT_TEST_CPP
    const auto expected = rCppSample::NaInteger;
#else // UNIT_TEST_CPP
    const auto expected = NA_INTEGER;
#endif // UNIT_TEST_CPP
    const auto actual = get_na_int_value();
    EXPECT_EQ(expected, actual);
}

namespace {
template <typename T, typename U>
bool are_equal(const T &expected, const U &actual) {
    // Rcpp::Vector has no == operators.
    return ((expected.size() == actual.size()) &&
            std::equal(expected.begin(), expected.end(), actual.begin(),
                       actual.end()));
}

class RcodeFeeder {
  public:
    using BufElement = unsigned char;
    using BufLen = int;
    using ReturnCode = int;
    explicit RcodeFeeder(const std::string &r_code) : r_code_(r_code) {}
    virtual ~RcodeFeeder(void) = default;
    ReturnCode feed_r_code(BufElement *buf, BufLen buflen) {
        if (!buf || !buflen) {
            return Return_Code;
        }
        *buf = '\0';

        const auto rcode_size = static_cast<decltype(buflen)>(r_code_.size());
        const auto suffix_size =
            static_cast<decltype(buflen)>(r_suffix_.size());
        // Include trailing NUL
        const auto total_size = rcode_size + suffix_size + 1;
        if (buflen < total_size) {
            return Return_Code;
        }

        std::string code{r_code_};
        code += r_suffix_;
        std::strncpy(reinterpret_cast<char *>(buf), code.c_str(),
                     code.size() + 1);
        return Return_Code;
    }

    static constexpr ReturnCode Return_Code{1};

  private:
    std::string r_code_;
    static const std::string r_suffix_;
};

const std::string RcodeFeeder::r_suffix_{"\n"};

template <RcodeFeeder::BufLen N>
auto call_feed_rcode(RcodeFeeder &feeder, RcodeFeeder::BufElement (&buf)[N]) {
    return feeder.feed_r_code(buf, N);
}
} // namespace

class TestHelper : public ::testing::Test {};

TEST_F(TestHelper, AreEqual) {
    using Numbers1 = std::vector<int>;
    using Numbers2 = std::vector<long long>;
    const Numbers1 empty1{};
    const Numbers2 empty2{};
    const Numbers1 one1{1};
    const Numbers2 one2{2};
    const Numbers1 many1a{2, 3, 5, 7};
    const Numbers1 many1b{2, 3, 5, 7};
    const Numbers2 many2{2, 3, 6, 18};
    EXPECT_TRUE(are_equal(empty1, empty2));
    EXPECT_TRUE(are_equal(one1, one1));
    EXPECT_TRUE(are_equal(many1a, many1b));
    EXPECT_FALSE(are_equal(one1, one2));
    EXPECT_FALSE(are_equal(one1, empty1));
    EXPECT_FALSE(are_equal(many1a, many2));
}

class TestFeedRcode : public ::testing::Test {};

TEST_F(TestFeedRcode, NullEmptyBuffer) {
    const std::string r_code{"print(1:5)"};
    RcodeFeeder feeder(r_code);
    EXPECT_EQ(feeder.Return_Code, feeder.feed_r_code(nullptr, 0));

    const char expected = 'A';
    RcodeFeeder::BufElement buf[2]{expected, '\n'};
    EXPECT_EQ(feeder.Return_Code, feeder.feed_r_code(buf, 0));
    ASSERT_EQ(expected, *buf);
}

TEST_F(TestFeedRcode, ShortBuffer) {
    const std::string r_code{"a"};
    RcodeFeeder feeder1(r_code);
    RcodeFeeder::BufElement buf1[1]{'\n'};
    EXPECT_EQ(feeder1.Return_Code, call_feed_rcode(feeder1, buf1));
    ASSERT_FALSE(*buf1);

    RcodeFeeder feeder2(r_code);
    RcodeFeeder::BufElement buf2[]{'a', 'b'};
    EXPECT_EQ(feeder2.Return_Code, call_feed_rcode(feeder2, buf2));
    ASSERT_FALSE(*buf2);

    RcodeFeeder feeder3(r_code);
    RcodeFeeder::BufElement buf3[3]{};
    EXPECT_EQ(feeder3.Return_Code, call_feed_rcode(feeder3, buf3));
    EXPECT_EQ(r_code.at(0), buf3[0]);
}

TEST_F(TestFeedRcode, NearFullSize) {
    const std::string r_code{"library(packageName)"};
    const auto min_size = static_cast<RcodeFeeder::BufLen>(r_code.size() - 1);
    // trailing "\n\0"
    const auto full_size = static_cast<RcodeFeeder::BufLen>(r_code.size() + 2);
    const auto buf_size = full_size + 2;

    for (RcodeFeeder::BufLen buflen{min_size}; buflen < full_size; ++buflen) {
        using LineBuffer = std::vector<RcodeFeeder::BufElement>;
        LineBuffer line_buffer(static_cast<LineBuffer::size_type>(buf_size));
        std::fill(line_buffer.begin(), line_buffer.end(), 0);

        RcodeFeeder feeder(r_code);
        auto buf = line_buffer.data();
        EXPECT_EQ(feeder.Return_Code, feeder.feed_r_code(buf, buflen));
        ASSERT_FALSE(line_buffer.at(0));
    }
}

TEST_F(TestFeedRcode, FullSize) {
    const std::string r_code{"library(packageName)"};
    // trailing "\n\0"
    const auto full_size = static_cast<RcodeFeeder::BufLen>(r_code.size() + 2);
    const auto buf_size = full_size + 2;
    std::string expected{r_code};
    expected += "\n";

    for (RcodeFeeder::BufLen buflen{full_size}; buflen <= buf_size; ++buflen) {
        using LineBuffer = std::vector<RcodeFeeder::BufElement>;
        LineBuffer line_buffer(static_cast<LineBuffer::size_type>(buf_size));
        std::fill(line_buffer.begin(), line_buffer.end(), 0);

        RcodeFeeder feeder(r_code);
        auto buf = line_buffer.data();
        EXPECT_EQ(feeder.Return_Code, feeder.feed_r_code(buf, buflen));
        const std::string actual(reinterpret_cast<const char *>(buf));
        ASSERT_EQ(expected, actual);
    }
}

class TestStrnpy : public ::testing::Test {};

TEST_F(TestStrnpy, NullBuffer) {
    constexpr size_t max_size = 8;
    const char cstr[max_size]{"0123456"};
    constexpr size_t buf_size = max_size + 2;

    for (size_t size = 0; size < buf_size; ++size) {
        std::vector<char> buffer(buf_size);
        std::fill(buffer.begin(), buffer.end(), 'a');
        std::strncpy(buffer.data(), cstr, size);
        if (size > 0) {
            if (size >= max_size) {
                ASSERT_FALSE(buffer.at(size - 1));
                ASSERT_FALSE(buffer.at(max_size - 1));
                ASSERT_FALSE(std::strcmp(cstr, buffer.data()));
            } else {
                EXPECT_TRUE(buffer.at(size - 1));
            }
        }
    }
}

class TestPopcount : public ::testing::Test {};

TEST_F(TestPopcount, Size) {
    // Empty arrays are acceptable
    using VectorType = rCppSample::RawVector;
    using SizeType = typename VectorSize<VectorType>::size_type;
    for (SizeType array_size{0}; array_size < 3; ++array_size) {
        const VectorType arg(array_size, 0);
        const auto actual = popcount_cpp_raw(arg);
        ASSERT_EQ(arg.size(), actual.size());
    }
}

TEST_F(TestPopcount, RawValues) {
    // Check 0..255 uint8 values
    const rCppSample::RawVector arg{0, 1, 2, 3, 6, 7, 254, 255};
    const rCppSample::IntegerVector expected{0, 1, 1, 2, 2, 3, 7, 8};
    const auto actual = popcount_cpp_raw(arg);
    EXPECT_TRUE(are_equal(expected, actual));
}

TEST_F(TestPopcount, IntegerValues) {
    // Check some non-negative int32 values
    using VectorType = rCppSample::IntegerVector;
    const VectorType arg{0,          1,          0xfe,     0xff,
                         0x100,      0xffff,     0x10000,  0x7ffffffe,
                         0x7fffffff, 0x70f0f0f0, 0xf0f0f0f};
    const VectorType expected{0, 1, 7, 8, 1, 16, 1, 30, 31, 15, 16};
    const auto actual = popcount_cpp_integer(arg);
    EXPECT_TRUE(are_equal(expected, actual));
}

TEST_F(TestPopcount, NAs) {
    // Check NA values
    using VectorType = rCppSample::IntegerVector;
    const VectorType arg{2, rCppSample::NaInteger, 14, rCppSample::NaInteger,
                         62};
    const VectorType expected{1, rCppSample::NaInteger, 3,
                              rCppSample::NaInteger, 5};
    const auto actual = popcount_cpp_integer(arg);
    EXPECT_TRUE(are_equal(expected, actual));
}

TEST_F(TestPopcount, NegativeIntegerValues) {
    // Check negative int32 values
    // 0xffffffff, 0xfffffffe, 0xff0f0f0f, 0xf0f0f0f0, 0x80000001
    // Note that 0x80000000 is treated as NA in R
    using VectorType = rCppSample::IntegerVector;
    const VectorType arg{-1, -2, -15790321, -252645136, -2147483647};
    const VectorType expected{32, 31, 20, 16, 2};
    const auto actual = popcount_cpp_integer(arg);
    EXPECT_TRUE(are_equal(expected, actual));
}

TEST_F(TestPopcount, FullRawValues) {
    // Check all uint8 values
    using ArgVectorType = rCppSample::RawVector;
    using SizeType = typename VectorSize<ArgVectorType>::size_type;
    using IndexType = size_t;
    SizeType element_size = 256;
    SizeType set_size = 1024;
    SizeType array_size = element_size * set_size;
    ArgVectorType arg(array_size);
    rCppSample::IntegerVector expected(array_size);

    using Element = uint8_t;
    constexpr auto max_value = std::numeric_limits<Element>::max();
    for (decltype(array_size) index{0}; index < array_size; ++index) {
        auto low_value = index;
        constexpr auto mask_value = static_cast<decltype(low_value)>(max_value);
        low_value &= mask_value;
        auto element = static_cast<Element>(low_value);
        const auto array_index = static_cast<IndexType>(index);
        arg.at(array_index) = element;

        for (size_t bit_index{0}; bit_index < (sizeof(Element) * 8);
             ++bit_index) {
            decltype(low_value) mask = 1;
            mask <<= bit_index;
            expected.at(array_index) += ((low_value & mask) != 0);
        }
    }

    const auto actual = popcount_cpp_raw(arg);
    ASSERT_EQ(array_size, actual.size());
    EXPECT_TRUE(are_equal(expected, actual));
}

namespace {
const std::string R_CODE{"library(rCppSample)"};
RcodeFeeder code_feeder(R_CODE);

int custom_r_readconsole(const char *prompt, unsigned char *buf, int buflen,
                         int hist) {
    return code_feeder.feed_r_code(buf, buflen);
}
} // namespace

int main(int argc, char *argv[]) {
    char name[] = "test_popcount";
    char arg1[] = "--no-save";
    char *args[]{name, arg1, nullptr};
    Rf_initEmbeddedR((sizeof(args) / sizeof(args[0])) - 1, args);
    ptr_R_ReadConsole = custom_r_readconsole;
    R_ReplDLLinit();
    R_ReplDLLdo1();

    ::testing::InitGoogleTest(&argc, argv);
    const auto result = RUN_ALL_TESTS();
    Rf_endEmbeddedR(0);
    return result;
}
