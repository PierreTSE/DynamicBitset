// Copyright (C) 2005-2018 Free Software Foundation, Inc.
//
// This file is part of the GNU ISO C++ Library.  This library is free
// software; you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the
// Free Software Foundation; either version 3, or (at your option)
// any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this library; see the file COPYING3.  If not see
// <http://www.gnu.org/licenses/>.

// { dg-do compile }

#include <catch.hpp>
#include "../../DynamicBitset.hpp"

// libstdc++/23632
TEST_CASE("01")
{
  DynamicBitset<> v(100);
  const DynamicBitset<>::iterator fu = v.begin();
  if (!fu[0])
    fu[0] = true;

  const DynamicBitset<>::const_iterator cfu = v.begin();
  if (cfu[0])
    ;
}


