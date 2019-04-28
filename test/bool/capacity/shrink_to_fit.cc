// { dg-do run { target c++11 } }

// Copyright (C) 2011-2018 Free Software Foundation, Inc.
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

#include "../../../DynamicBitset.hpp"
#include <catch.hpp>

TEST_CASE("01")
{
  DynamicBitset vb(__CHAR_BIT__ * sizeof(unsigned long) + 1);
  vb.pop_back();

  auto old_capacity = vb.capacity();
  vb.shrink_to_fit();
  REQUIRE( vb.capacity() < old_capacity );
  REQUIRE( vb.size() == vb.capacity() );
}


