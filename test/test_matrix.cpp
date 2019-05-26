#include <catch2/catch.hpp>

#include <type_traits>
#include <tuple>

#include <matrix.h>

#include "utils.h"

using namespace std;
using namespace griha;
using namespace Catch::Matchers;

TEMPLATE_TEST_CASE("Matrix dimension", "[matrix][static]"
    , (tuple<const size_t, const size_t>)
    , (tuple<const size_t, const size_t, const size_t>)
    , (tuple<const size_t, const size_t, const size_t, const size_t, const size_t>)
    , (tuple<const size_t, const size_t, const size_t, const size_t, const size_t,
             const size_t, const size_t, const size_t, const size_t, const size_t>)) {

    using matrix_type = Matrix<int, 5, tuple_size_v<TestType>>;
    using value_type = decltype(tuple_cat(TestType{}, make_tuple<int>(0)));

    STATIC_REQUIRE(is_same_v<typename matrix_type::index_type, TestType>);
    STATIC_REQUIRE(is_same_v<typename matrix_type::value_type, value_type>);
}

TEST_CASE("Matrix access", "[matrix]") {
    constexpr int Def = 5;
    Matrix<int, Def> m;
    Matrix<int, Def, 3> m_dim3;
    Matrix<int, Def, 5> m_dim5;

    auto i = GENERATE(take(5, chunk(5, random(0, 999))));
    ostringstream os;
    copy(begin(i), prev(end(i)), ostream_iterator<int>(os, ":"));
    os << *prev(end(i));
    auto i_str = os.str();
    
    DYNAMIC_SECTION("read for indexes \"" << i_str << "\"") {
        REQUIRE(m[i[0]][i[1]] == Def);
        REQUIRE(m_dim3[i[0]][i[1]][i[2]] == Def);
        REQUIRE(m_dim5[i[0]][i[1]][i[2]][i[3]][i[4]] == Def);
    }

    DYNAMIC_SECTION("write for indexes \"" << i_str << "\"") {
        auto v = GENERATE(range(0, 100));
        m[i[0]][i[1]] = v;
        REQUIRE(m[i[0]][i[1]] == v);
        m_dim3[i[0]][i[1]][i[2]] = v;
        REQUIRE(m_dim3[i[0]][i[1]][i[2]] == v);
        m_dim5[i[0]][i[1]][i[2]][i[3]][i[4]] = v;
        REQUIRE(m_dim5[i[0]][i[1]][i[2]][i[3]][i[4]] == v);
    }

    DYNAMIC_SECTION("write for indexes \"" << i_str << "\" advanced") {
        auto v = GENERATE(range(0, 100));
        
        m[i[0]][i[1]] = m[i[1]][i[0]] = v;
        REQUIRE(m[i[0]][i[1]] == v);
        REQUIRE(m[i[1]][i[0]] == v);

        m_dim3[i[0]][i[1]][i[2]] = m_dim3[i[2]][i[1]][i[0]] = v;
        REQUIRE(m_dim3[i[0]][i[1]][i[2]] == v);
        REQUIRE(m_dim3[i[2]][i[1]][i[0]] == v);

        m_dim5[i[0]][i[1]][i[2]][i[3]][i[4]] = m_dim5[i[4]][i[3]][i[2]][i[1]][i[0]] = v;
        REQUIRE(m_dim5[i[0]][i[1]][i[2]][i[3]][i[4]] == v);
        REQUIRE(m_dim5[i[4]][i[3]][i[2]][i[1]][i[0]] == v);

        ((m[i[0]][i[1]] = m[i[1]][i[2]] = v - 100) = m[i[2]][i[3]] = v) = m[i[3]][i[4]] = v + 100;
        REQUIRE(m[i[0]][i[1]] == v + 100);
        REQUIRE(m[i[1]][i[2]] == v - 100);
        REQUIRE(m[i[2]][i[3]] == v);
        REQUIRE(m[i[3]][i[4]] == v + 100);
    }

    DYNAMIC_SECTION("write for indexes \"" << i_str << "\" lvalue") {
        auto v = GENERATE(range(0, 100));
        
        auto el = m[i[0]][i[1]];
        el = v;
        REQUIRE(m[i[0]][i[1]] == Def);
        REQUIRE(el == v);
        REQUIRE(m[i[0]][i[1]] == m[i[1]][i[0]]);
        std::move(el) = v;
        REQUIRE(m[i[0]][i[1]] == v);
        REQUIRE(el == v);

        auto el_dim3 = m_dim3[i[0]][i[1]][i[2]];
        el_dim3 = v;
        REQUIRE(m_dim3[i[0]][i[1]][i[2]] == Def);
        REQUIRE(el_dim3 == v);
        REQUIRE(m_dim3[i[0]][i[1]][i[2]] == m_dim3[i[2]][i[1]][i[0]]);
        std::move(el_dim3) = v;
        REQUIRE(m_dim3[i[0]][i[1]][i[2]] == v);
        REQUIRE(el_dim3 == v);
    }
}

