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
#include "../test.hpp"
#include "../mocks/blocks.hpp"
#include "../mocks/chunk_store.hpp"

struct query_initialize_setup_fixture
{
    DELETE_COPY_MOVE(query_initialize_setup_fixture);
    BC_PUSH_WARNING(NO_THROW_IN_NOEXCEPT)

    query_initialize_setup_fixture() NOEXCEPT
    {
        BOOST_REQUIRE(test::clear(test::directory));
    }

    ~query_initialize_setup_fixture() NOEXCEPT
    {
        BOOST_REQUIRE(test::clear(test::directory));
    }

    BC_POP_WARNING()
};

BOOST_FIXTURE_TEST_SUITE(query_initialize_tests, query_initialize_setup_fixture)

// nop event handler.
const auto events_handler = [](auto, auto) {};

BOOST_AUTO_TEST_CASE(query_initialize__blocks__verify__expected)
{
    BOOST_REQUIRE_EQUAL(test::block1.hash(), test::block1_hash);
    BOOST_REQUIRE_EQUAL(test::block2.hash(), test::block2_hash);
    BOOST_REQUIRE_EQUAL(test::block3.hash(), test::block3_hash);
    BOOST_REQUIRE_EQUAL(test::block1.header().previous_block_hash(), test::genesis.hash());
    BOOST_REQUIRE_EQUAL(test::block2.header().previous_block_hash(), test::block1.hash());
    BOOST_REQUIRE_EQUAL(test::block3.header().previous_block_hash(), test::block2.hash());
}

// initialize

BOOST_AUTO_TEST_CASE(query_initialize__initialize__is_initialized__true)
{
    settings settings{};
    settings.path = TEST_DIRECTORY;
    test::chunk_store store{ settings };
    test::query_accessor query{ store };
    BOOST_REQUIRE_EQUAL(store.create(events_handler), error::success);
    BOOST_REQUIRE(query.initialize(test::genesis));
    BOOST_REQUIRE(query.is_initialized());
}

// is_initialized

BOOST_AUTO_TEST_CASE(query_initialize__is_initialized__default__false)
{
    settings settings{};
    settings.path = TEST_DIRECTORY;
    test::chunk_store store{ settings };
    test::query_accessor query{ store };
    BOOST_REQUIRE_EQUAL(store.create(events_handler), error::success);
    BOOST_REQUIRE(!query.is_initialized());
}

BOOST_AUTO_TEST_CASE(query_initialize__is_initialized__unconfirmed__false)
{
    settings settings{};
    settings.path = TEST_DIRECTORY;
    test::chunk_store store{ settings };
    test::query_accessor query{ store };
    BOOST_REQUIRE_EQUAL(store.create(events_handler), error::success);
    BOOST_REQUIRE(query.set(test::genesis, test::context, false, false));
    BOOST_REQUIRE(query.push_candidate(query.to_header(test::genesis.hash())));
    BOOST_REQUIRE(!query.is_initialized());
}

BOOST_AUTO_TEST_CASE(query_initialize__is_initialized__candidate__false)
{
    settings settings{};
    settings.path = TEST_DIRECTORY;
    test::chunk_store store{ settings };
    test::query_accessor query{ store };
    BOOST_REQUIRE_EQUAL(store.create(events_handler), error::success);
    BOOST_REQUIRE(query.set(test::genesis, test::context, false, false));
    BOOST_REQUIRE(query.push_candidate(query.to_header(test::genesis.hash())));
    BOOST_REQUIRE(!query.is_initialized());
}

BOOST_AUTO_TEST_CASE(query_initialize__is_initialized__confirmed__false)
{
    settings settings{};
    settings.path = TEST_DIRECTORY;
    test::chunk_store store{ settings };
    test::query_accessor query{ store };
    BOOST_REQUIRE_EQUAL(store.create(events_handler), error::success);
    BOOST_REQUIRE(query.set(test::genesis, test::context, false, false));
    BOOST_REQUIRE(query.push_confirmed(query.to_header(test::genesis.hash()), false));
    BOOST_REQUIRE(!query.is_initialized());
}

