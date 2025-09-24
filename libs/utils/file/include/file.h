#pragma once

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <optional>
#include <chrono>
#include <functional>

namespace wip::utils::file {

// Type aliases for convenience
using Path = std::filesystem::path;
using FileTime = std::filesystem::file_time_type;
using Permissions = std::filesystem::perms;
using FileType = std::filesystem::file_type;
using ErrorCode = std::error_code;

/**
 * @brief File operations result wrapper
 * @tparam T Result type
 */
template<typename T>
using Result = std::optional<T>;

/**
 * @brief File information structure
 */
struct FileInfo {
    Path path;
    std::uintmax_t size;
    FileTime last_write_time;
    FileType type;
    Permissions permissions;
    bool is_directory;
    bool is_regular_file;
    bool is_symlink;
};

// ==================== File Existence and Type Operations ====================

/**
 * @brief Check if a file or directory exists
 * @param path Path to check
 * @return true if exists, false otherwise
 */
bool exists(const Path& path) noexcept;

/**
 * @brief Check if path is a regular file
 * @param path Path to check
 * @return true if is regular file, false otherwise
 */
bool is_regular_file(const Path& path) noexcept;

/**
 * @brief Check if path is a directory
 * @param path Path to check
 * @return true if is directory, false otherwise
 */
bool is_directory(const Path& path) noexcept;

/**
 * @brief Check if path is a symbolic link
 * @param path Path to check
 * @return true if is symbolic link, false otherwise
 */
bool is_symlink(const Path& path) noexcept;

/**
 * @brief Check if path is empty (file with 0 bytes or empty directory)
 * @param path Path to check
 * @return true if empty, false otherwise
 */
bool is_empty(const Path& path) noexcept;

// ==================== File Information Operations ====================

/**
 * @brief Get file size in bytes
 * @param path Path to file
 * @return File size or nullopt if error
 */
Result<std::uintmax_t> file_size(const Path& path) noexcept;

/**
 * @brief Get file last write time
 * @param path Path to file
 * @return Last write time or nullopt if error
 */
Result<FileTime> last_write_time(const Path& path) noexcept;

/**
 * @brief Get comprehensive file information
 * @param path Path to file
 * @return FileInfo structure or nullopt if error
 */
Result<FileInfo> get_file_info(const Path& path) noexcept;

// ==================== Directory Operations ====================

/**
 * @brief Create a directory
 * @param path Directory path to create
 * @return true if created successfully, false otherwise
 */
bool create_directory(const Path& path) noexcept;

/**
 * @brief Create directories recursively
 * @param path Directory path to create (including parent directories)
 * @return true if created successfully, false otherwise
 */
bool create_directories(const Path& path) noexcept;

/**
 * @brief List directory contents
 * @param path Directory path
 * @return Vector of paths in directory or nullopt if error
 */
Result<std::vector<Path>> list_directory(const Path& path) noexcept;

/**
 * @brief List directory contents recursively
 * @param path Directory path
 * @return Vector of all paths in directory tree or nullopt if error
 */
Result<std::vector<Path>> list_directory_recursive(const Path& path) noexcept;

/**
 * @brief Remove empty directory
 * @param path Directory path to remove
 * @return true if removed successfully, false otherwise
 */
bool remove_directory(const Path& path) noexcept;

/**
 * @brief Remove directory and all its contents recursively
 * @param path Directory path to remove
 * @return Number of files/directories removed, or nullopt if error
 */
Result<std::uintmax_t> remove_all(const Path& path) noexcept;

// ==================== File Content Operations ====================

/**
 * @brief Read entire file content as string
 * @param path File path to read
 * @return File content or nullopt if error
 */
Result<std::string> read_file(const Path& path) noexcept;

/**
 * @brief Read file content as binary data
 * @param path File path to read
 * @return Binary content or nullopt if error
 */
Result<std::vector<uint8_t>> read_binary_file(const Path& path) noexcept;

/**
 * @brief Read file lines into vector
 * @param path File path to read
 * @return Vector of lines or nullopt if error
 */
Result<std::vector<std::string>> read_lines(const Path& path) noexcept;

/**
 * @brief Write string content to file
 * @param path File path to write
 * @param content Content to write
 * @param append If true, append to file; if false, overwrite
 * @return true if written successfully, false otherwise
 */
bool write_file(const Path& path, const std::string& content, bool append = false) noexcept;

/**
 * @brief Write binary data to file
 * @param path File path to write
 * @param data Binary data to write
 * @param append If true, append to file; if false, overwrite
 * @return true if written successfully, false otherwise
 */
bool write_binary_file(const Path& path, const std::vector<uint8_t>& data, bool append = false) noexcept;

/**
 * @brief Write lines to file
 * @param path File path to write
 * @param lines Lines to write
 * @param append If true, append to file; if false, overwrite
 * @return true if written successfully, false otherwise
 */
bool write_lines(const Path& path, const std::vector<std::string>& lines, bool append = false) noexcept;

// ==================== File Operation Commands ====================

/**
 * @brief Copy file or directory
 * @param source Source path
 * @param destination Destination path
 * @param overwrite_existing If true, overwrite existing files
 * @return true if copied successfully, false otherwise
 */
bool copy(const Path& source, const Path& destination, bool overwrite_existing = false) noexcept;

/**
 * @brief Copy file or directory recursively
 * @param source Source path
 * @param destination Destination path
 * @param overwrite_existing If true, overwrite existing files
 * @return true if copied successfully, false otherwise
 */
bool copy_recursive(const Path& source, const Path& destination, bool overwrite_existing = false) noexcept;

/**
 * @brief Move/rename file or directory
 * @param source Source path
 * @param destination Destination path
 * @return true if moved successfully, false otherwise
 */
bool move(const Path& source, const Path& destination) noexcept;

/**
 * @brief Remove file
 * @param path File path to remove
 * @return true if removed successfully, false otherwise
 */
bool remove_file(const Path& path) noexcept;

// ==================== Path Operations ====================

/**
 * @brief Get current working directory
 * @return Current working directory or nullopt if error
 */
Result<Path> current_path() noexcept;

/**
 * @brief Set current working directory
 * @param path New working directory
 * @return true if changed successfully, false otherwise
 */
bool set_current_path(const Path& path) noexcept;

/**
 * @brief Get absolute path
 * @param path Relative or absolute path
 * @return Absolute path or nullopt if error
 */
Result<Path> absolute_path(const Path& path) noexcept;

/**
 * @brief Get canonical path (resolve symlinks and relative components)
 * @param path Path to canonicalize
 * @return Canonical path or nullopt if error
 */
Result<Path> canonical_path(const Path& path) noexcept;

/**
 * @brief Get relative path from base to target
 * @param target Target path
 * @param base Base path
 * @return Relative path or nullopt if error
 */
Result<Path> relative_path(const Path& target, const Path& base) noexcept;

// ==================== Permission Operations ====================

/**
 * @brief Get file permissions
 * @param path File path
 * @return File permissions or nullopt if error
 */
Result<Permissions> get_permissions(const Path& path) noexcept;

/**
 * @brief Set file permissions
 * @param path File path
 * @param permissions New permissions
 * @return true if set successfully, false otherwise
 */
bool set_permissions(const Path& path, Permissions permissions) noexcept;

// ==================== Utility Functions ====================

/**
 * @brief Get file extension
 * @param path File path
 * @return File extension (including dot) or empty string
 */
std::string get_extension(const Path& path) noexcept;

/**
 * @brief Get filename without extension
 * @param path File path
 * @return Filename without extension
 */
std::string get_stem(const Path& path) noexcept;

/**
 * @brief Get parent directory path
 * @param path File/directory path
 * @return Parent directory path
 */
Path get_parent_path(const Path& path) noexcept;

/**
 * @brief Join multiple path components
 * @param components Path components to join
 * @return Joined path
 */
template<typename... Args>
Path join_path(Args&&... components) {
    Path result;
    ((result /= std::forward<Args>(components)), ...);
    return result;
}

/**
 * @brief Create temporary file with unique name
 * @param directory Directory for temp file (default: system temp)
 * @param prefix Filename prefix
 * @param extension File extension
 * @return Path to created temp file or nullopt if error
 */
Result<Path> create_temp_file(const Path& directory = std::filesystem::temp_directory_path(),
                             const std::string& prefix = "temp",
                             const std::string& extension = ".tmp") noexcept;

/**
 * @brief Create temporary directory with unique name
 * @param directory Parent directory for temp dir (default: system temp)
 * @param prefix Directory name prefix
 * @return Path to created temp directory or nullopt if error
 */
Result<Path> create_temp_directory(const Path& directory = std::filesystem::temp_directory_path(),
                                  const std::string& prefix = "temp") noexcept;

/**
 * @brief Find files matching pattern in directory
 * @param directory Directory to search
 * @param pattern Glob pattern (e.g., "*.txt")
 * @param recursive Search recursively
 * @return Vector of matching file paths or nullopt if error
 */
Result<std::vector<Path>> find_files(const Path& directory,
                                    const std::string& pattern,
                                    bool recursive = false) noexcept;

/**
 * @brief Calculate directory size recursively
 * @param path Directory path
 * @return Total size in bytes or nullopt if error
 */
Result<std::uintmax_t> directory_size(const Path& path) noexcept;

}  // namespace wip::utils::file