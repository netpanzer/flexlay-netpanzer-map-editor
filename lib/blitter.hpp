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

#ifndef HEADER_FLEXLAY_BLITTER_HPP
#define HEADER_FLEXLAY_BLITTER_HPP

#include <ClanLib/Display/pixel_buffer.h>

void blit(CL_PixelBuffer target, CL_PixelBuffer brush, int x_pos, int y_pos);
void blit_opaque(CL_PixelBuffer target, CL_PixelBuffer brush, int x_pos, int y_pos);
void clear(CL_PixelBuffer target);

#endif

/* EOF */