BOOST_AUTO_TEST_CASE(query_initialize__is_initialized__candidate_and_confirmed__true)
{
    settings settings{};
    settings.path = TEST_DIRECTORY;
    test::chunk_store store{ settings };
    test::query_accessor query{ store };
    BOOST_REQUIRE_EQUAL(store.create(events_handler), error::success);
    BOOST_REQUIRE(query.set(test::genesis, test::context, false, false));
    BOOST_REQUIRE(query.push_candidate(query.to_header(test::genesis.hash())));
    BOOST_REQUIRE(query.push_confirmed(query.to_header(test::genesis.hash()), false));
    BOOST_REQUIRE(query.is_initialized());
}

// get_top

BOOST_AUTO_TEST_CASE(query_initialize__get_top__genesis_confirmed__0)
{
    settings settings{};
    settings.path = TEST_DIRECTORY;
    test::chunk_store store{ settings };
    test::query_accessor query{ store };
    BOOST_REQUIRE_EQUAL(store.create(events_handler), error::success);
    BOOST_REQUIRE(query.set(test::genesis, test::context, false, false));
    BOOST_REQUIRE(query.push_confirmed(query.to_header(test::genesis.hash()), false));
    ////BOOST_REQUIRE(query.push_candidate(query.to_header(genesis.hash())));
    BOOST_REQUIRE_EQUAL(query.get_top_confirmed(), 0u);
}

BOOST_AUTO_TEST_CASE(query_initialize__get_top__three_blocks_confirmed__2)
{
    settings settings{};
    settings.path = TEST_DIRECTORY;
    test::chunk_store store{ settings };
    test::query_accessor query{ store };
    BOOST_REQUIRE_EQUAL(store.create(events_handler), error::success);
    BOOST_REQUIRE(query.set(test::genesis, test::context, false, false));
    BOOST_REQUIRE(query.set(test::block1, test::context, false, false));
    BOOST_REQUIRE(query.set(test::block2, test::context, false, false));
    BOOST_REQUIRE(query.push_candidate(query.to_header(test::genesis.hash())));
    BOOST_REQUIRE(query.push_confirmed(query.to_header(test::genesis.hash()), false));
    BOOST_REQUIRE(query.push_confirmed(query.to_header(test::block1.hash()), false));
    BOOST_REQUIRE(query.push_confirmed(query.to_header(test::block2.hash()), false));
    BOOST_REQUIRE_EQUAL(query.get_top_confirmed(), 2u);
    BOOST_REQUIRE_EQUAL(query.get_top_candidate(), 0u);
}

// get_top_candidate

BOOST_AUTO_TEST_CASE(query_initialize__get_top_candidate__genesis_candidated__0)
{
    settings settings{};
    settings.path = TEST_DIRECTORY;
    test::chunk_store store{ settings };
    test::query_accessor query{ store };
    BOOST_REQUIRE_EQUAL(store.create(events_handler), error::success);
    BOOST_REQUIRE(query.set(test::genesis, test::context, false, false));
    BOOST_REQUIRE(query.push_candidate(query.to_header(test::genesis.hash())));
    ////BOOST_REQUIRE(query.push_confirmed(query.to_header(test::genesis.hash()), false));
    BOOST_REQUIRE_EQUAL(query.get_top_candidate(), 0u);
}

