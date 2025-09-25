#include "file.h"

#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

using namespace wip::utils::file;

class FileTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create test directory
        test_dir = std::filesystem::temp_directory_path() / "file_test";
        if (wip::utils::file::exists(test_dir)) {
            wip::utils::file::remove_all(test_dir);
        }
        wip::utils::file::create_directories(test_dir);
        
        // Create some test files and directories
        test_file = test_dir / "test_file.txt";
        test_dir_nested = test_dir / "nested";
        test_file_nested = test_dir_nested / "nested_file.txt";
        
        wip::utils::file::create_directory(test_dir_nested);
        wip::utils::file::write_file(test_file, "Hello, World!");
        wip::utils::file::write_file(test_file_nested, "Nested content");
    }
    
    void TearDown() override {
        // Clean up test directory
        if (wip::utils::file::exists(test_dir)) {
            wip::utils::file::remove_all(test_dir);
        }
    }
    
    std::filesystem::path test_dir;
    std::filesystem::path test_file;
    std::filesystem::path test_dir_nested;
    std::filesystem::path test_file_nested;
};

// ==================== File Existence and Type Tests ====================

TEST_F(FileTest, ExistsFunction) {
    EXPECT_TRUE(wip::utils::file::exists(test_file));
    EXPECT_TRUE(wip::utils::file::exists(test_dir));
    EXPECT_FALSE(wip::utils::file::exists(test_dir / "nonexistent.txt"));
}

TEST_F(FileTest, IsRegularFile) {
    EXPECT_TRUE(wip::utils::file::is_regular_file(test_file));
    EXPECT_FALSE(wip::utils::file::is_regular_file(test_dir));
    EXPECT_FALSE(wip::utils::file::is_regular_file(test_dir / "nonexistent.txt"));
}

TEST_F(FileTest, IsDirectory) {
    EXPECT_TRUE(wip::utils::file::is_directory(test_dir));
    EXPECT_TRUE(wip::utils::file::is_directory(test_dir_nested));
    EXPECT_FALSE(wip::utils::file::is_directory(test_file));
    EXPECT_FALSE(wip::utils::file::is_directory(test_dir / "nonexistent"));
}

TEST_F(FileTest, IsSymlink) {
    // Create a symlink for testing
    auto link_path = test_dir / "test_link";
    std::error_code ec;
    std::filesystem::create_symlink(test_file, link_path, ec);
    
    if (!ec) {
        EXPECT_TRUE(wip::utils::file::is_symlink(link_path));
    }
    EXPECT_FALSE(wip::utils::file::is_symlink(test_file));
    EXPECT_FALSE(wip::utils::file::is_symlink(test_dir));
}

TEST_F(FileTest, IsEmpty) {
    // Create empty file and directory
    auto empty_file = test_dir / "empty.txt";
    auto empty_dir = test_dir / "empty_dir";
    
    wip::utils::file::write_file(empty_file, "");
    wip::utils::file::create_directory(empty_dir);
    
    EXPECT_TRUE(wip::utils::file::is_empty(empty_file));
    EXPECT_TRUE(wip::utils::file::is_empty(empty_dir));
    EXPECT_FALSE(wip::utils::file::is_empty(test_file));
    EXPECT_FALSE(wip::utils::file::is_empty(test_dir));
}

// ==================== File Information Tests ====================

TEST_F(FileTest, FileSize) {
    auto size = wip::utils::file::file_size(test_file);
    ASSERT_TRUE(size.has_value());
    EXPECT_EQ(size.value(), 13); // "Hello, World!" is 13 characters
    
    auto invalid_size = wip::utils::file::file_size(test_dir / "nonexistent.txt");
    EXPECT_FALSE(invalid_size.has_value());
}

TEST_F(FileTest, LastWriteTime) {
    auto time = wip::utils::file::last_write_time(test_file);
    ASSERT_TRUE(time.has_value());
    // Just check that we got a valid time (check if it's a valid file time)
    auto now = std::filesystem::file_time_type::clock::now();
    EXPECT_LE(time.value(), now); // File time should not be in the future
    
    auto invalid_time = wip::utils::file::last_write_time(test_dir / "nonexistent.txt");
    EXPECT_FALSE(invalid_time.has_value());
}

TEST_F(FileTest, GetFileInfo) {
    auto info = wip::utils::file::get_file_info(test_file);
    ASSERT_TRUE(info.has_value());
    
    const auto& file_info = info.value();
    EXPECT_EQ(file_info.path, test_file);
    EXPECT_EQ(file_info.size, 13);
    EXPECT_TRUE(file_info.is_regular_file);
    EXPECT_FALSE(file_info.is_directory);
    EXPECT_FALSE(file_info.is_symlink);
    
    auto dir_info = wip::utils::file::get_file_info(test_dir);
    ASSERT_TRUE(dir_info.has_value());
    EXPECT_TRUE(dir_info.value().is_directory);
    EXPECT_FALSE(dir_info.value().is_regular_file);
}

