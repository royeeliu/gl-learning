#pragma once

#include "sample_factory.h"

namespace samples {

struct SampleItem
{
    wchar_t const* name;
    SampleFactory& factory;
};

struct SampleGroupItem
{
    wchar_t const* const name;
    SampleItem const* const items;
    size_t const count;
};

// clang-format off
static SampleItem const HelloSamples[] = {
    {L"Hello Window", factories::hello_window_factory}, 
    {L"Hello Triangle", factories::hello_triangle_factory}
};

static SampleGroupItem SampleGroups[] = {
    {L"Hello", HelloSamples, _countof(HelloSamples)}
};
// clang-format on

} // namespace samples
