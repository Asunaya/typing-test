#pragma once
#include "typing.h"
#include <algorithm>
#include <string_view>

inline auto get_time()
{
    return typing_state::clock::now();
}

template <size_t size>
void copy(char (&dest)[size], const std::string_view& src)
{
    int i = 0;
    int end = static_cast<int>(std::min(size - 1, src.size()));
    for (; i < end; ++i)
    {
        dest[i] = src[i];
    }

    dest[end] = 0;
}
    
template <size_t size>
void append(char (&dest)[size], const std::string_view& src)
{
    int len = strlen(dest);
    int i = 0;
    int end = static_cast<int>(std::min(size - 1 - len, src.size()));
    for (; i < end; ++i)
    {
        dest[len + i] = src[i];
    }

    dest[len + end] = 0;
}

struct circular_range_iterator
{
    int i;
    typing_state* state;

    circular_range_iterator& operator++()
    {
        i = (i + 1) % state->num_words;
        return *this;
    }

    circular_range_iterator operator++(int)
    {
        auto temp = *this;
        ++*this;
        return temp;
    }

    int operator*() const
    {
        return i;
    }

    bool operator==(const circular_range_iterator& rhs) const
    {
        return i == rhs.i;
    }

    bool operator!=(const circular_range_iterator& rhs) const
    {
        return !(*this == rhs);
    }
};

template <typename T>
struct range
{
    T a, b;

    T begin() const { return a; }
    T end()   const { return b; }
};

range<circular_range_iterator> circular_range(typing_state& state, int a, int b)
{
    return {circular_range_iterator{a, &state}, circular_range_iterator{b, &state}};
}
