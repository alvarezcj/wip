#include "file.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <random>
#include <algorithm>
#include <regex>

namespace wip::utils::file {

// Legacy function (keeping for compatibility)
void hello() {
    std::cout << "Hello from WIP Utils File!" << std::endl;
}

// ==================== File Existence and Type Operations ====================

bool exists(const Path& path) noexcept {
    std::error_code ec;
    return std::filesystem::exists(path, ec);
}

bool is_regular_file(const Path& path) noexcept {
    std::error_code ec;
    return std::filesystem::is_regular_file(path, ec);
}

bool is_directory(const Path& path) noexcept {
    std::error_code ec;
    return std::filesystem::is_directory(path, ec);
}

bool is_symlink(const Path& path) noexcept {
    std::error_code ec;
    return std::filesystem::is_symlink(path, ec);
}

bool is_empty(const Path& path) noexcept {
    std::error_code ec;
    return std::filesystem::is_empty(path, ec);
}

// ==================== File Information Operations ====================

Result<std::uintmax_t> file_size(const Path& path) noexcept {
    std::error_code ec;
    auto size = std::filesystem::file_size(path, ec);
    if (ec) {
        return std::nullopt;
    }
    return size;
}

Result<FileTime> last_write_time(const Path& path) noexcept {
    std::error_code ec;
    auto time = std::filesystem::last_write_time(path, ec);
    if (ec) {
        return std::nullopt;
    }
    return time;
}

Result<FileInfo> get_file_info(const Path& path) noexcept {
    std::error_code ec;
    auto status = std::filesystem::status(path, ec);
    if (ec) {
        return std::nullopt;
    }

    FileInfo info;
    info.path = path;
    info.type = status.type();
    info.permissions = status.permissions();
    info.is_directory = std::filesystem::is_directory(status);
    info.is_regular_file = std::filesystem::is_regular_file(status);
    info.is_symlink = std::filesystem::is_symlink(path, ec);

    if (info.is_regular_file) {
        auto size = std::filesystem::file_size(path, ec);
        info.size = ec ? 0 : size;
    } else {
        info.size = 0;
    }

    auto time = std::filesystem::last_write_time(path, ec);
    info.last_write_time = ec ? FileTime{} : time;

    return info;
}

// ==================== Directory Operations ====================

bool create_directory(const Path& path) noexcept {
    std::error_code ec;
    return std::filesystem::create_directory(path, ec);
}

bool create_directories(const Path& path) noexcept {
    std::error_code ec;
    return std::filesystem::create_directories(path, ec);
}

Result<std::vector<Path>> list_directory(const Path& path) noexcept {
    std::error_code ec;
    std::vector<Path> entries;
    
    for (const auto& entry : std::filesystem::directory_iterator(path, ec)) {
        if (ec) {
            return std::nullopt;
        }
        entries.push_back(entry.path());
    }
    
    return entries;
}

Result<std::vector<Path>> list_directory_recursive(const Path& path) noexcept {
    std::error_code ec;
    std::vector<Path> entries;
    
    for (const auto& entry : std::filesystem::recursive_directory_iterator(path, ec)) {
        if (ec) {
            return std::nullopt;
        }
        entries.push_back(entry.path());
    }
    
    return entries;
}

bool remove_directory(const Path& path) noexcept {
    std::error_code ec;
    return std::filesystem::remove(path, ec);
}

Result<std::uintmax_t> remove_all(const Path& path) noexcept {
    std::error_code ec;
    auto count = std::filesystem::remove_all(path, ec);
    if (ec) {
        return std::nullopt;
    }
    return count;
}

// ==================== File Content Operations ====================

Result<std::string> read_file(const Path& path) noexcept {
    try {
        std::ifstream file(path, std::ios::in);
        if (!file.is_open()) {
            return std::nullopt;
        }

        std::ostringstream content;
        content << file.rdbuf();
        return content.str();
    } catch (...) {
        return std::nullopt;
    }
}

Result<std::vector<uint8_t>> read_binary_file(const Path& path) noexcept {
    try {
        std::ifstream file(path, std::ios::in | std::ios::binary);
        if (!file.is_open()) {
            return std::nullopt;
        }

        file.seekg(0, std::ios::end);
        auto size = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<uint8_t> data(size);
        file.read(reinterpret_cast<char*>(data.data()), size);
        
        if (file.fail() && !file.eof()) {
            return std::nullopt;
        }

        return data;
    } catch (...) {
        return std::nullopt;
    }
}

Result<std::vector<std::string>> read_lines(const Path& path) noexcept {
    try {
        std::ifstream file(path);
        if (!file.is_open()) {
            return std::nullopt;
        }

        std::vector<std::string> lines;
        std::string line;
        while (std::getline(file, line)) {
            lines.push_back(line);
        }

        return lines;
    } catch (...) {
        return std::nullopt;
    }
}

bool write_file(const Path& path, const std::string& content, bool append) noexcept {
    try {
        std::ios::openmode mode = std::ios::out;
        if (append) {
            mode |= std::ios::app;
        }

        std::ofstream file(path, mode);
        if (!file.is_open()) {
            return false;
        }

        file << content;
        return file.good();
    } catch (...) {
        return false;
    }
}

bool write_binary_file(const Path& path, const std::vector<uint8_t>& data, bool append) noexcept {
    try {
        std::ios::openmode mode = std::ios::out | std::ios::binary;
        if (append) {
            mode |= std::ios::app;
        }

        std::ofstream file(path, mode);
        if (!file.is_open()) {
            return false;
        }

        file.write(reinterpret_cast<const char*>(data.data()), data.size());
        return file.good();
    } catch (...) {
        return false;
    }
}

bool write_lines(const Path& path, const std::vector<std::string>& lines, bool append) noexcept {
    try {
        std::ios::openmode mode = std::ios::out;
        if (append) {
            mode |= std::ios::app;
        }

        std::ofstream file(path, mode);
        if (!file.is_open()) {
            return false;
        }

        for (const auto& line : lines) {
            file << line << '\n';
        }

        return file.good();
    } catch (...) {
        return false;
    }
}

// ==================== File Operation Commands ====================

bool copy(const Path& source, const Path& destination, bool overwrite_existing) noexcept {
    std::error_code ec;
    auto options = std::filesystem::copy_options::none;
    if (overwrite_existing) {
        options |= std::filesystem::copy_options::overwrite_existing;
    }
    
    std::filesystem::copy(source, destination, options, ec);
    return !ec;
}

bool copy_recursive(const Path& source, const Path& destination, bool overwrite_existing) noexcept {
    std::error_code ec;
    auto options = std::filesystem::copy_options::recursive;
    if (overwrite_existing) {
        options |= std::filesystem::copy_options::overwrite_existing;
    }
    
    std::filesystem::copy(source, destination, options, ec);
    return !ec;
}

bool move(const Path& source, const Path& destination) noexcept {
    std::error_code ec;
    std::filesystem::rename(source, destination, ec);
    return !ec;
}

bool remove_file(const Path& path) noexcept {
    std::error_code ec;
    return std::filesystem::remove(path, ec);
}

// ==================== Path Operations ====================

Result<Path> current_path() noexcept {
    std::error_code ec;
    auto path = std::filesystem::current_path(ec);
    if (ec) {
        return std::nullopt;
    }
    return path;
}

bool set_current_path(const Path& path) noexcept {
    std::error_code ec;
    std::filesystem::current_path(path, ec);
    return !ec;
}

Result<Path> absolute_path(const Path& path) noexcept {
    std::error_code ec;
    auto abs_path = std::filesystem::absolute(path, ec);
    if (ec) {
        return std::nullopt;
    }
    return abs_path;
}

Result<Path> canonical_path(const Path& path) noexcept {
    std::error_code ec;
    auto canonical = std::filesystem::canonical(path, ec);
    if (ec) {
        return std::nullopt;
    }
    return canonical;
}

Result<Path> relative_path(const Path& target, const Path& base) noexcept {
    std::error_code ec;
    auto relative = std::filesystem::relative(target, base, ec);
    if (ec) {
        return std::nullopt;
    }
    return relative;
}

// ==================== Permission Operations ====================

Result<Permissions> get_permissions(const Path& path) noexcept {
    std::error_code ec;
    auto status = std::filesystem::status(path, ec);
    if (ec) {
        return std::nullopt;
    }
    return status.permissions();
}

bool set_permissions(const Path& path, Permissions permissions) noexcept {
    std::error_code ec;
    std::filesystem::permissions(path, permissions, ec);
    return !ec;
}

// ==================== Utility Functions ====================

std::string get_extension(const Path& path) noexcept {
    return path.extension().string();
}

std::string get_stem(const Path& path) noexcept {
    return path.stem().string();
}

Path get_parent_path(const Path& path) noexcept {
    return path.parent_path();
}

Result<Path> create_temp_file(const Path& directory, const std::string& prefix, const std::string& extension) noexcept {
    try {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(100000, 999999);
        
        for (int attempts = 0; attempts < 100; ++attempts) {
            auto temp_name = prefix + std::to_string(dis(gen)) + extension;
            auto temp_path = directory / temp_name;
            
            if (!wip::utils::file::exists(temp_path)) {
                std::ofstream file(temp_path);
                if (file.good()) {
                    return temp_path;
                }
            }
        }
        return std::nullopt;
    } catch (...) {
        return std::nullopt;
    }
}

Result<Path> create_temp_directory(const Path& directory, const std::string& prefix) noexcept {
    try {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(100000, 999999);
        
        for (int attempts = 0; attempts < 100; ++attempts) {
            auto temp_name = prefix + std::to_string(dis(gen));
            auto temp_path = directory / temp_name;
            
            if (!wip::utils::file::exists(temp_path) && wip::utils::file::create_directory(temp_path)) {
                return temp_path;
            }
        }
        return std::nullopt;
    } catch (...) {
        return std::nullopt;
    }
}

Result<std::vector<Path>> find_files(const Path& directory, const std::string& pattern, bool recursive) noexcept {
    try {
        std::vector<Path> matches;
        
        // Convert glob pattern to regex
        std::string regex_pattern = pattern;
        std::replace(regex_pattern.begin(), regex_pattern.end(), '.', '\\');
        std::replace(regex_pattern.begin(), regex_pattern.end(), '*', '.');
        regex_pattern = std::regex_replace(regex_pattern, std::regex("\\\\."), "\\.");
        regex_pattern = std::regex_replace(regex_pattern, std::regex("\\."), ".*");
        
        std::regex reg(regex_pattern, std::regex_constants::icase);
        
        std::error_code ec;
        if (recursive) {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(directory, ec)) {
                if (ec) return std::nullopt;
                if (entry.is_regular_file() && std::regex_match(entry.path().filename().string(), reg)) {
                    matches.push_back(entry.path());
                }
            }
        } else {
            for (const auto& entry : std::filesystem::directory_iterator(directory, ec)) {
                if (ec) return std::nullopt;
                if (entry.is_regular_file() && std::regex_match(entry.path().filename().string(), reg)) {
                    matches.push_back(entry.path());
                }
            }
        }
        
        return matches;
    } catch (...) {
        return std::nullopt;
    }
}

Result<std::uintmax_t> directory_size(const Path& path) noexcept {
    try {
        std::uintmax_t total_size = 0;
        std::error_code ec;
        
        for (const auto& entry : std::filesystem::recursive_directory_iterator(path, ec)) {
            if (ec) return std::nullopt;
            if (entry.is_regular_file()) {
                auto size = std::filesystem::file_size(entry, ec);
                if (ec) continue; // Skip files we can't read
                total_size += size;
            }
        }
        
        return total_size;
    } catch (...) {
        return std::nullopt;
    }
}

}  // namespace wip::utils::file
