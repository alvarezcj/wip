#include "file.h"

#include <gtest/gtest.h>

TEST(FileTest, Hello) {
    EXPECT_NO_THROW(wip::utils::file::hello());
}
// To run the tests, use the command: ctest --output-on-failure

