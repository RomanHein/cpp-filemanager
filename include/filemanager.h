#ifndef FILEMANAGER_H
#define FILEMANAGER_H

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <filesystem>
#include <stdexcept>
#include <utility>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <variant>

#ifdef _MSC_VER
#include <intrin.h>
#endif

#define THROW_FILE_LOCKED throw std::logical_error("FileManager -> All attempts to save data failed");
#define THROW_INVALID_INDEX(index, active_indices) throw std::out_of_range("FileManager -> Index " + std::to_string(index) + " is out of bounds (file only has " + std::to_string(active_indices) + " lines)");
#define THROW_NO_FRONT_OR_BACK throw std::out_of_range("FileManager -> Can't access front/back because file is empty");
#define THROW_ILLEGAL_CHARACTER throw std::invalid_argument("FileManager -> Tried to append/overwrite with illegal character (\\n or \\r)");
#define THROW_COULDNT_OPEN_FILE(file) throw std::runtime_error("FileManager -> Couldn't open file " + file);
#define THROW_COULDNT_DELETE_JOURNAL throw std::runtime_error("FileManager -> Couldn't delete journal, missing permissions?");

#define JOURNAL_TOKEN_DELIMITER ';'

namespace fm {
namespace internal {
// --------------------------------------------------
//                       ENUMS
// --------------------------------------------------

enum class Instruction : char {
    Append = 'A',
    Overwrite = 'O',
    Erase = 'E',
    Clear = 'C'
};

enum class ManagerState {
    Healthy,
    Degraded,
    Locked
};

// --------------------------------------------------
//                        POD
// --------------------------------------------------

struct UniformPage {
    uint16_t line_size;
};

struct VariedPage {
    std::vector<uint32_t> offset_sums;
};

struct Token {
    bool is_valid;
    std::string value;
};

// --------------------------------------------------
//                     COMPONENTS
// --------------------------------------------------

class Page {
public:
    explicit Page(const uint64_t page_offset, UniformPage uniform_page)
        :   page_offset_(page_offset),
            page_(uniform_page)
    {}

    explicit Page(const uint64_t page_offset, VariedPage varied_page)
        :   page_offset_(page_offset),
            page_(std::move(varied_page))
    {}

    [[nodiscard]] uint64_t select(const size_t line_index) const {
        if (const auto* uniform_page = std::get_if<UniformPage>(&page_)) {
            return page_offset_ + uniform_page->line_size * line_index;
        }

        return page_offset_ + std::get<VariedPage>(page_).offset_sums[line_index];
    }

private:
    uint64_t page_offset_;
    std::variant<UniformPage, VariedPage> page_;
};

class Bitset {
public:
    explicit Bitset(const uint16_t active_bits = 0) {
        assert(active_bits <= 64 && "Bitset64::Bitset64() -> Active bits to set is higher than 64 (max)");

        if (active_bits == 0) {
            bitset_ = 0;
        } else if (active_bits >= 64) {
            bitset_ = ~0ULL;
        } else {
            bitset_ = (1ULL << active_bits) - 1;
        }
    }

    void set(const uint16_t bit) {
        assert(bit < 64 && "Bitset::set() -> bit is out of bounds");
        bitset_ |= (1ULL << bit);
    }

    void reset(const uint16_t bit) {
        assert(bit < 64 && "Bitset::reset() -> bit is out of bounds");
        bitset_ &= ~(1ULL << bit);
    }

    [[nodiscard]] bool test(const uint16_t bit) const {
        assert(bit < 64 && "Bitset::test() -> bit is out of bounds");
        return bitset_ & (1ULL << bit);
    }

    [[nodiscard]] uint16_t count() const {
        return static_cast<uint16_t>(
#ifdef _MSC_VER
            __popcnt64(bitset_)
#else
            __builtin_popcountll(bitset_)
#endif
        );
    }

