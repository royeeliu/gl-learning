#include "sample_factory.h"
#include "hello_window.h"
#include "hello_triangle.h"

namespace samples {
namespace {

auto hello_window_factory_impl = SampleFactoryImpl<HelloWindow>{};
auto hello_triangle_facory_impl = SampleFactoryImpl<HelloTriangle>{};

} // namespace

namespace factories {

SampleFactory& hello_window_factory = hello_window_factory_impl;
SampleFactory& hello_triangle_factory = hello_triangle_facory_impl;

} // namespace factories

} // namespace samples
