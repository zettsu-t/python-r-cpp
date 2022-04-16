#include "test_popcount.h"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <gtest/gtest.h>
#include <limits>
#define R_INTERFACE_PTRS
#include <Rembedded.h>
#include <Rinterface.h>

namespace {
class RcodeFeeder {
  public:
    explicit RcodeFeeder(const std::string &r_code) : r_code_(r_code) {}
    virtual ~RcodeFeeder(void) = default;
    int feed_r_code(unsigned char *buf, int buflen) {
        if (!buf || (buflen <= 0)) {
            return 0;
        }
        *buf = '\0';

        const auto rcode_size = get_rcode_size();
        const auto suffix_buffer_size = get_suffix_buffer_size();
        // Exclude trailing NUL
        int n_remaining = rcode_size - r_code_copied_;
        // Include trailing NUL
        int buf_size = std::min(buflen, n_remaining + suffix_buffer_size);

        if (buf_size > suffix_buffer_size) {
            int code_size = buf_size - suffix_buffer_size;
            char *dst = reinterpret_cast<decltype(dst)>(buf);

            std::string r_code;
            r_code += r_code_.substr(r_code_copied_, code_size);
            r_code += r_suffix_;
            if (r_code.size() < buf_size) {
                strncpy(dst, r_code.c_str(), buf_size);
                r_code_copied_ += code_size;
            }
        }

        return r_code_copied_ >= rcode_size;
    }

    int get_rcode_size(void) { return static_cast<int>(r_code_.size()); }

    int get_suffix_size(void) { return static_cast<int>(r_suffix_.size()); }

    int get_suffix_buffer_size(void) { return get_suffix_size() + 1; }

  private:
    std::string r_code_;
    int r_code_copied_{0};
    static const std::string r_suffix_;
};

const std::string RcodeFeeder::r_suffix_{"\n"};
} // namespace

class TestFeedRcode : public ::testing::Test {};

namespace {
template <size_t N>
auto call_feed_rcode(RcodeFeeder &feeder, unsigned char (&buf)[N]) {
    return feeder.feed_r_code(buf, N);
}
} // namespace

TEST_F(TestFeedRcode, NullBuffer) {
    const std::string r_code{"print(1:5)"};
    RcodeFeeder feeder(r_code);
    ASSERT_FALSE(feeder.feed_r_code(nullptr, 0));

    const char expected = 'A';
    unsigned char buf[2]{expected, '\n'};
    ASSERT_FALSE(feeder.feed_r_code(buf, 0));
    ASSERT_EQ(expected, *buf);
}

TEST_F(TestFeedRcode, Empty) {
    const std::string r_code{"print(1:5)"};
    RcodeFeeder feeder1(r_code);
    unsigned char buf1[1]{'\n'};
    ASSERT_FALSE(call_feed_rcode(feeder1, buf1));
    ASSERT_FALSE(*buf1);

    RcodeFeeder feeder2(r_code);
    unsigned char buf2[]{'a', 'b'};
    ASSERT_FALSE(call_feed_rcode(feeder2, buf2));
    ASSERT_FALSE(*buf2);

    RcodeFeeder feeder3(r_code);
    unsigned char buf3[3]{};
    ASSERT_FALSE(call_feed_rcode(feeder3, buf3));
    ASSERT_TRUE(*buf3);
}

TEST_F(TestFeedRcode, Multiline) {
    const std::string r_code{"library(\"packageName\")"};
    RcodeFeeder feeder(r_code);

    unsigned char buf_1st[10];
    ASSERT_FALSE(call_feed_rcode(feeder, buf_1st));
    const std::string actual_1st(reinterpret_cast<const char *>(buf_1st));
    const std::string expected_1st("library(\n");
    ASSERT_EQ(expected_1st, actual_1st);

    unsigned char buf_2nd[15];
    ASSERT_FALSE(call_feed_rcode(feeder, buf_2nd));
    const std::string actual_2nd(reinterpret_cast<const char *>(buf_2nd));
    const std::string expected_2nd("\"packageName\"\n");
    ASSERT_EQ(expected_2nd, actual_2nd);

    unsigned char buf_3rd[3];
    ASSERT_TRUE(call_feed_rcode(feeder, buf_3rd));
    const std::string actual_3rd(reinterpret_cast<const char *>(buf_3rd));
    const std::string expected_3rd(")\n");
    ASSERT_EQ(expected_3rd, actual_3rd);
}

TEST_F(TestFeedRcode, NearExactSize) {
    const std::string r_code{"library(\"packageName\")"};
    RcodeFeeder common_feeder(r_code);
    const auto min_size = common_feeder.get_rcode_size() - 1;
    const auto full_size =
        common_feeder.get_rcode_size() + common_feeder.get_suffix_size() + 1;
    const auto buf_size = full_size + 2;
    const std::string suffix{"\n"};

    std::vector<unsigned char> line_buffer(buf_size);
    for (int buflen{min_size}; buflen < full_size; ++buflen) {
        std::fill(line_buffer.begin(), line_buffer.end(), 0);
        auto buf = line_buffer.data();
        RcodeFeeder feeder(r_code);

        ASSERT_FALSE(feeder.feed_r_code(buf, buflen));
        const std::string actual_1st(reinterpret_cast<const char *>(buf));
        std::string expected_1st = r_code.substr(0, buflen - 2);
        expected_1st += suffix;
        EXPECT_EQ(expected_1st, actual_1st);

        ASSERT_TRUE(feeder.feed_r_code(buf, buflen));
        const std::string actual_2nd(reinterpret_cast<const char *>(buf));
        std::string expected_2nd = r_code.substr(buflen - 2);
        expected_2nd += suffix;
        EXPECT_EQ(expected_2nd, actual_2nd);
    }

    for (int buflen{full_size}; buflen <= buf_size; ++buflen) {
        std::fill(line_buffer.begin(), line_buffer.end(), 0);
        auto buf = line_buffer.data();
        RcodeFeeder feeder(r_code);

        ASSERT_TRUE(feeder.feed_r_code(buf, buflen));
        const std::string actual(reinterpret_cast<const char *>(buf));
        auto expected = r_code;
        expected += suffix;
        ASSERT_EQ(expected, actual);
    }
}