    [[nodiscard]] int16_t find_first(const uint16_t start_pos = 0) const {
        assert(start_pos < 64 && "Bitset::find_first() -> start_pos is out of bounds");
        const uint64_t mask = bitset_ & (~0ULL << start_pos);
        if (mask == 0) return -1;
#ifdef _MSC_VER
        unsigned long index;
        _BitScanForward64(&index, mask);
        return static_cast<int16_t>(index);
#else
        return static_cast<int16_t>(__builtin_ctzll(mask));
#endif
    }

private:
    uint64_t bitset_ = 0;
};

// --------------------------------------------------
//                      SERVICES
// --------------------------------------------------

class LineByteOffsetMap {
public:
    explicit LineByteOffsetMap(std::filesystem::path file_path)
        :   file_path_(std::move(file_path))
    {}

    [[nodiscard]] uint64_t select(const size_t line_index) const {
        return pages_[line_index / 128].select(line_index % 128);
    }

    void build() {
        std::ifstream istream(file_path_, std::ios::binary);
        if (!istream.is_open()) THROW_COULDNT_OPEN_FILE(file_path_.string());

        size_t first_line_size = std::string::npos;
        std::vector<uint32_t> offset_sums(128);
        uint64_t page_offset = 0;
        bool is_uniform = true;
        std::string line;
        size_ = 0;

        while (true) {
            const uint64_t offset = istream.tellg();
            if (!std::getline(istream, line)) break;

            const uint16_t line_size = static_cast<uint64_t>(istream.tellg()) - offset;
            offset_sums.emplace_back(offset - page_offset);

            if (first_line_size == std::string::npos) {
                first_line_size = line_size;
            } else if (line_size != first_line_size) {
                is_uniform = false;
            }

            ++size_;

            if (size_ % 128 == 0) {
                if (is_uniform) {
                    pages_.emplace_back(page_offset, UniformPage{static_cast<uint16_t>(first_line_size)});
                } else {
                    pages_.emplace_back(page_offset, VariedPage{std::move(offset_sums)});
                }

                is_uniform = true;
                page_offset = istream.tellg();
                offset_sums.clear();
                first_line_size = std::string::npos;
            }
        }

        if (size_ % 128 > 0) {
            if (is_uniform) {
                pages_.emplace_back(page_offset, UniformPage{static_cast<uint16_t>(first_line_size)});
            } else {
                pages_.emplace_back(page_offset, VariedPage{std::move(offset_sums)});
            }
        }
    }

    [[nodiscard]] size_t size() const noexcept {
        return size_;
    }

private:
    const std::filesystem::path file_path_;
    std::vector<Page> pages_;
    size_t size_ = 0;
};

class PhysicalIndexMap {
public:
    [[nodiscard]] size_t select(const size_t logical_index) {
        for (size_t total_active_bits = 0, i = front() / 64; i < layout_.size(); ++i) {
            if (const uint16_t active_bits = layout_[i].count();
                active_bits + total_active_bits > logical_index) {
                const size_t goal = logical_index - total_active_bits;
                uint16_t bit_pos = 0;

                for (int k = 0; k < goal; ++k) {
                    bit_pos = layout_[i].find_first(bit_pos) + 1;
                }

                return i * 64 + layout_[i].find_first(bit_pos);
            } else {
                total_active_bits += active_bits;
            }
        }

        assert(false && "IndexMap::select() failed -> Couldn't find physical index for logical index");
    }

    [[nodiscard]] size_t front() {
        assert(size_ > 0 && "IndexMap::front() -> Can't access front index because the index map is empty");

        if (front_ == ULLONG_MAX) {
            front_ = 0; // select also uses front(), not setting 0 would cause an infinite recursive loop
            front_ = select(0);
        }

        return front_;
    }

    [[nodiscard]] size_t back() {
        assert(size_ > 0 && "IndexMap::back() -> Can't access back index because the index map is empty");

        if (back_ == ULLONG_MAX) {
            back_ = select(size_ - 1);
        }

        return back_;
    }

    void expand() {
        if (capacity_ / 64 >= layout_.size()) layout_.emplace_back();
        layout_[capacity_ / 64].set(capacity_ % 64);
        back_ = capacity_;
        ++capacity_;
        ++size_;
    }