BOOST_AUTO_TEST_CASE(query_initialize__get_top__three_blocks_candidated__2)
{
    settings settings{};
    settings.path = TEST_DIRECTORY;
    test::chunk_store store{ settings };
    test::query_accessor query{ store };
    BOOST_REQUIRE_EQUAL(store.create(events_handler), error::success);
    BOOST_REQUIRE(query.set(test::genesis, test::context, false, false));
    BOOST_REQUIRE(query.set(test::block1, test::context, false, false));
    BOOST_REQUIRE(query.set(test::block2, test::context, false, false));
    BOOST_REQUIRE(query.push_candidate(query.to_header(test::genesis.hash())));
    BOOST_REQUIRE(query.push_confirmed(query.to_header(test::genesis.hash()), false));
    BOOST_REQUIRE(query.push_candidate(query.to_header(test::block1.hash())));
    BOOST_REQUIRE(query.push_candidate(query.to_header(test::block2.hash())));
    BOOST_REQUIRE_EQUAL(query.get_top_confirmed(), 0u);
    BOOST_REQUIRE_EQUAL(query.get_top_candidate(), 2u);
}

// get_fork

BOOST_AUTO_TEST_CASE(query_initialize__get_fork__initialized__0)
{
    settings settings{};
    settings.path = TEST_DIRECTORY;
    test::chunk_store store{ settings };
    test::query_accessor query{ store };
    BOOST_REQUIRE_EQUAL(store.create(events_handler), error::success);
    BOOST_REQUIRE(query.set(test::genesis, test::context, false, false));
    BOOST_REQUIRE(query.push_candidate(query.to_header(test::genesis.hash())));
    BOOST_REQUIRE(query.push_confirmed(query.to_header(test::genesis.hash()), false));
    BOOST_REQUIRE_EQUAL(query.get_fork(), 0u);
}

BOOST_AUTO_TEST_CASE(query_initialize__get_fork__candidate_ahead__expected)
{
    settings settings{};
    settings.path = TEST_DIRECTORY;
    test::chunk_store store{ settings };
    test::query_accessor query{ store };
    BOOST_REQUIRE_EQUAL(store.create(events_handler), error::success);
    BOOST_REQUIRE(query.set(test::genesis, test::context, false, false));
    BOOST_REQUIRE(query.set(test::block1, test::context, false, false));
    BOOST_REQUIRE(query.set(test::block2, test::context, false, false));
    BOOST_REQUIRE(query.push_candidate(query.to_header(test::genesis.hash())));
    BOOST_REQUIRE(query.push_confirmed(query.to_header(test::genesis.hash()), false));
    BOOST_REQUIRE(query.push_candidate(query.to_header(test::block1.hash())));
    BOOST_REQUIRE(query.push_confirmed(query.to_header(test::block1.hash()), false));
    BOOST_REQUIRE(query.push_candidate(query.to_header(test::block2.hash())));
    BOOST_REQUIRE_EQUAL(query.get_fork(), 1u);
}

BOOST_AUTO_TEST_CASE(query_initialize__get_fork__confirmed_ahead__expected)
{
    settings settings{};
    settings.path = TEST_DIRECTORY;
    test::chunk_store store{ settings };
    test::query_accessor query{ store };
    BOOST_REQUIRE_EQUAL(store.create(events_handler), error::success);
    BOOST_REQUIRE(query.set(test::genesis, test::context, false, false));
    BOOST_REQUIRE(query.set(test::block1, test::context, false, false));
    BOOST_REQUIRE(query.set(test::block2, test::context, false, false));
    BOOST_REQUIRE(query.push_candidate(query.to_header(test::genesis.hash())));
    BOOST_REQUIRE(query.push_confirmed(query.to_header(test::genesis.hash()), false));
    BOOST_REQUIRE(query.push_candidate(query.to_header(test::block1.hash())));
    BOOST_REQUIRE(query.push_confirmed(query.to_header(test::block1.hash()), false));
    BOOST_REQUIRE(query.push_confirmed(query.to_header(test::block2.hash()), false));
    BOOST_REQUIRE_EQUAL(query.get_fork(), 1u);
}

// get_top_associated_from/get_top_associated

