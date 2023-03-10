/*
-------------------------------------------------------------------------------
    Copyright (c) Charles Carley.

  This software is provided 'as-is', without any express or implied
  warranty. In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
-------------------------------------------------------------------------------
*/
#pragma once
#include <chrono>
#include "Utils/Definitions.h"

namespace Rt2
{
    using Clock = std::chrono::high_resolution_clock;

    class Time
    {
    public:
        static U16 now16()
        {
            return U16(Clock::now().time_since_epoch().count() % 0xFFFF);
        }

        static U32 now32()
        {
            return U32(Clock::now().time_since_epoch().count() % 0xFFFFFFFF);
        }
    };

}  // namespace Rt2
