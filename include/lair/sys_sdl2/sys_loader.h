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


#ifndef _LAIR_SYS_SDL2_SYS_LOADER_H
#define _LAIR_SYS_SDL2_SYS_LOADER_H


#include <lair/core/lair.h>

#include <lair/utils/loader.h>

#include <lair/sys_sdl2/image_loader.h>


namespace lair
{


class SysLoader : public LoaderManager {
public:
	typedef std::shared_ptr<ImageLoader> ImageLoaderPtr;

public:
	SysLoader(size_t maxCacheSize, unsigned nThread = 1,
	          Logger& log = noopLogger);
	SysLoader(const SysLoader&) = delete;
	SysLoader(SysLoader&&)      = delete;
	~SysLoader();

	SysLoader& operator=(const SysLoader&) = delete;
	SysLoader& operator=(SysLoader&&)      = delete;

	ImageLoaderPtr loadImage(const std::string file);

protected:
};


}


#endif