// ==================== Directory Operation Tests ====================

TEST_F(FileTest, CreateDirectory) {
    auto new_dir = test_dir / "new_directory";
    EXPECT_TRUE(wip::utils::file::create_directory(new_dir));
    EXPECT_TRUE(wip::utils::file::is_directory(new_dir));
    
    // Creating existing directory should return false
    EXPECT_FALSE(wip::utils::file::create_directory(new_dir));
}

TEST_F(FileTest, CreateDirectories) {
    auto nested_path = test_dir / "level1" / "level2" / "level3";
    EXPECT_TRUE(wip::utils::file::create_directories(nested_path));
    EXPECT_TRUE(wip::utils::file::is_directory(nested_path));
    
    // Creating existing directory structure should return false
    EXPECT_FALSE(wip::utils::file::create_directories(nested_path));
}

TEST_F(FileTest, ListDirectory) {
    auto entries = wip::utils::file::list_directory(test_dir);
    ASSERT_TRUE(entries.has_value());
    
    // Should contain at least test_file and test_dir_nested
    EXPECT_GE(entries.value().size(), 2);
    
    bool found_file = false, found_dir = false;
    for (const auto& entry : entries.value()) {
        if (entry == test_file) found_file = true;
        if (entry == test_dir_nested) found_dir = true;
    }
    EXPECT_TRUE(found_file);
    EXPECT_TRUE(found_dir);
}

TEST_F(FileTest, ListDirectoryRecursive) {
    auto entries = wip::utils::file::list_directory_recursive(test_dir);
    ASSERT_TRUE(entries.has_value());
    
    // Should contain test_file, test_dir_nested, and test_file_nested
    EXPECT_GE(entries.value().size(), 3);
    
    bool found_nested_file = false;
    for (const auto& entry : entries.value()) {
        if (entry == test_file_nested) {
            found_nested_file = true;
            break;
        }
    }
    EXPECT_TRUE(found_nested_file);
}

TEST_F(FileTest, RemoveDirectory) {
    auto empty_dir = test_dir / "empty_to_remove";
    wip::utils::file::create_directory(empty_dir);
    
    EXPECT_TRUE(wip::utils::file::remove_directory(empty_dir));
    EXPECT_FALSE(wip::utils::file::exists(empty_dir));
    
    // Should not be able to remove non-empty directory
    EXPECT_FALSE(wip::utils::file::remove_directory(test_dir_nested));
    EXPECT_TRUE(wip::utils::file::exists(test_dir_nested));
}

TEST_F(FileTest, RemoveAll) {
    auto dir_to_remove = test_dir / "remove_all_test";
    wip::utils::file::create_directories(dir_to_remove / "subdir");
    wip::utils::file::write_file(dir_to_remove / "file.txt", "content");
    wip::utils::file::write_file(dir_to_remove / "subdir" / "file2.txt", "content2");
    
    auto count = wip::utils::file::remove_all(dir_to_remove);
    ASSERT_TRUE(count.has_value());
    EXPECT_GT(count.value(), 0);
    EXPECT_FALSE(wip::utils::file::exists(dir_to_remove));
}

// ==================== File Content Operation Tests ====================

TEST_F(FileTest, ReadFile) {
    auto content = wip::utils::file::read_file(test_file);
    ASSERT_TRUE(content.has_value());
    EXPECT_EQ(content.value(), "Hello, World!");
    
    auto invalid_content = wip::utils::file::read_file(test_dir / "nonexistent.txt");
    EXPECT_FALSE(invalid_content.has_value());
}

TEST_F(FileTest, ReadBinaryFile) {
    // Create binary file
    std::vector<uint8_t> binary_data = {0x00, 0x01, 0x02, 0xFF, 0xFE};
    auto binary_file = test_dir / "binary.bin";
    wip::utils::file::write_binary_file(binary_file, binary_data);
    
    auto read_data = wip::utils::file::read_binary_file(binary_file);
    ASSERT_TRUE(read_data.has_value());
    EXPECT_EQ(read_data.value(), binary_data);
}

TEST_F(FileTest, ReadLines) {
    auto lines_file = test_dir / "lines.txt";
    std::vector<std::string> original_lines = {"Line 1", "Line 2", "Line 3"};
    wip::utils::file::write_lines(lines_file, original_lines);
    
    auto read_lines_result = wip::utils::file::read_lines(lines_file);
    ASSERT_TRUE(read_lines_result.has_value());
    EXPECT_EQ(read_lines_result.value(), original_lines);
}

