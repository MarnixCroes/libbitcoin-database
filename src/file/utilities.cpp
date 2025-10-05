/**
 * Copyright (c) 2011-2025 libbitcoin developers (see AUTHORS)
 *
 * This file is part of libbitcoin.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <bitcoin/database/file/utilities.hpp>

#if defined(HAVE_MSC)
    #include <io.h>
#endif
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <filesystem>
#include <ios>
#include <bitcoin/system.hpp>
#include <bitcoin/database/define.hpp>
#include <bitcoin/database/error.hpp>

namespace libbitcoin {
namespace database {
namespace file {

using namespace system;

// Many return values are redundant with return codes.
// There is no reason to obtain the return value of these non-discardables.
BC_PUSH_WARNING(DISCARDING_NON_DISCARDABLE)
BC_PUSH_WARNING(NO_IGNORE_RETURN_VALUE)
BC_PUSH_WARNING(NO_THROW_IN_NOEXCEPT)

inline path trim(const path& value) NOEXCEPT
{
    // Trailing slash may cause successful create_directories returning false.
    return system::trim_right_copy(value.string(), { "/", "\\", "\x20" });
}

// We don't use ec because it gets set (not found) when false, but no way to
// differentiate from false with failure code (and never a code on success).
bool is_directory(const path& directory) NOEXCEPT
{
    code ec{ system::error::errorno_t::no_error };
    return std::filesystem::is_directory(system::extended_path(directory), ec);
}

bool clear_directory(const path& directory) NOEXCEPT
{
    return !clear_directory_ex(directory);
}

code clear_directory_ex(const path& directory) NOEXCEPT
{
    code ec{ system::error::errorno_t::no_error };
    const auto path = system::extended_path(directory);
    std::filesystem::remove_all(path, ec);
    if (ec) return ec;
    std::filesystem::create_directories(path, ec);
    return ec;
}

bool create_directory(const path& directory) NOEXCEPT
{
    return !create_directory_ex(directory);
}

code create_directory_ex(const path& directory) NOEXCEPT
{
    code ec{ system::error::errorno_t::no_error };
    const auto path = system::extended_path(trim(directory));
    const auto created = std::filesystem::create_directories(path, ec);
    if (ec || created) return ec;
    return system::error::errorno_t::is_a_directory;
}

// We don't use ec because it gets set (not found) when false, but no way to
// differentiate from false with failure code (and never a code on success).
bool is_file(const path& filename) NOEXCEPT
{
    code ec{ system::error::errorno_t::no_error };
    return std::filesystem::is_regular_file(filename, ec);
}

bool create_file(const path& filename) NOEXCEPT
{
    return !create_file_ex(filename);
}

// std::filesystem does not provide file creation.
code create_file_ex(const path& filename) NOEXCEPT
{
    try
    {
        // Throws.
        ofstream file(filename);

        // Allow throw.
        file.exceptions(std::ifstream::failbit);

        // noexcept.
        if (!file.good())
            return system::error::errorno_t::not_a_stream;

        // Sets failbit (but not noexcept).
        file.close();

        // noexcept.
        return file.good() ?
            system::error::errorno_t::no_error :
            system::error::errorno_t::not_a_stream;
    }
    catch (const std::ios_base::failure& e)
    {
        // Prefer throw, since we get a platform code.
        return e.code();
    }
}

bool create_file(const path& to, const uint8_t* data, size_t size) NOEXCEPT
{
    return !create_file_ex(to, data, size);
}

// std::filesystem does not provide file creation.
code create_file_ex(const path& to, const uint8_t* data, size_t size) NOEXCEPT
{
    // Binary mode on Windows ensures that \n not replaced with \r\n.
    try
    {
        // Throws.
        ofstream file(to, std::ios_base::binary);

        // Allow throw.
        file.exceptions(std::ifstream::failbit);

        // noexcept.
        if (!file.good())
            return system::error::errorno_t::not_a_stream;

        // May throw.
        file.write(pointer_cast<const char>(data), size);

        // noexcept.
        if (!file.good())
            return system::error::errorno_t::not_a_stream;

        // Sets failbit (but not noexcept).
        file.close();

        // noexcept.
        return file.good() ?
            system::error::errorno_t::no_error :
            system::error::errorno_t::stream_timeout;
    }
    catch (const std::ios_base::failure& e)
    {
        // Prefer throw, since we get a platform code.
        return e.code();
    }
}

// directory|file
bool remove(const path& name) NOEXCEPT
{
    return !remove_ex(name);
}

// directory|file
// Error code is not set if file did not exist.
code remove_ex(const path& name) NOEXCEPT
{
    code ec{ system::error::errorno_t::no_error };
    std::filesystem::remove(system::extended_path(name), ec);
    return ec;
}

// directory|file
bool rename(const path& from, const path& to) NOEXCEPT
{
    return !rename_ex(from, to);
}

// directory|file
code rename_ex(const path& from, const path& to) NOEXCEPT
{
    code ec{ system::error::errorno_t::no_error };
    std::filesystem::rename(system::extended_path(from),
        system::extended_path(to), ec);

    return ec;
}

// file
bool copy(const path& from, const path& to) NOEXCEPT
{
    return !copy_ex(from, to);
}

// file
code copy_ex(const path& from, const path& to) NOEXCEPT
{
    code ec{ system::error::errorno_t::no_error };
    std::filesystem::copy_file(system::extended_path(from),
        system::extended_path(to), ec);

    return ec;
}

// directory
bool copy_directory(const path& from, const path& to) NOEXCEPT
{
    return !copy_directory_ex(from, to);
}

// directory
code copy_directory_ex(const path& from, const path& to) NOEXCEPT
{
    if (file::is_directory(to))
        return system::error::errorno_t::is_a_directory;

    if (!file::is_directory(from))
        return system::error::errorno_t::not_a_directory;

    code ec{ system::error::errorno_t::no_error };
    std::filesystem::copy(system::extended_path(from),
        system::extended_path(to), ec);

    return ec;
}

// File descriptor functions required for memory mapping.

#if defined(HAVE_MSC) || !defined(HAVE_APPLE)
    #define MSC_OR_NOAPPLE(parameter) parameter
#else
    #define MSC_OR_NOAPPLE(parameter)
#endif

int open(const path& filename, bool MSC_OR_NOAPPLE(random)) NOEXCEPT
{
    const auto path = system::extended_path(filename);
    int file_descriptor{};

#if defined(HAVE_MSC)
    // _wsopen_s and wstring do not throw (but are unannotated).
    // sets file_descriptor = -1 and errno on error.
    const auto access = (random ? _O_RANDOM : _O_SEQUENTIAL);
    ::_wsopen_s(&file_descriptor, path.c_str(),
        O_RDWR | _O_BINARY | access, _SH_DENYWR, _S_IREAD | _S_IWRITE);
#else
    // open sets errno on failure.
    file_descriptor = ::open(path.c_str(), O_RDWR, S_IRUSR | S_IWUSR);

#if !defined(HAVE_APPLE)
    if (file_descriptor != -1)
    {
        // _O_RANDOM equivalent, posix_fadvise returns error on failure.
        const auto advice = random ? POSIX_FADV_RANDOM : POSIX_FADV_SEQUENTIAL;
        const auto result = ::posix_fadvise(file_descriptor, 0, 0, advice);
        if (!is_zero(result))
        {
            close(file_descriptor);
            file_descriptor = -1;
            errno = result;
        }
        else
        {
            // _SH_DENYWR equivalent.
            const struct flock lock{ F_WRLCK, SEEK_SET, 0, 0 };
            if (::fcntl(file_descriptor, F_SETLK, &lock) == -1)
            {
                const auto last = errno;
                close(file_descriptor);
                file_descriptor = -1;
                errno = last;
            }
        }
    }
#endif // !HAVE_APPLE
#endif // HAVE_MSC

    return file_descriptor;
}

code open_ex(int& file_descriptor, const path& filename, bool random) NOEXCEPT
{
    system::error::clear_errno();
    file_descriptor = open(filename, random);
    return system::error::get_errno();
}

bool close(int file_descriptor) NOEXCEPT
{
    // close and _close() set errno on failure.
#if defined(HAVE_MSC)
    return is_zero(::_close(file_descriptor));
#else
    return is_zero(::close(file_descriptor));
#endif
}

code close_ex(int file_descriptor) NOEXCEPT
{
    system::error::clear_errno();
    close(file_descriptor);
    return system::error::get_errno();
}

bool size(size_t& out, int file_descriptor) NOEXCEPT
{
    if (file_descriptor == -1)
    {
        system::error::set_errno(system::error::errorno_t::invalid_argument);
        return false;
    }

    // _fstat64, _fstat32 and fstat set errno on failure.
    // This is required because off_t is defined as long, which is 32|64 bits
    // in msvc and 64 bits in linux/osx, and stat contains off_t.
#if defined(HAVE_MSC)
#if defined(HAVE_X64)
    struct _stat64 sbuf{};
    if (_fstat64(file_descriptor, &sbuf) == -1)
        return false;
#else
    struct _stat32 sbuf{};
    if (_fstat32(file_descriptor, &sbuf) == -1)
        return false;
#endif
#else
    // Limited to 32 bit files on 32 bit systems.
    struct stat sbuf{};
    if (fstat(file_descriptor, &sbuf) == -1)
        return false;
#endif
    if (is_limited<size_t>(sbuf.st_size))
    {
        system::error::set_errno(system::error::errorno_t::value_too_large);
        return false;
    }

    out = sign_cast<size_t>(sbuf.st_size);
    return true;
}

code size_ex(size_t& out, int file_descriptor) NOEXCEPT
{
    system::error::clear_errno();
    size(out, file_descriptor);
    return system::error::get_errno();
}

bool size(size_t& out, const std::filesystem::path& filename) NOEXCEPT
{
    return !size_ex(out, filename);
}

code size_ex(size_t& out, const std::filesystem::path& filename) NOEXCEPT
{
    code ec{ system::error::errorno_t::no_error };
    const auto size = std::filesystem::file_size(
        system::extended_path(filename), ec);

    if (ec) return ec;
    if (is_limited<size_t>(size))
        return system::error::errorno_t::value_too_large;

    out = possible_narrow_cast<size_t>(size);
    return ec;
}

bool space(size_t& out, const path& filename) NOEXCEPT
{
    return !space_ex(out, filename);
}

code space_ex(size_t& out, const path& filename) NOEXCEPT
{
    code ec{ system::error::errorno_t::no_error };
    const auto space = std::filesystem::space(
        system::extended_path(filename), ec);
    if (ec) return ec;
    if (is_limited<size_t>(space.available))
        return system::error::errorno_t::value_too_large;

    out = possible_narrow_cast<size_t>(space.available);
    return ec;
}

BC_POP_WARNING()
BC_POP_WARNING()
BC_POP_WARNING()

} // namespace file
} // namespace database
} // namespace libbitcoin