    void disable(const size_t physical_index) {
        assert(physical_index < capacity_ && "IndexMap::disable() -> Physical index is out of bounds");

        if (!layout_[physical_index / 64].test(physical_index % 64)) return;

        if (physical_index == front_) {
            front_ = ULLONG_MAX;
        } else if (physical_index == back_) {
            back_ = ULLONG_MAX;
        }

        layout_[physical_index / 64].reset(physical_index % 64);
        --size_;
    }

    void reset(const size_t active_indices = 0) {
        if (active_indices == 0 && capacity_ == 0) return;
        layout_.clear();

        if (active_indices > 0) {
            for (size_t i = 0; i < active_indices / 64; ++i) {
                layout_.emplace_back(64);
            }

            if (active_indices % 64 > 0) {
                layout_.emplace_back(active_indices % 64);
            }
        }

        size_ = active_indices;
        capacity_ = active_indices;
        front_ = active_indices > 0 ? 0 : ULLONG_MAX;
        back_ = active_indices > 0 ? active_indices - 1 : ULLONG_MAX;
    }

    [[nodiscard]] bool empty() const {
        return size_ == 0;
    }

    [[nodiscard]] size_t size() const {
        return size_;
    }

    [[nodiscard]] size_t capacity() const {
        return capacity_;
    }

    [[nodiscard]] bool is_active(const size_t physical_index) const {
        assert(physical_index < capacity_ && "IndexMap::is_active() -> Physical index is out of bounds");
        return layout_[physical_index / 64].test(physical_index % 64);
    }

private:
    std::vector<Bitset> layout_;
    size_t capacity_ = 0; // total indices, including "deleted" indices
    size_t size_ = 0; // active indices
    size_t back_ = ULLONG_MAX;
    size_t front_ = ULLONG_MAX;
};

// --------------------------------------------------
//                       SYSTEMS
// --------------------------------------------------

class FileIO {
public:
    explicit FileIO(std::filesystem::path file_path)
        :   byte_offset_map_(file_path),
            istream_(file_path, std::ios::binary),
            file_path_(std::move(file_path))
    {}

    void load() {
        byte_offset_map_.build();
        index_map_.reset(byte_offset_map_.size());
    }

    [[nodiscard]] bool commit() {
        if (!needs_consolidation_) return true;

        std::filesystem::path tmp_path = file_path_;
        tmp_path.replace_extension(".tmp");
        std::ofstream ostream(tmp_path, std::ios::out);

        if (!ostream.is_open()) return false;

        if (!index_map_.empty()) {
            for (size_t i = index_map_.front(); i < index_map_.capacity(); ++i) {
                if (index_map_.is_active(i)) {
                    ostream << access_(i) << "\n";
                }
            }
        }

        ostream.close();
        istream_.close();
        std::error_code ec;
        std::filesystem::rename(tmp_path, file_path_, ec);
        istream_.open(file_path_.c_str(), std::ios::in);

        if (ec) {
            std::filesystem::remove(tmp_path, ec);
            return false;
        }

        changes_.clear();
        needs_consolidation_ = false;
        return true;
    }

    [[nodiscard]] std::string read(const size_t logical_index) {
        return access_(index_map_.select(logical_index));
    }

    [[nodiscard]] std::string front() {
        return access_(index_map_.front());
    }

    [[nodiscard]] std::string back() {
        return access_(index_map_.back());
    }

    [[nodiscard]] std::vector<std::string> all() {
        if (index_map_.empty()) return {};

        std::vector<std::string> result;
        result.reserve(index_map_.size());

        for (size_t i = index_map_.front(); i < index_map_.capacity(); ++i) {
            if (index_map_.is_active(i)) {
                result.emplace_back(access_(i));
            }
        }

        return result;
    }

    void append(std::string text) {
        changes_.insert_or_assign(index_map_.size(), std::move(text));
        index_map_.expand();
        needs_consolidation_ = true;
    }

    void overwrite(const size_t logical_index, std::string text) {
        changes_.insert_or_assign(index_map_.select(logical_index), std::move(text));
        needs_consolidation_ = true;
    }