TEST_F(FileTest, WriteFile) {
    auto write_test_file = test_dir / "write_test.txt";
    std::string content = "Test content";
    
    EXPECT_TRUE(wip::utils::file::write_file(write_test_file, content));
    
    auto read_content = wip::utils::file::read_file(write_test_file);
    ASSERT_TRUE(read_content.has_value());
    EXPECT_EQ(read_content.value(), content);
    
    // Test append mode
    std::string append_content = " appended";
    EXPECT_TRUE(wip::utils::file::write_file(write_test_file, append_content, true));
    
    auto full_content = wip::utils::file::read_file(write_test_file);
    ASSERT_TRUE(full_content.has_value());
    EXPECT_EQ(full_content.value(), content + append_content);
}

TEST_F(FileTest, WriteBinaryFile) {
    auto binary_file = test_dir / "binary_write.bin";
    std::vector<uint8_t> data = {0xDE, 0xAD, 0xBE, 0xEF};
    
    EXPECT_TRUE(wip::utils::file::write_binary_file(binary_file, data));
    
    auto read_data = wip::utils::file::read_binary_file(binary_file);
    ASSERT_TRUE(read_data.has_value());
    EXPECT_EQ(read_data.value(), data);
}

TEST_F(FileTest, WriteLines) {
    auto lines_file = test_dir / "write_lines.txt";
    std::vector<std::string> lines = {"First line", "Second line", "Third line"};
    
    EXPECT_TRUE(wip::utils::file::write_lines(lines_file, lines));
    
    auto read_lines_result = wip::utils::file::read_lines(lines_file);
    ASSERT_TRUE(read_lines_result.has_value());
    EXPECT_EQ(read_lines_result.value(), lines);
}

// ==================== File Operation Command Tests ====================

TEST_F(FileTest, CopyFile) {
    auto destination = test_dir / "copied_file.txt";
    
    EXPECT_TRUE(wip::utils::file::copy(test_file, destination));
    EXPECT_TRUE(wip::utils::file::exists(destination));
    
    auto original_content = wip::utils::file::read_file(test_file);
    auto copied_content = wip::utils::file::read_file(destination);
    ASSERT_TRUE(original_content.has_value() && copied_content.has_value());
    EXPECT_EQ(original_content.value(), copied_content.value());
}

TEST_F(FileTest, CopyRecursive) {
    auto destination = test_dir / "copied_directory";
    
    EXPECT_TRUE(wip::utils::file::copy_recursive(test_dir_nested, destination));
    EXPECT_TRUE(wip::utils::file::is_directory(destination));
    
    auto copied_nested_file = destination / "nested_file.txt";
    EXPECT_TRUE(wip::utils::file::exists(copied_nested_file));
    
    auto original_content = wip::utils::file::read_file(test_file_nested);
    auto copied_content = wip::utils::file::read_file(copied_nested_file);
    ASSERT_TRUE(original_content.has_value() && copied_content.has_value());
    EXPECT_EQ(original_content.value(), copied_content.value());
}

TEST_F(FileTest, MoveFile) {
    auto temp_file = test_dir / "temp_for_move.txt";
    wip::utils::file::write_file(temp_file, "Move me");
    
    auto destination = test_dir / "moved_file.txt";
    
    EXPECT_TRUE(wip::utils::file::move(temp_file, destination));
    EXPECT_FALSE(wip::utils::file::exists(temp_file));
    EXPECT_TRUE(wip::utils::file::exists(destination));
    
    auto content = wip::utils::file::read_file(destination);
    ASSERT_TRUE(content.has_value());
    EXPECT_EQ(content.value(), "Move me");
}

TEST_F(FileTest, RemoveFile) {
    auto temp_file = test_dir / "temp_for_removal.txt";
    wip::utils::file::write_file(temp_file, "Remove me");
    
    EXPECT_TRUE(wip::utils::file::exists(temp_file));
    EXPECT_TRUE(wip::utils::file::remove_file(temp_file));
    EXPECT_FALSE(wip::utils::file::exists(temp_file));
}

// ==================== Path Operation Tests ====================

TEST_F(FileTest, CurrentPath) {
    auto current = wip::utils::file::current_path();
    ASSERT_TRUE(current.has_value());
    EXPECT_FALSE(current.value().empty());
}

TEST_F(FileTest, AbsolutePath) {
    auto relative = std::filesystem::path(".");
    auto absolute = wip::utils::file::absolute_path(relative);
    ASSERT_TRUE(absolute.has_value());
    EXPECT_TRUE(absolute.value().is_absolute());
}

TEST_F(FileTest, CanonicalPath) {
    auto canonical = wip::utils::file::canonical_path(test_file);
    ASSERT_TRUE(canonical.has_value());
    EXPECT_TRUE(canonical.value().is_absolute());
}

TEST_F(FileTest, RelativePath) {
    auto relative = wip::utils::file::relative_path(test_file, test_dir);
    ASSERT_TRUE(relative.has_value());
    EXPECT_EQ(relative.value(), "test_file.txt");
}

