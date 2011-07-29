// test.cpp
//
// Copyright (c) 2008-2009
// Steven Watanabe
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <test.hpp>

struct outer {
    struct inner {
    };
};

template<int N>
struct inheritence_test
    :
    inheritence_test<N - 1>
    {};

template<>
struct inheritence_test<0> {};

template<int N>
struct member_test {
    typedef typename member_test<N - 1>::type type;
};

template<>
struct member_test<0> {
    typedef void type;
};

int main() {
    X<int> x1;
    X<char> x2;
    f(1);
    f('a');
    inheritence_test<5> i_test;
    member_test<5> m_test;
}