    void erase(const size_t logical_index) {
        index_map_.disable(index_map_.select(logical_index));
        needs_consolidation_ = true;
    }

    void clear() {
        index_map_.reset();
        needs_consolidation_ = true;
    }

    [[nodiscard]] bool exists() const {
        return std::filesystem::exists(file_path_);
    }

    [[nodiscard]] size_t size() const {
        return index_map_.size();
    }

    [[nodiscard]] bool empty() const {
        return index_map_.empty();
    }

private:
    [[nodiscard]] std::string access_(const size_t physical_index) {
        if (const auto it = changes_.find(physical_index); it != changes_.end()) return it->second;
        if (!istream_.is_open()) THROW_COULDNT_OPEN_FILE(file_path_.string());
        istream_.seekg(static_cast<std::streamoff>(byte_offset_map_.select(physical_index)), std::ios::beg);

        std::string line;
        std::getline(istream_, line);
        istream_.clear();

        return line;
    }

    PhysicalIndexMap index_map_;
    LineByteOffsetMap byte_offset_map_;
    std::ifstream istream_;
    const std::filesystem::path file_path_;
    std::pmr::unordered_map<size_t, std::string> changes_;
    bool needs_consolidation_ = false;
};

class Journal {
public:
    explicit Journal(const std::filesystem::path &file_path) : journal_path_(
        file_path.parent_path() / (file_path.stem().string() + "_journal.log")) {
    }

    template<typename... Args>
    void record(Instruction instruction, Args... args) {
        std::string entry;
        entry.push_back(static_cast<char>(instruction));
        entry.push_back(JOURNAL_TOKEN_DELIMITER);

        if (sizeof...(Args) > 0) {
            ((entry += tokenize_(std::forward<Args>(args))), ...);
        }

        unsaved_records_.push_back(std::move(entry));
    }

    void replay(FileIO &io_handler) const {
        std::ifstream istream(journal_path_, std::ios::in);
        std::string line;
        std::vector<std::string> args;

        if (!istream.is_open())
            THROW_COULDNT_OPEN_FILE(journal_path_.string());

        while (std::getline(istream, line)) {
            if (line.size() < 2) continue;
            const auto instruction = static_cast<Instruction>(line[0]);
            size_t cursor_pos = 2;
            args.clear();

            while (cursor_pos != std::string::npos && line.size() > 2) {
                if (auto [is_valid, value] = extract_token_(line, cursor_pos); is_valid) {
                    args.push_back(std::move(value));
                } else {
                    break;
                }
            }

            handle_instruction_(instruction, args, io_handler);
        }
    }

    bool flush() {
        if (unsaved_records_.empty()) return true;

        std::ofstream ostream(journal_path_, std::ios::app);
        if (!ostream.is_open()) return false;

        for (const auto &entry: unsaved_records_) {
            ostream << entry << "\n";
        }

        ostream.close();
        ostream.flush();
        unsaved_records_.clear();
        return true;
    }

    [[nodiscard]] bool destroy() {
        if (!exists()) return true; // User may have deleted journal during run-time
        unsaved_records_.clear();
        std::error_code ec;
        if (std::filesystem::resize_file(journal_path_, 0, ec); ec) return false;
        if (std::filesystem::remove(journal_path_, ec); ec) return false;
        return true;
    }

    [[nodiscard]] bool exists() const noexcept {
        std::error_code ec;
        return std::filesystem::exists(journal_path_, ec) && std::filesystem::file_size(journal_path_, ec) > 0;
        // exists but size == 0? destroy failed partially
    }

private:
    template<typename T>
    [[nodiscard]] static std::string tokenize_(T argument) {
        std::ostringstream oss;
        oss << argument;
        return std::to_string(oss.str().size()) + JOURNAL_TOKEN_DELIMITER + oss.str() + JOURNAL_TOKEN_DELIMITER;
    }