BOOST_AUTO_TEST_CASE(query_initialize__get_top_associated_from__terminal__max_size_t)
{
    settings settings{};
    settings.path = TEST_DIRECTORY;
    test::chunk_store store{ settings };
    test::query_accessor query{ store };
    BOOST_REQUIRE_EQUAL(store.create(events_handler), error::success);
    BOOST_REQUIRE(query.initialize(test::genesis));
    BOOST_REQUIRE_EQUAL(query.get_top_associated_from(max_size_t), max_size_t);
    BOOST_REQUIRE_EQUAL(query.get_top_associated_from(height_link::terminal), max_size_t);
    BOOST_REQUIRE_EQUAL(query.get_top_associated(), 0u);

    // unassociated, but correct.
    BOOST_REQUIRE_EQUAL(query.get_top_associated_from(42), 42u);
}

BOOST_AUTO_TEST_CASE(query_initialize__get_top_associated_from__initialized__zero)
{
    settings settings{};
    settings.path = TEST_DIRECTORY;
    test::chunk_store store{ settings };
    test::query_accessor query{ store };
    BOOST_REQUIRE_EQUAL(store.create(events_handler), error::success);
    BOOST_REQUIRE(query.initialize(test::genesis));
    BOOST_REQUIRE_EQUAL(query.get_top_associated(), 0u);
    BOOST_REQUIRE_EQUAL(query.get_top_associated_from(0), 0u);

    // unassociated, but correct.
    BOOST_REQUIRE_EQUAL(query.get_top_associated_from(42), 42u);
}

BOOST_AUTO_TEST_CASE(query_initialize__get_top_associated_from__non_candidate__expected)
{
    settings settings{};
    settings.path = TEST_DIRECTORY;
    test::chunk_store store{ settings };
    test::query_accessor query{ store };
    BOOST_REQUIRE_EQUAL(store.create(events_handler), error::success);
    BOOST_REQUIRE(query.initialize(test::genesis));
    BOOST_REQUIRE(query.set(test::block1, test::context, false, false));
    BOOST_REQUIRE(query.set(test::block2, test::context, false, false));
    BOOST_REQUIRE(query.set(test::block3, test::context, false, false));
    BOOST_REQUIRE(query.push_candidate(query.to_header(test::block1.hash())));
    BOOST_REQUIRE(query.push_candidate(query.to_header(test::block2.hash())));
    BOOST_REQUIRE_EQUAL(query.get_top_associated(), 2u);
    BOOST_REQUIRE_EQUAL(query.get_top_associated_from(0), 2u);
    BOOST_REQUIRE_EQUAL(query.get_top_associated_from(1), 2u);
    BOOST_REQUIRE_EQUAL(query.get_top_associated_from(2), 2u);

    // unassociated, but correct.
    BOOST_REQUIRE_EQUAL(query.get_top_associated_from(3), 3u);
}

BOOST_AUTO_TEST_CASE(query_initialize__get_top_associated_from__gapped_candidate__expected)
{
    settings settings{};
    settings.path = TEST_DIRECTORY;
    test::chunk_store store{ settings };
    test::query_accessor query{ store };
    BOOST_REQUIRE_EQUAL(store.create(events_handler), error::success);
    BOOST_REQUIRE(query.initialize(test::genesis));
    BOOST_REQUIRE(query.set(test::block1, test::context, false, false));
    BOOST_REQUIRE(query.set(test::block2.header(), test::context, false)); // header only
    BOOST_REQUIRE(query.set(test::block3, test::context, false, false));
    BOOST_REQUIRE(query.push_candidate(query.to_header(test::block1.hash())));
    BOOST_REQUIRE(query.push_candidate(query.to_header(test::block2.hash())));
    BOOST_REQUIRE(query.push_candidate(query.to_header(test::block3.hash())));
    BOOST_REQUIRE_EQUAL(query.get_top_associated(), 1u);
    BOOST_REQUIRE_EQUAL(query.get_top_associated_from(0), 1u);
    BOOST_REQUIRE_EQUAL(query.get_top_associated_from(1), 1u);

    // gapped, but correct.
    BOOST_REQUIRE_EQUAL(query.get_top_associated_from(2), 3u);

    // unassociated, but correct.
    BOOST_REQUIRE_EQUAL(query.get_top_associated_from(3), 3u);
}

