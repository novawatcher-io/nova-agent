#include "common/file.h"
#include <gtest/gtest.h>

TEST(TestFileReader, ReadFile) {
    App::Common::BasicFileReader reader;
    EXPECT_FALSE(reader.ReadFile("/non/exist/file", nullptr));
    EXPECT_FALSE(reader.ReadFile("/dev/null", nullptr));

    std::string content;
    EXPECT_FALSE(reader.ReadFile("/non/exist/file", &content));

    EXPECT_TRUE(reader.ReadFile("/proc/1/status", &content));
    EXPECT_GT(content.size(), 0);

    EXPECT_TRUE(reader.ReadFile("/dev/null", &content));
    EXPECT_EQ(content.size(), 0);
}