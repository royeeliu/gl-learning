#pragma once

#include <cassert>

#ifndef _ASSERT
#define _ASSERT assert;
#endif

#define ASSERT(exp) _ASSERT(exp)
#define ENSURE(exp) _ASSERT(exp)
#define REQUIRE(exp) _ASSERT(exp)

#define NOTREATCHED() _ASSERT(false)
#define NOTIMPLEMENTED() _ASSERT(false)