// get_unassociated_above/get_all_unassociated

BOOST_AUTO_TEST_CASE(query_initialize__get_unassociated_above__initialized__empty)
{
    settings settings{};
    settings.path = TEST_DIRECTORY;
    test::chunk_store store{ settings };
    test::query_accessor query{ store };
    BOOST_REQUIRE_EQUAL(store.create(events_handler), error::success);
    BOOST_REQUIRE(query.initialize(test::genesis));
    BOOST_REQUIRE(query.get_all_unassociated().empty());
    BOOST_REQUIRE(query.get_unassociated_above(0).empty());
    BOOST_REQUIRE(query.get_unassociated_above(1).empty());
}

BOOST_AUTO_TEST_CASE(query_initialize__get_unassociated_above__gapped_candidate__expected)
{
    settings settings{};
    settings.path = TEST_DIRECTORY;
    test::chunk_store store{ settings };
    test::query_accessor query{ store };
    BOOST_REQUIRE_EQUAL(store.create(events_handler), error::success);

    constexpr database::context context2
    {
        0x01020302, // flags
        0x00121312, // height (3 bytes)
        0x21222322  // mtp
    };
    constexpr database::context context3
    {
        0x01020303, // flags
        0x00121313, // height (3 bytes)
        0x21222323  // mtp
    };
    BOOST_REQUIRE(query.initialize(test::genesis));            // associated
    BOOST_REQUIRE(query.set(test::block1, test::context, false, false));     // associated
    BOOST_REQUIRE(query.set(test::block2.header(), context2, false)); // header only
    BOOST_REQUIRE(query.set(test::block3.header(), context3, false)); // header only
    BOOST_REQUIRE(query.push_candidate(query.to_header(test::block1.hash())));
    BOOST_REQUIRE(query.push_candidate(query.to_header(test::block2.hash())));
    BOOST_REQUIRE(query.push_candidate(query.to_header(test::block3.hash())));

    // There are two unassociated blocks above genesis (fork point).
    BOOST_REQUIRE_EQUAL(query.get_all_unassociated().size(), 2u);

    const auto unassociated0 = query.get_unassociated_above(0);
    BOOST_REQUIRE(!unassociated0.empty());
    BOOST_REQUIRE_EQUAL(unassociated0.size(), 2u);

    const associations copy{ unassociated0.begin(), unassociated0.end() };
    BOOST_REQUIRE(!copy.empty());
    BOOST_REQUIRE_EQUAL(copy.size(), 2u);

    BOOST_REQUIRE(copy.find(system::null_hash) == copy.end());
    BOOST_REQUIRE(copy.find(system::null_hash) == copy.end());
    BOOST_REQUIRE(copy.find(zero) == copy.pos_end());

    const auto it2 = unassociated0.find(test::block2.hash());
    BOOST_REQUIRE(it2 != unassociated0.end());
    BOOST_REQUIRE_EQUAL(it2->link, 2u);
    BOOST_REQUIRE_EQUAL(it2->context.flags, context2.flags);
    BOOST_REQUIRE_EQUAL(it2->context.timestamp, test::block2.header().timestamp());
    BOOST_REQUIRE_EQUAL(it2->context.median_time_past, context2.mtp);
    BOOST_REQUIRE_EQUAL(it2->context.height, context2.height);

    const auto it3 = unassociated0.find(test::block3.hash());
    BOOST_REQUIRE(it3 != unassociated0.end());
    BOOST_REQUIRE_EQUAL(it3->link, 3u);
    BOOST_REQUIRE_EQUAL(it3->context.flags, context3.flags);
    BOOST_REQUIRE_EQUAL(it3->context.timestamp, test::block3.header().timestamp());
    BOOST_REQUIRE_EQUAL(it3->context.median_time_past, context3.mtp);
    BOOST_REQUIRE_EQUAL(it3->context.height, context3.height);

    const auto unassociated1 = query.get_unassociated_above(1);
    BOOST_REQUIRE_EQUAL(unassociated1.size(), 2u);
    BOOST_REQUIRE(query.get_unassociated_above(1, 0).empty());
    BOOST_REQUIRE_EQUAL(query.get_unassociated_above(1, 1).size(), 1u);

    const auto it2s = unassociated1.find(context2.height);
    BOOST_REQUIRE(it2s != unassociated1.pos_end());
    BOOST_REQUIRE_EQUAL(it2s->context.flags, context2.flags);
    BOOST_REQUIRE_EQUAL(it2s->context.timestamp, test::block2.header().timestamp());
    BOOST_REQUIRE_EQUAL(it2s->context.median_time_past, context2.mtp);
    BOOST_REQUIRE_EQUAL(it2s->context.height, context2.height);

    const auto it3s = unassociated1.find(context3.height);
    BOOST_REQUIRE(it3s != unassociated1.pos_end());
    BOOST_REQUIRE_EQUAL(it3s->context.flags, context3.flags);
    BOOST_REQUIRE_EQUAL(it3s->context.timestamp, test::block3.header().timestamp());
    BOOST_REQUIRE_EQUAL(it3s->context.median_time_past, context3.mtp);
    BOOST_REQUIRE_EQUAL(it3s->context.height, context3.height);
    BOOST_REQUIRE_EQUAL(unassociated1.top().height, context3.height);

    const auto unassociated2 = query.get_unassociated_above(2);
    BOOST_REQUIRE_EQUAL(unassociated2.size(), 1u);

    const auto it3a = unassociated2.find(test::block3.hash());
    BOOST_REQUIRE(it3a != unassociated2.end());
    BOOST_REQUIRE_EQUAL(it3a->context.flags, context3.flags);
    BOOST_REQUIRE_EQUAL(it3a->context.timestamp, test::block3.header().timestamp());
    BOOST_REQUIRE_EQUAL(it3a->context.median_time_past, context3.mtp);
    BOOST_REQUIRE_EQUAL(it3a->context.height, context3.height);
    BOOST_REQUIRE_EQUAL(unassociated2.top().height, context3.height);

    const auto unassociated3 = query.get_unassociated_above(3);
    BOOST_REQUIRE_EQUAL(unassociated3.size(), 0u);

    // There are two unassociated blocks above block 1 (new fork point).
    BOOST_REQUIRE(query.set(test::block1, false));
    BOOST_REQUIRE(query.push_confirmed(query.to_header(test::block1.hash()), false));
    BOOST_REQUIRE_EQUAL(query.get_all_unassociated().size(), 2u);

    // There is one unassociated block above block 2 (new fork point).
    BOOST_REQUIRE(query.set(test::block2, false));
    BOOST_REQUIRE(query.push_confirmed(query.to_header(test::block2.hash()), false));
    BOOST_REQUIRE_EQUAL(query.get_all_unassociated().size(), 1u);

    // There are no unassociated blocks above block 3 (new fork point).
    BOOST_REQUIRE(query.set(test::block3, false));
    BOOST_REQUIRE(query.push_confirmed(query.to_header(test::block3.hash()), false));
    BOOST_REQUIRE_EQUAL(query.get_all_unassociated().size(), 0u);
}