    [[nodiscard]] static Token extract_token_(const std::string &line, size_t &cursor_pos) {
        const size_t delimiter_pos = line.find(JOURNAL_TOKEN_DELIMITER, cursor_pos);

        if (delimiter_pos == std::string::npos) {
            cursor_pos = std::string::npos;
            return {false, ""};
        }

        const std::string length_token = line.substr(cursor_pos, delimiter_pos - cursor_pos);

        if (!std::all_of(length_token.begin(), length_token.end(), isdigit)) {
            cursor_pos = std::string::npos;
            return {false, ""};
        }

        const size_t value_length = std::stoull(length_token);

        if (line.size() - delimiter_pos - 1 <= value_length) {
            cursor_pos = std::string::npos;
            return {false, ""};
        }

        std::string value = line.substr(delimiter_pos + 1, value_length);
        const size_t new_cursor_pos = delimiter_pos + value_length + 2;
        cursor_pos = new_cursor_pos < line.size() ? new_cursor_pos : std::string::npos;

        return {true, std::move(value)};
    }

    static void handle_instruction_(const Instruction instruction, const std::vector<std::string> &args,
                                    FileIO &io_handler) {
        switch (instruction) {
            case Instruction::Append:
                if (args.empty()) return;
                io_handler.append(args[0]);
                break;
            case Instruction::Overwrite:
                if (args.size() < 2 || !std::all_of(args[0].begin(), args[0].end(), isdigit)) return;
                io_handler.overwrite(std::stoull(args[0]), args[1]);
                break;
            case Instruction::Erase:
                if (args.empty() || !std::all_of(args[0].begin(), args[0].end(), isdigit)) return;
                io_handler.erase(std::stoull(args[0]));
                break;
            case Instruction::Clear:
                io_handler.clear();
                break;
            default:
                break;
        }
    }

    const std::filesystem::path journal_path_;
    std::vector<std::string> unsaved_records_;
};

}

// --------------------------------------------------
//                     FILE MANAGER
// --------------------------------------------------

class filemanager {
public:
    /**
     * @brief Creates a new file manager instance
     * @param file_path Path of the file you want to manage
     */
    explicit filemanager(const std::filesystem::path &file_path) : file_io_(file_path),
                                                                   journal_(file_path) {
        if (file_io_.exists()) {
            file_io_.load();
        }

        if (journal_.exists()) {
            journal_.replay(file_io_);
        }
    }

    /**
     * @brief Reads the text line at a specified index
     * @param index The index of the line you want to read (0-index based)
     * @return The text at the specified index
     * @throw out_of_range If the given index is out of bounds
     */
    [[nodiscard]] std::string read(const size_t index) {
        if (index >= file_io_.size())
            THROW_INVALID_INDEX(index, file_io_.size());
        return file_io_.read(index);
    }

    /**
     * @brief Returns the text at the first line
     * @return The text of the first line
     * @throw out_of_range If the file is empty
     */
    [[nodiscard]] std::string front() {
        if (file_io_.empty())
            THROW_NO_FRONT_OR_BACK;
        return file_io_.front();
    }

    /**
     * @brief Returns the text at the last line
     * @return The text of the last line
     * @throw out_of_range If the file is empty
     */
    [[nodiscard]] std::string back() {
        if (file_io_.empty())
            THROW_NO_FRONT_OR_BACK;
        return file_io_.back();
    }

    /**
     * @brief Returns a copy of every line
     * @return Copy of every line
     * @note Vector is empty if file is empty.
     */
    [[nodiscard]] std::vector<std::string> all() {
        return file_io_.all();
    }

    /**
     * @brief Adds the given arguments to a new line at the end of the file
     * @param args The content you want to append
     * @throw invalid_argument If the text contains illegal characters (\\n or \\r)
     */
    template<typename... Args>
    void append(Args... args) {
        if (manager_state_ == internal::ManagerState::Locked)
            THROW_FILE_LOCKED;

        std::ostringstream oss;
        (oss << ... << args);

        if (!is_valid_input_(oss.str())) THROW_ILLEGAL_CHARACTER;

        file_io_.append(oss.str());
        journal_.record(internal::Instruction::Append, oss.str());
    }