TEST_CASE("Matrix size", "[matrix]") {
    constexpr int Def = 5;
    Matrix<int, Def> m;
    Matrix<int, Def, 3> m_dim3;
    Matrix<int, Def, 5> m_dim5;

    auto i = GENERATE(take(10, chunk(5, random(0, 999))));
    ostringstream os;
    copy(begin(i), prev(end(i)), ostream_iterator<int>(os, ":"));
    os << *prev(end(i));
    auto i_str = os.str();
    
    DYNAMIC_SECTION("- for indexes \"" << i_str << "\"") {
        auto v = GENERATE(range(Def + 1, 100 + Def));

        REQUIRE(m.size() == 0);
        REQUIRE(m_dim3.size() == 0);
        REQUIRE(m_dim5.size() == 0);

        m[i[0]][i[1]] = v;
        REQUIRE(m.size() == 1);
        m[i[1]][i[0]] = v;
        REQUIRE(m.size() == 2);
        m[i[0]][i[1]] = Def;
        REQUIRE(m.size() == 1);
        m[i[1]][i[0]] = Def;
        REQUIRE(m.size() == 0);
        
        m_dim3[i[0]][i[1]][i[2]] = v;
        REQUIRE(m_dim3.size() == 1);
        m_dim3[i[2]][i[0]][i[1]] = v;
        REQUIRE(m_dim3.size() == 2);
        m_dim3[i[1]][i[2]][i[0]] = v;
        REQUIRE(m_dim3.size() == 3);
        m_dim3[i[0]][i[1]][i[2]] = Def;
        REQUIRE(m_dim3.size() == 2);
        m_dim3[i[2]][i[0]][i[1]] = Def;
        REQUIRE(m_dim3.size() == 1);
        m_dim3[i[1]][i[2]][i[0]] = Def;
        REQUIRE(m_dim3.size() == 0);

        m_dim5[i[0]][i[1]][i[2]][i[3]][i[4]] = v;
        REQUIRE(m_dim5.size() == 1);
        m_dim5[i[1]][i[2]][i[3]][i[4]][i[0]] = v;
        REQUIRE(m_dim5.size() == 2);
        m_dim5[i[2]][i[3]][i[4]][i[0]][i[1]] = v;
        REQUIRE(m_dim5.size() == 3);
        m_dim5[i[3]][i[4]][i[0]][i[1]][i[2]] = v;
        REQUIRE(m_dim5.size() == 4);
        m_dim5[i[4]][i[0]][i[1]][i[2]][i[3]] = v;
        REQUIRE(m_dim5.size() == 5);
        m_dim5[i[0]][i[1]][i[2]][i[3]][i[4]] = Def;
        REQUIRE(m_dim5.size() == 4);
        m_dim5[i[1]][i[2]][i[3]][i[4]][i[0]] = Def;
        REQUIRE(m_dim5.size() == 3);
        m_dim5[i[2]][i[3]][i[4]][i[0]][i[1]] = Def;
        REQUIRE(m_dim5.size() == 2);
        m_dim5[i[3]][i[4]][i[0]][i[1]][i[2]] = Def;
        REQUIRE(m_dim5.size() == 1);
        m_dim5[i[4]][i[0]][i[1]][i[2]][i[3]] = Def;
        REQUIRE(m_dim5.size() == 0);
    }
}

TEST_CASE("Matrix iterators", "[matrix]") {
    constexpr int Def = 5;
    Matrix<int, Def> m;
    Matrix<int, Def, 3> m_dim3;

    m[0][0] = 3;
    m[10][50] = 10;
    m[3][60] = 4;

    m_dim3[0][1][2] = 13;
    m_dim3[0][2][2] = 30;
    m_dim3[0][1][6] = -1;
    m_dim3[0][7][2] = 88;

    SECTION("read/write access") {
        Matrix<int, Def>::iterator it;
        it = m.begin();
        REQUIRE(std::get<2>(*it) == 3);
        std::get<2>(*it) = 4;
        REQUIRE(std::get<2>(*it) == 4);
        REQUIRE(m[0][0] == 4);
        std::get<2>(*it) = 5; // it is important - if default value is set via iterator no clean will be done
        REQUIRE(std::get<2>(*it) == 5);
        REQUIRE(m[0][0] == 5);
    }

    SECTION("const iterator") {
        Matrix<int, Def>::const_iterator cit;
        cit = m.begin();
        REQUIRE(std::get<2>(*cit) == 3);
    }

    SECTION("for_each") {
        ostringstream os;
        for (auto el : m) {
            size_t x, y;
            int v;
            tie(x, y, v) = el;

            os << "{[" << x << "][" << y << "]=" << v << '}';
        }
        REQUIRE_THAT(os.str(), Equals("{[0][0]=3}{[3][60]=4}{[10][50]=10}"));
        
        os.str("");
        os.clear();
        for (auto el : m_dim3) {
            size_t x, y, z;
            int v;
            tie(x, y, z, v) = el;

            os << "{[" << x << "][" << y << "][" << z << "]=" << v << '}';
        }
        REQUIRE_THAT(os.str(), Equals("{[0][1][2]=13}{[0][1][6]=-1}{[0][2][2]=30}{[0][7][2]=88}"));
    }

}