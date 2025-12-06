#include "../../common/include/parser_utilities.hpp"
#include <gtest/gtest.h>
#include <string>
#include <vector>

// Fixture para tests de parser utilities
class ParserUtilitiesTest : public ::testing::Test {
protected:
  void SetUp() override { }
};

// Tests de split_line
TEST_F(ParserUtilitiesTest, SplitLine_EmptyString_ReturnsEmptyVector) {
  std::vector<std::string> const result = split_line("");

  EXPECT_TRUE(result.empty());
}

TEST_F(ParserUtilitiesTest, SplitLine_SingleWord_ReturnsSingleToken) {
  std::vector<std::string> const result = split_line("hello");

  ASSERT_EQ(result.size(), 1);
  EXPECT_EQ(result[0], "hello");
}

TEST_F(ParserUtilitiesTest, SplitLine_MultipleWords_ReturnsAllTokens) {
  std::vector<std::string> const result = split_line("hello world test");

  ASSERT_EQ(result.size(), 3);
  EXPECT_EQ(result[0], "hello");
  EXPECT_EQ(result[1], "world");
  EXPECT_EQ(result[2], "test");
}

TEST_F(ParserUtilitiesTest, SplitLine_MultipleSpaces_HandlesCorrectly) {
  std::vector<std::string> const result = split_line("  hello   world  test  ");

  ASSERT_EQ(result.size(), 3);
  EXPECT_EQ(result[0], "hello");
  EXPECT_EQ(result[1], "world");
  EXPECT_EQ(result[2], "test");
}

TEST_F(ParserUtilitiesTest, SplitLine_TabsAndSpaces_HandlesCorrectly) {
  std::vector<std::string> const result = split_line("hello\tworld  test");

  ASSERT_EQ(result.size(), 3);
  EXPECT_EQ(result[0], "hello");
  EXPECT_EQ(result[1], "world");
  EXPECT_EQ(result[2], "test");
}

TEST_F(ParserUtilitiesTest, SplitLine_OnlySpaces_ReturnsEmptyVector) {
  std::vector<std::string> const result = split_line("   \t   ");

  EXPECT_TRUE(result.empty());
}

// Tests de is_empty_line
TEST_F(ParserUtilitiesTest, IsEmptyLine_EmptyString_ReturnsTrue) {
  EXPECT_TRUE(is_empty_line(""));
}

TEST_F(ParserUtilitiesTest, IsEmptyLine_OnlySpaces_ReturnsTrue) {
  EXPECT_TRUE(is_empty_line("   "));
  EXPECT_TRUE(is_empty_line("\t"));
  EXPECT_TRUE(is_empty_line(" \t \t "));
}

TEST_F(ParserUtilitiesTest, IsEmptyLine_WithContent_ReturnsFalse) {
  EXPECT_FALSE(is_empty_line("hello"));
  EXPECT_FALSE(is_empty_line(" hello "));
  EXPECT_FALSE(is_empty_line("  hello world  "));
}

TEST_F(ParserUtilitiesTest, IsEmptyLine_MixedWhitespaceWithContent_ReturnsFalse) {
  EXPECT_FALSE(is_empty_line("\thello\t"));
  EXPECT_FALSE(is_empty_line(" \t test \t "));
}

// Tests de print_error (verificar que no crash)
TEST_F(ParserUtilitiesTest, PrintError_DoesNotCrash) {
  EXPECT_NO_THROW(print_error("Test error message"));
  EXPECT_NO_THROW(print_error(""));
  EXPECT_NO_THROW(print_error("Very long error message with details"));
}

// Tests de integración entre split_line e is_empty_line
TEST_F(ParserUtilitiesTest, Integration_SplitLineAfterIsEmptyLine) {
  // Línea con contenido
  std::string const line_with_content = "test line with tokens";
  EXPECT_FALSE(is_empty_line(line_with_content));

  std::vector<std::string> tokens = split_line(line_with_content);
  ASSERT_EQ(tokens.size(), 4);
  EXPECT_EQ(tokens[0], "test");
  EXPECT_EQ(tokens[1], "line");
  EXPECT_EQ(tokens[2], "with");
  EXPECT_EQ(tokens[3], "tokens");

  // Línea vacía
  std::string const empty_line = "   \t   ";
  EXPECT_TRUE(is_empty_line(empty_line));

  tokens = split_line(empty_line);
  EXPECT_TRUE(tokens.empty());
}
