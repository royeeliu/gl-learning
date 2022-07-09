#pragma once

#include "sample_base.h"

#include <memory>

namespace samples {

class SampleFactory
{
public:
    virtual std::unique_ptr<SampleBase> Create(HWND hwnd, ErrorCallback on_error) = 0;

protected:
    ~SampleFactory() {}
};

template<class T, class = std::enable_if<std::is_base_of<SampleBase, T>::value, void>::type>
class SampleFactoryImpl : public SampleFactory
{
    using SampleType = T;

public:
    std::unique_ptr<SampleBase> Create(HWND hwnd, ErrorCallback on_error) override {
        return std::make_unique<SampleType>(hwnd, std::move(on_error));
    }
};

namespace factories {

extern SampleFactory& hello_window_factory;
extern SampleFactory& hello_triangle_factory;

} // namespace factories

} // namespace samples