// get_unassociated_count_above/get_unassociated_count

BOOST_AUTO_TEST_CASE(query_initialize__get_unassociated_count_above__gapped_candidate__expected)
{
    settings settings{};
    settings.path = TEST_DIRECTORY;
    test::chunk_store store{ settings };
    test::query_accessor query{ store };
    BOOST_REQUIRE_EQUAL(store.create(events_handler), error::success);

    constexpr database::context context2
    {
        0x01020302, // flags
        0x00121312, // height (3 bytes)
        0x21222322  // mtp
    };
    constexpr database::context context3
    {
        0x01020303, // flags
        0x00121313, // height (3 bytes)
        0x21222323  // mtp
    };
    BOOST_REQUIRE(query.initialize(test::genesis));                   // associated
    BOOST_REQUIRE(query.set(test::block1, test::context, false, false));     // associated
    BOOST_REQUIRE(query.set(test::block2.header(), context2, false)); // header only
    BOOST_REQUIRE(query.set(test::block3.header(), context3, false)); // header only
    BOOST_REQUIRE(query.push_candidate(query.to_header(test::block1.hash())));
    BOOST_REQUIRE(query.push_candidate(query.to_header(test::block2.hash())));
    BOOST_REQUIRE(query.push_candidate(query.to_header(test::block3.hash())));

    // There are two unassociated blocks above genesis.
    BOOST_REQUIRE_EQUAL(query.get_unassociated_count(), 2u);
    BOOST_REQUIRE_EQUAL(query.get_unassociated_count_above(0), 2u);
    BOOST_REQUIRE_EQUAL(query.get_unassociated_count_above(1), 2u);
    BOOST_REQUIRE_EQUAL(query.get_unassociated_count_above(2), 1u);
    BOOST_REQUIRE_EQUAL(query.get_unassociated_count_above(3), 0u);

    BOOST_REQUIRE_EQUAL(query.get_unassociated_count_above(0, 0), 0u);
    BOOST_REQUIRE_EQUAL(query.get_unassociated_count_above(0, 1), 1u);
    BOOST_REQUIRE_EQUAL(query.get_unassociated_count_above(0, 2), 2u);
    BOOST_REQUIRE_EQUAL(query.get_unassociated_count_above(0, 3), 2u);
    BOOST_REQUIRE_EQUAL(query.get_unassociated_count_above(1, 0), 0u);
    BOOST_REQUIRE_EQUAL(query.get_unassociated_count_above(1, 1), 1u);
    BOOST_REQUIRE_EQUAL(query.get_unassociated_count_above(1, 2), 2u);
    BOOST_REQUIRE_EQUAL(query.get_unassociated_count_above(1, 3), 2u);
    BOOST_REQUIRE_EQUAL(query.get_unassociated_count_above(2, 0), 0u);
    BOOST_REQUIRE_EQUAL(query.get_unassociated_count_above(2, 1), 1u);
    BOOST_REQUIRE_EQUAL(query.get_unassociated_count_above(2, 2), 1u);
    BOOST_REQUIRE_EQUAL(query.get_unassociated_count_above(3, 0), 0u);
    BOOST_REQUIRE_EQUAL(query.get_unassociated_count_above(3, 1), 0u);

    // There is one unassociated block at block 2.
    BOOST_REQUIRE(query.set(test::block3, false));                    // associated
    BOOST_REQUIRE_EQUAL(query.get_unassociated_count(), 1u);
    BOOST_REQUIRE_EQUAL(query.get_unassociated_count_above(0), 1u);
    BOOST_REQUIRE_EQUAL(query.get_unassociated_count_above(1), 1u);
    BOOST_REQUIRE_EQUAL(query.get_unassociated_count_above(2), 0u);
    BOOST_REQUIRE_EQUAL(query.get_unassociated_count_above(3), 0u);

    // There are no unassociated blocks.
    BOOST_REQUIRE(query.set(test::block2, false));                    // associated
    BOOST_REQUIRE_EQUAL(query.get_unassociated_count(), 0u);
    BOOST_REQUIRE_EQUAL(query.get_unassociated_count_above(0), 0u);
    BOOST_REQUIRE_EQUAL(query.get_unassociated_count_above(1), 0u);
    BOOST_REQUIRE_EQUAL(query.get_unassociated_count_above(2), 0u);
    BOOST_REQUIRE_EQUAL(query.get_unassociated_count_above(3), 0u);
}

