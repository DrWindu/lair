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


#include <SDL_surface.h>
#include <SDL_image.h>

#include <lair/core/lair.h>
#include <lair/core/log.h>

#include "lair/sys_sdl2/image_loader.h"


namespace lair
{


ImageAspect::ImageAspect(AssetSP asset, Image&& image)
	: Aspect(asset),
	  _image(std::move(image)){
}

ImageLoader::ImageLoader(LoaderManager* manager, AssetSP asset)
    : Loader(manager, asset) {
}


ImageLoader::~ImageLoader() {
}


void ImageLoader::loadSyncImpl(Logger& log) {
	auto surf = make_unique(IMG_Load(realPath().utf8CStr()), SDL_FreeSurface);
	if(surf) {
		Image::Format format;
		switch(surf->format->format) {
		case SDL_PIXELFORMAT_RGB24:
			format = Image::Format::FormatRGB8;
			break;
		case SDL_PIXELFORMAT_ABGR8888:
			format = Image::Format::FormatRGBA8;
			break;
		default:
			log.error("Unable to load image \"", _asset->logicPath(), "\" (", realPath(),"): unsupported format ",
			          SDL_GetPixelFormatName(surf->format->format));
			format = Image::Format::FormatInvalid;
		}

		if(format != Image::Format::FormatInvalid) {
			Image image(surf->w, surf->h, format, surf->pixels);
			AspectSP aspect = std::make_shared<ImageAspect>(_asset, std::move(image));
			_asset->manager()->setAspect(_asset, aspect);
		}
	} else {
		log.error("Unable to load image \"", _asset->logicPath(), "\" (", realPath(),"): ", IMG_GetError());
	}

	_success();
}


}
