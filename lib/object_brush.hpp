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

#ifndef HEADER_FLEXLAY_OBJECT_BRUSH_HPP
#define HEADER_FLEXLAY_OBJECT_BRUSH_HPP

#include "objmap_sprite_object.hpp"

class MetaData;

class ObjectBrushImpl;

class ObjectBrush
{
public:
  ObjectBrush();
  ObjectBrush(const CL_Sprite& sprite_,
              const MetaData& data_);

  CL_Sprite get_sprite();
  MetaData get_data();

  ObjMapSpriteObject to_sprite_object(const CL_Pointf& pos);
private:
  boost::shared_ptr<ObjectBrushImpl> impl;
};

#endif

/* EOF */
