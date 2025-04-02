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
#ifndef LIBBITCOIN_DATABASE_TABLES_ASSOCIATION_HPP
#define LIBBITCOIN_DATABASE_TABLES_ASSOCIATION_HPP

#include <bitcoin/system.hpp>
#include <bitcoin/database/define.hpp>
#include <bitcoin/database/tables/schema.hpp>
#include <bitcoin/database/primitives/primitives.hpp>

namespace libbitcoin {
namespace database {

/// Association between block hash and context.
struct association
{
    schema::height::link::integer link;
    system::hash_digest hash;
    system::chain::context context;

    struct key{};
    struct pos{};

    struct name_extractor
    {
        using result_type = size_t;

        inline const result_type& operator()(
            const association& item) const NOEXCEPT
        {
            return item.context.height;
        }

        inline result_type& operator()(association* item) const NOEXCEPT
        {
            BC_ASSERT_MSG(item, "null pointer");
            return item->context.height;
        }
    };
};

} // namespace database
} // namespace libbitcoin

#endif
