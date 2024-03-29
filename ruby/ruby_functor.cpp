//  $Id$
//
//  Pingus - A free Lemmings clone
//  Copyright (C) 2002 Ingo Ruhnke <grumbel@gmx.de>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <iostream>
#include "flexlay_wrap.hpp"
#include "ruby_functor.hpp"

void
RubyFunctor::print_error()
{
  // FIXME: Potential memory leak
  std::cerr << "######################################################" << std::endl;
  std::cerr << "RubyException: ";
  // Code below needs updating for ruby 3. -andy5995/2024-01-31
            //<< rb_str2cstr(rb_inspect(ruby_errinfo), 0)
            //<< std::endl;

  //VALUE trace = rb_funcall(ruby_errinfo, rb_intern("backtrace"), 0);
  //for (int i = 0; i < RARRAY(trace)->len; ++i)
    //std::cout << rb_str2cstr(rb_ary_entry(trace, i), 0) << std::endl;
  //std::cout << "######################################################" << std::endl;
  //ruby_errinfo = Qnil;
}

VALUE
RubyFunctor::funcall_protect(VALUE self)
{
  return rb_funcall(reinterpret_cast<RubyFunctor*>(self)->val.ptr(), rb_intern("call"), 0);
}

VALUE
RubyFunctor::funcall_protect1(VALUE self)
{
  VALUE* args = reinterpret_cast<VALUE*>(self);
  return rb_funcall(reinterpret_cast<RubyFunctor*>(args[0])->val.ptr(), rb_intern("call"), 1,
                    args[1]);
}

VALUE
RubyFunctor::funcall_protect2(VALUE self)
{
  VALUE* args = reinterpret_cast<VALUE*>(self);
  return rb_funcall(reinterpret_cast<RubyFunctor*>(args[0])->val.ptr(), rb_intern("call"), 2,
                    args[1], args[2]);
}

RubyFunctor::RubyFunctor(const RubyObject& val_)
  : val(val_)
{
}

RubyFunctor::~RubyFunctor()
{
}

void
RubyFunctor::operator()()
{
  int state = 0;
  rb_protect(&RubyFunctor::funcall_protect, reinterpret_cast<VALUE>(this), &state);
  if (state)
    print_error();
}

// FIXME: Protect the function calls with arguments too

void
RubyFunctor::operator()(int i)
{
  //rb_funcall(val.ptr(), rb_intern("call"), 1, INT2FIX(i));
  int state = 0;
  VALUE args[2];
  args[0] = reinterpret_cast<VALUE>(this);
  args[1] = INT2FIX(i);
  rb_protect(&RubyFunctor::funcall_protect1, reinterpret_cast<VALUE>(args), &state);

  if (state)
    print_error();
}

void
RubyFunctor::operator()(int x, int y)
{
  //  rb_funcall(val.ptr(), rb_intern("call"), 2, INT2FIX(x), INT2FIX(y));
  int state = 0;
  VALUE args[3];
  args[0] = reinterpret_cast<VALUE>(this);
  args[1] = INT2FIX(x);
  args[2] = INT2FIX(y);
  rb_protect(&RubyFunctor::funcall_protect2, reinterpret_cast<VALUE>(args), &state);

  if (state)
    print_error();
}

/* EOF */
