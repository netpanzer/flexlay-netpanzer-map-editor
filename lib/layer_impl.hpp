//  Flexlay - A Generic 2D Game Editor
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

#ifndef HEADER_FLEXLAY_LAYER_IMPL_HPP
#define HEADER_FLEXLAY_LAYER_IMPL_HPP

#include <ClanLib/Core/Math/rect.h>
#include <ClanLib/Display/graphic_context.h>
#include "meta_data.hpp"

class GraphicContextState;

class LayerImpl
{
public:
  /** MetaData attached to the layer, MetaData can be any data
      supplied by the user, but most commonly it is used to associate
      the given layer with a scripting language object (PyObj, SCM,
      etc.), so that the user can attach additional data to a layer
      from the scripting side. */
  MetaData  data;
  
  /** The position of the layer */
  CL_Pointf pos;

  LayerImpl() 
    : pos(0, 0)
  {}
  virtual ~LayerImpl() {}

  virtual void draw(const GraphicContextState& state, CL_GraphicContext* gc) =0;
  virtual bool has_bounding_rect() const =0;

  // FIXME: Should use CL_Rectf
  virtual CL_Rect get_bounding_rect() { return CL_Rect(); }
};

#endif

/* EOF */