    /**
     * @brief Overwrites the text at a specified index with the given arguments
     * @param index The index of the line you want to overwrite (0-index based)
     * @param args The content you want to overwrite the line with
     * @throw out_of_range If the given index is out of bounds
     * @throw invalid_argument If the text contains illegal characters ('\n' or '\r')
     */
    template<typename... Args>
    void overwrite(const size_t index, Args... args) {
        if (manager_state_ == internal::ManagerState::Locked)
            THROW_FILE_LOCKED;
        if (index >= file_io_.size())
            THROW_INVALID_INDEX(index, file_io_.size());

        std::ostringstream oss;
        (oss << ... << args);

        if (!is_valid_input_(oss.str()))
            THROW_ILLEGAL_CHARACTER;

        file_io_.overwrite(index, oss.str());
        journal_.record(internal::Instruction::Overwrite, index, oss.str());
    }

    /**
     * @brief Deletes a specified line, shifting all later lines down
     * @param index The index of the line you want to delete (0-index based)
     * @throw out_of_range If the given index is out of bounds
     */
    void erase(const size_t index) {
        if (manager_state_ == internal::ManagerState::Locked)
            THROW_FILE_LOCKED;
        if (index >= file_io_.size())
            THROW_INVALID_INDEX(index, file_io_.size());
        file_io_.erase(index);
        journal_.record(internal::Instruction::Erase, index);
    }

    /**
     * @brief Deletes every line, making the file empty
     */
    void clear() {
        if (manager_state_ == internal::ManagerState::Locked)
            THROW_FILE_LOCKED;
        file_io_.clear();
        journal_.record(internal::Instruction::Clear);
    }

    /**
     * @brief Saves changes so that they will be recoverable in the case of a crash
     * @return Whether the flush was successful
     * @note Doesn't update the managed file with the changes, call commit() for that.
     */
    [[nodiscard]] bool flush() {
        if (!journal_.flush()) {
            manager_state_ = internal::ManagerState::Locked;
            return false;
        }

        if (manager_state_ == internal::ManagerState::Locked) {
            manager_state_ = internal::ManagerState::Degraded;
        } else {
            manager_state_ = internal::ManagerState::Healthy;
        }

        return true;
    }

    /**
     * @brief Writes all changes to the file
     * @return Whether the commit was successful
     * @note Very expensive operation. Called when the file manager is destroyed. Consider using flush()
     * instead if you don't care whether the file will be readable by a human.
     */
    [[nodiscard]] bool commit() {
        if (const bool commited = file_io_.commit(); commited && journal_.destroy()) {
            manager_state_ = internal::ManagerState::Healthy;
            file_io_.load();
            return true;
        } else if (commited) {
            manager_state_ = internal::ManagerState::Locked;
            file_io_.load();
            THROW_COULDNT_DELETE_JOURNAL;
        }

        if (!journal_.flush()) {
            manager_state_ = internal::ManagerState::Locked;
            return false;
        }

        manager_state_ = internal::ManagerState::Degraded;
        return false;
    }

    /**
     * @brief Checks whether the file has no present lines
     * @return Whether the file has no present lines
     */
    [[nodiscard]] bool empty() const noexcept {
        return file_io_.empty();
    }

    /**
     * @brief Returns the number of present lines
     * @return Number of present lines
     */
    [[nodiscard]] size_t size() const noexcept {
        return file_io_.size();
    }

    filemanager(const filemanager &) = delete;
    filemanager &operator=(const filemanager &) = delete;

private:
    [[nodiscard]] static bool is_valid_input_(const std::string &text) {
        return text.find('\n') == std::string::npos && text.find('\r') == std::string::npos;
    }

    internal::FileIO file_io_;
    internal::Journal journal_;
    internal::ManagerState manager_state_ = internal::ManagerState::Healthy;
};
}

#undef THROW_FILE_LOCKED
#undef THROW_ILLEGAL_CHARACTER
#undef THROW_INVALID_INDEX
#undef THROW_NO_FRONT_OR_BACK
#undef THROW_COULDNT_OPEN_FILE

#undef JOURNAL_TOKEN_DELIMITER

#endif