class TestPopcount : public ::testing::Test {};

TEST_F(TestPopcount, Size) {
    // Empty arrays are acceptable
    for (size_t array_size{0}; array_size < 3; ++array_size) {
        const rCppSample::RawVector arg(array_size, 0);
        const auto actual = popcount_cpp_raw(arg);
        ASSERT_EQ(arg.size(), actual.size());
    }
}

TEST_F(TestPopcount, RawValues) {
    // Check 0..255 uint8 values
    const rCppSample::RawVector arg{0, 1, 2, 3, 6, 7, 254, 255};
    const rCppSample::IntegerVector expected{0, 1, 1, 2, 2, 3, 7, 8};
    const auto actual = popcount_cpp_raw(arg);
    ASSERT_EQ(expected.size(), actual.size());

    auto size = actual.size();
    for (decltype(size) i{0}; i < size; ++i) {
        EXPECT_EQ(expected.at(i), actual.at(i));
    }
}

TEST_F(TestPopcount, IntegerValues) {
    // Check some non-negative int32 values
    const rCppSample::IntegerVector arg{
        0,       1,          0xfe,       0xff,       0x100,    0xffff,
        0x10000, 0x7ffffffe, 0x7fffffff, 0x70f0f0f0, 0xf0f0f0f};
    const rCppSample::IntegerVector expected{0, 1,  7,  8,  1, 16,
                                             1, 30, 31, 15, 16};
    const auto actual = popcount_cpp_integer(arg);
    ASSERT_EQ(expected.size(), actual.size());

    auto size = actual.size();
    for (decltype(size) i{0}; i < size; ++i) {
        EXPECT_EQ(expected.at(i), actual.at(i));
    }
}

TEST_F(TestPopcount, NAs) {
    // Check NA values
    const rCppSample::IntegerVector arg{2, rCppSample::NaInteger, 14,
                                        rCppSample::NaInteger, 62};
    const rCppSample::IntegerVector expected{1, rCppSample::NaInteger, 3,
                                             rCppSample::NaInteger, 5};
    const auto actual = popcount_cpp_integer(arg);
    ASSERT_EQ(expected.size(), actual.size());

    auto size = actual.size();
    for (decltype(size) i{0}; i < size; ++i) {
        EXPECT_EQ(expected.at(i), actual.at(i));
    }
}

TEST_F(TestPopcount, NegativeIntegerValues) {
    // Check negative int32 values
    // 0xffffffff, 0xfffffffe, 0xff0f0f0f, 0xf0f0f0f0, 0x80000001
    // Note that 0x80000000 is treated as NA in R
    const rCppSample::IntegerVector arg{-1, -2, -15790321, -252645136,
                                        -2147483647};
    const rCppSample::IntegerVector expected{32, 31, 20, 16, 2};
    const auto actual = popcount_cpp_integer(arg);
    ASSERT_EQ(expected.size(), actual.size());

    auto size = actual.size();
    for (decltype(size) i{0}; i < size; ++i) {
        EXPECT_EQ(expected.at(i), actual.at(i));
    }
}

TEST_F(TestPopcount, FullRawValues) {
    // Check all uint8 values
    size_t element_size = 256;
    size_t set_size = 1024;
    size_t array_size = element_size * set_size;
    rCppSample::RawVector arg(array_size);
    rCppSample::IntegerVector expected(array_size);

    using Element = uint8_t;
    constexpr auto max_value = std::numeric_limits<Element>::max();
    for (decltype(array_size) index{0}; index < array_size; ++index) {
        auto low_value = index;
        constexpr auto mask_value = static_cast<decltype(low_value)>(max_value);
        low_value &= mask_value;
        auto element = static_cast<Element>(low_value);
        arg.at(index) = element;
        for (size_t bit_index{0}; bit_index < (sizeof(Element) * 8);
             ++bit_index) {
            decltype(low_value) mask = 1;
            mask <<= bit_index;
            expected.at(index) += ((low_value & mask) != 0);
        }
    }

    const auto actual = popcount_cpp_raw(arg);
    ASSERT_EQ(array_size, actual.size());
    for (decltype(array_size) i{0}; i < array_size; ++i) {
        EXPECT_EQ(expected.at(i), actual.at(i));
    }
}

namespace {
const std::string R_CODE{"library(\"rCppSample\")"};
RcodeFeeder code_feeder(R_CODE);
int custom_r_readconsole(const char *prompt, unsigned char *buf, int buflen,
                         int hist) {
    return code_feeder.feed_r_code(buf, buflen);
}
} // namespace

int main(int argc, char *argv[]) {
    char name[] = "test_popcount";
    char arg1[] = "--no-save";
    char *args[]{name, arg1};
    Rf_initEmbeddedR(sizeof(args) / sizeof(args[0]), args);
    ptr_R_ReadConsole = custom_r_readconsole;
    R_ReplDLLinit();
    R_ReplDLLdo1();

    ::testing::InitGoogleTest(&argc, argv);
    auto result = RUN_ALL_TESTS();
    Rf_endEmbeddedR(0);
    return result;
}