// get_candidate_hashes

BOOST_AUTO_TEST_CASE(query_initialize__get_candidate_hashes__initialized__one)
{
    settings settings{};
    settings.path = TEST_DIRECTORY;
    test::chunk_store store{ settings };
    test::query_accessor query{ store };
    BOOST_REQUIRE_EQUAL(store.create(events_handler), error::success);
    BOOST_REQUIRE(query.initialize(test::genesis));
    BOOST_REQUIRE_EQUAL(query.get_candidate_hashes({ 0, 1, 2, 4, 6, 8 }).size(), 1u);
}

BOOST_AUTO_TEST_CASE(query_initialize__get_candidate_hashes__gapped__expected)
{
    settings settings{};
    settings.path = TEST_DIRECTORY;
    test::chunk_store store{ settings };
    test::query_accessor query{ store };
    BOOST_REQUIRE_EQUAL(store.create(events_handler), error::success);
    BOOST_REQUIRE(query.initialize(test::genesis));
    BOOST_REQUIRE(query.set(test::block1, test::context, false, false));
    BOOST_REQUIRE(query.set(test::block2, test::context, false, false));
    BOOST_REQUIRE(query.set(test::block3, test::context, false, false));
    BOOST_REQUIRE(query.push_candidate(query.to_header(test::block1.hash())));
    BOOST_REQUIRE(query.push_candidate(query.to_header(test::block2.hash())));
    BOOST_REQUIRE(query.push_candidate(query.to_header(test::block3.hash())));
    const auto locator = query.get_candidate_hashes({ 0, 1, 3, 4 });
    BOOST_REQUIRE_EQUAL(locator.size(), 3u);
    BOOST_REQUIRE_EQUAL(locator[0], test::genesis.hash());
    BOOST_REQUIRE_EQUAL(locator[1], test::block1.hash());
    BOOST_REQUIRE_EQUAL(locator[2], test::block3.hash());
}

