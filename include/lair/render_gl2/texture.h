/*
 *  Copyright (C) 2015 Simon Boyé
 *
 *  This file is part of lair.
 *
 *  lair is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  lair is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with lair.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#ifndef _LAIR_RENDER_GL2_TEXTURE_H
#define _LAIR_RENDER_GL2_TEXTURE_H


#include <lair/core/lair.h>

#include <lair/render_gl2/gl.h>

namespace lair
{

class Image;

class Texture {
public:
	enum {
		MAG_NEAREST    = 0 << 1,
		MAG_LINEAR     = 1 << 1,

		MIN_NEAREST    = 0 << 2,
		MIN_LINEAR     = 1 << 2,

		MIPMAP_NONE    = 0 << 3,
		MIPMAP_NEAREST = 1 << 3,
		MIPMAP_LINEAR  = 2 << 3,

		NEAREST        = MAG_NEAREST | MIN_NEAREST | MIPMAP_NONE,
		BILINEAR       = MAG_LINEAR  | MIN_LINEAR  | MIPMAP_NEAREST,
		TRILINEAR      = MAG_LINEAR  | MIN_LINEAR  | MIPMAP_LINEAR,

		REPEAT_S       = 0 << 5,
		CLAMP_S        = 1 << 5,
		MIRROR_S       = 2 << 5,

		REPEAT_T       = 0 << 7,
		CLAMP_T        = 1 << 7,
		MIRROR_T       = 2 << 7,

		REPEAT         = REPEAT_S | REPEAT_T,
		CLAMP          =  CLAMP_S |  CLAMP_T,
		MIRROR         = MIRROR_S | MIRROR_T,

		MAG_MASK       = (1 << 1),
		MIN_MASK       = (1 << 2),
		MIPMAP_MASK    = (1 << 3) | (1 << 4),
		WRAP_S_MASK    = (1 << 5) | (1 << 6),
		WRAP_T_MASK    = (1 << 7) | (1 << 8),
		WRAP_MASK      = WRAP_S_MASK | WRAP_T_MASK
	};

public:
	Texture();
	Texture(const Texture&) = delete;
	Texture(Texture&&)      = delete;
	~Texture();

	Texture& operator=(const Texture&) = delete;
	Texture& operator=(Texture&&)      = delete;

	inline bool   isValid()    const { return _id; }

	inline uint32 magFilter()  const { return _flags &    MAG_MASK; }
	inline uint32 minFilter()  const { return _flags &    MIN_MASK; }
	inline uint32 mipmapMode() const { return _flags & MIPMAP_MASK; }
	inline uint32 wrapS()      const { return _flags & WRAP_S_MASK; }
	inline uint32 wrapT()      const { return _flags & WRAP_T_MASK; }
	inline uint32 wrap()       const { return _flags &   WRAP_MASK; }

	inline uint16 width()      const { return _width; }
	inline uint16 height()     const { return _height; }

	bool upload(const Image& image, uint32 flags = BILINEAR | REPEAT);
	void setFlags(uint32 flags);
	void release();

	inline GLuint _glId() { return _id; }

protected:
	GLuint _id;
	uint32 _flags;
	uint16 _width;
	uint16 _height;
};


}


#endif
