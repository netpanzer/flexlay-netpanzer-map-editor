//  Flexlay - A Generic 2D Game Editor
//  Copyright (C) 2004 Ingo Ruhnke <grumbel@gmx.de>
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

#ifndef HEADER_FLEXLAY_BRUSHMASK_HPP
#define HEADER_FLEXLAY_BRUSHMASK_HPP

#include <ClanLib/Display/pixel_buffer.h>

enum BrushShape {
  BRUSH_SHAPE_CIRCLE,
  BRUSH_SHAPE_SQUARE,
  BRUSH_SHAPE_DIAMOND
};

CL_PixelBuffer generate_brushmask(BrushShape shape,
                                  float  radius,
                                  int    spikes,        /* 2 - 20     */
                                  float  hardness,      /* 0.0 - 1.0  */
                                  float  aspect_ratio,  /* y/x        */
                                  float  angle);        /* in degrees */

#endif

/* EOF */