// get_confirmed_hashes

BOOST_AUTO_TEST_CASE(query_initialize__get_confirmed_hashes__initialized__one)
{
    settings settings{};
    settings.path = TEST_DIRECTORY;
    test::chunk_store store{ settings };
    test::query_accessor query{ store };
    BOOST_REQUIRE_EQUAL(store.create(events_handler), error::success);
    BOOST_REQUIRE(query.initialize(test::genesis));
    BOOST_REQUIRE_EQUAL(query.get_confirmed_hashes({ 0, 1, 2, 4, 6, 8 }).size(), 1u);
}

BOOST_AUTO_TEST_CASE(query_initialize__get_confirmed_hashes__gapped__expected)
{
    settings settings{};
    settings.path = TEST_DIRECTORY;
    test::chunk_store store{ settings };
    test::query_accessor query{ store };
    BOOST_REQUIRE_EQUAL(store.create(events_handler), error::success);
    BOOST_REQUIRE(query.initialize(test::genesis));
    BOOST_REQUIRE(query.set(test::block1, test::context, false, false));
    BOOST_REQUIRE(query.set(test::block2, test::context, false, false));
    BOOST_REQUIRE(query.set(test::block3, test::context, false, false));
    BOOST_REQUIRE(query.push_confirmed(query.to_header(test::block1.hash()), false));
    BOOST_REQUIRE(query.push_confirmed(query.to_header(test::block2.hash()), false));
    BOOST_REQUIRE(query.push_confirmed(query.to_header(test::block3.hash()), false));
    const auto locator = query.get_confirmed_hashes({ 0, 1, 3, 4 });
    BOOST_REQUIRE_EQUAL(locator.size(), 3u);
    BOOST_REQUIRE_EQUAL(locator[0], test::genesis.hash());
    BOOST_REQUIRE_EQUAL(locator[1], test::block1.hash());
    BOOST_REQUIRE_EQUAL(locator[2], test::block3.hash());
}

BOOST_AUTO_TEST_SUITE_END()
