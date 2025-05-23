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
#ifndef LIBBITCOIN_DATABASE_LOCKS_FILE_LOCK_HPP
#define LIBBITCOIN_DATABASE_LOCKS_FILE_LOCK_HPP

#include <filesystem>
#include <bitcoin/system.hpp>
#include <bitcoin/database/define.hpp>

namespace libbitcoin {
namespace database {
    
/// This class is not thread safe, and does not throw.
class BCD_API file_lock
{
public:
    DELETE_COPY_MOVE_DESTRUCT(file_lock);

    /// Construction does not touch the file.
    file_lock(const std::filesystem::path& file) NOEXCEPT;

    /// Path to lock file.
    const std::filesystem::path& file() const NOEXCEPT;

protected:
    /// True if file exists.
    bool exists() const NOEXCEPT;

    /// True if file exists or was created.
    bool create() NOEXCEPT;

    /// True if file does not exist or was deleted.
    bool destroy() NOEXCEPT;

private:
    const std::filesystem::path file_;
};

} // namespace database
} // namespace libbitcoin

#endif