// ==================== Permission Operation Tests ====================

TEST_F(FileTest, GetAndSetPermissions) {
    auto perms = wip::utils::file::get_permissions(test_file);
    ASSERT_TRUE(perms.has_value());
    
    // Try to set read-only permissions
    auto readonly_perms = std::filesystem::perms::owner_read | 
                         std::filesystem::perms::group_read | 
                         std::filesystem::perms::others_read;
    
    EXPECT_TRUE(wip::utils::file::set_permissions(test_file, readonly_perms));
    
    // Restore original permissions
    EXPECT_TRUE(wip::utils::file::set_permissions(test_file, perms.value()));
}

// ==================== Utility Function Tests ====================

TEST_F(FileTest, GetExtension) {
    EXPECT_EQ(wip::utils::file::get_extension(test_file), ".txt");
    EXPECT_EQ(wip::utils::file::get_extension(test_dir / "file.tar.gz"), ".gz");
    EXPECT_EQ(wip::utils::file::get_extension(test_dir / "noext"), "");
}

TEST_F(FileTest, GetStem) {
    EXPECT_EQ(wip::utils::file::get_stem(test_file), "test_file");
    EXPECT_EQ(wip::utils::file::get_stem(test_dir / "file.tar.gz"), "file.tar");
    EXPECT_EQ(wip::utils::file::get_stem(test_dir / "noext"), "noext");
}

TEST_F(FileTest, GetParentPath) {
    EXPECT_EQ(wip::utils::file::get_parent_path(test_file), test_dir);
    EXPECT_EQ(wip::utils::file::get_parent_path(test_file_nested), test_dir_nested);
}

TEST_F(FileTest, JoinPath) {
    auto joined = wip::utils::file::join_path("path1", "path2", "file.txt");
    auto expected = std::filesystem::path("path1") / "path2" / "file.txt";
    EXPECT_EQ(joined, expected);
}

TEST_F(FileTest, CreateTempFile) {
    auto temp_file_path = wip::utils::file::create_temp_file();
    ASSERT_TRUE(temp_file_path.has_value());
    EXPECT_TRUE(wip::utils::file::exists(temp_file_path.value()));
    
    // Clean up
    wip::utils::file::remove_file(temp_file_path.value());
}

TEST_F(FileTest, CreateTempDirectory) {
    auto temp_dir_path = wip::utils::file::create_temp_directory();
    ASSERT_TRUE(temp_dir_path.has_value());
    EXPECT_TRUE(wip::utils::file::is_directory(temp_dir_path.value()));
    
    // Clean up
    wip::utils::file::remove_directory(temp_dir_path.value());
}

TEST_F(FileTest, FindFiles) {
    // Create some test files with different extensions
    wip::utils::file::write_file(test_dir / "file1.txt", "content1");
    wip::utils::file::write_file(test_dir / "file2.txt", "content2");
    wip::utils::file::write_file(test_dir / "file3.log", "content3");
    wip::utils::file::write_file(test_dir / "README.md", "readme");
    
    auto txt_files = wip::utils::file::find_files(test_dir, "*.txt", false);
    ASSERT_TRUE(txt_files.has_value());
    EXPECT_GE(txt_files.value().size(), 3); // at least 3 .txt files
    
    auto all_files = wip::utils::file::find_files(test_dir, "*", false);
    ASSERT_TRUE(all_files.has_value());
    EXPECT_GT(all_files.value().size(), txt_files.value().size());
}

TEST_F(FileTest, DirectorySize) {
    auto size = wip::utils::file::directory_size(test_dir);
    ASSERT_TRUE(size.has_value());
    EXPECT_GT(size.value(), 0);
    
    // Create a larger file and check that size increases
    wip::utils::file::write_file(test_dir / "large_file.txt", std::string(1000, 'X'));
    auto new_size = wip::utils::file::directory_size(test_dir);
    ASSERT_TRUE(new_size.has_value());
    EXPECT_GT(new_size.value(), size.value());
}

// ==================== Error Condition Tests ====================

TEST_F(FileTest, ErrorConditions) {
    auto nonexistent = test_dir / "nonexistent.txt";
    
    // Test operations on nonexistent files
    EXPECT_FALSE(wip::utils::file::read_file(nonexistent).has_value());
    EXPECT_FALSE(wip::utils::file::file_size(nonexistent).has_value());
    EXPECT_FALSE(wip::utils::file::get_file_info(nonexistent).has_value());
    EXPECT_FALSE(wip::utils::file::copy(nonexistent, test_dir / "dest.txt"));
    EXPECT_FALSE(wip::utils::file::move(nonexistent, test_dir / "dest.txt"));
}

// To run the tests, use the command: ctest --output-on-failure