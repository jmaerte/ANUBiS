//
// Created by maertej on 29.09.19.
//

#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>
#include <anubis/simplicial/simplicial.hpp>

TEST_CASE( "homology of octahedron", "[main]" ) {
    simplicial_complex ({{0,1,2}, {0,2,3}, {0,3,4}, {0,1,4}, {1,2,5}, {2,3,5}, {3,4,5}, {1,4,}});
    REQUIRE(si.homology(true) == {0, 0, 1}); // compare as std::array s
}
