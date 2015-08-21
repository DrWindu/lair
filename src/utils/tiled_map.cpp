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


#include <lair/core/lair.h>
#include <lair/core/log.h>

#include "lair/utils/tiled_map.h"


namespace lair
{


TiledMap::TiledMap() {
}


TiledMap::~TiledMap() {
}


unsigned TiledMap::nLayers() const {
	return _layers.size();
}


unsigned TiledMap::width(unsigned /*layer*/) const {
	return _width;
}


unsigned TiledMap::height(unsigned /*layer*/) const {
	return _height;
}


TiledMap::TileIndex TiledMap::tile(unsigned x, unsigned y, unsigned layer) const {
	unsigned i = y * width(layer) * x;
	lairAssert(i < _layers[layer].size());
	return _layers[layer][i];
}


void TiledMap::setTile(unsigned x, unsigned y, unsigned layer, TileIndex tile) {
	unsigned i = y * width(layer) * x;
	lairAssert(i < _layers[layer].size());
	_layers[layer][i] = tile;
}


unsigned TiledMap::nObjectLayer() const {
	return _objectLayers.size();
}


const Json::Value& TiledMap::objectLayer(unsigned layer) const {
	lairAssert(layer < _objectLayers.size());
	return _objectLayers[layer];
}


const std::string& TiledMap::tileSetFile() const {
	return _tileSetFile;
}


unsigned TiledMap::tileSetHTiles() const {
	return _tileSetHTiles;
}


unsigned TiledMap::tileSetVTiles() const {
	return _tileSetVTiles;
}


bool TiledMap::setFromJson(Logger& log, const std::string& name, const Json::Value& value) {
	_width  = value.get("width",  0).asInt();
	_height = value.get("height", 0).asInt();

	if(_width == 0 || _height == 0) {
		log.error("Tiled map \"", name, "\" has invalid size.");
		return false;
	}

	const Json::Value& tilesets = value["tilesets"];
	if(!tilesets.isArray() || tilesets.size() != 1) {
		log.error("Tiled map \"", name, "\" has invalid number of tilesets.");
		return false;
	}
	_tileSetFile = tilesets[0].get("image", "").asString();
	_tileSetHTiles = tilesets[0].get("imagewidth", 0).asInt()
	               / tilesets[0].get("tilewidth", 1).asInt();
	_tileSetVTiles = tilesets[0].get("imageheight", 0).asInt()
	               / tilesets[0].get("tileheight", 1).asInt();

	for(const Json::Value& layer: value["layers"]) {
		if(layer.get("type", "").asString() == "tilelayer") {
			unsigned width  = layer.get("width",  0).asInt();
			unsigned height = layer.get("height", 0).asInt();

			if(width != _width || height != _height) {
				log.error("Tiled map \"", name, "\" has a layer with invalid size");
				return false;
			}

			_layers.emplace_back();
			TileMap& map = _layers.back();
			map.reserve(_width * _height);
			for(const Json::Value& v: layer["data"]) {
				if(!v.isIntegral()) {
					log.error("Tiled map \"", name, "\": invalid tile index");
					return false;
				}
				map.push_back(v.asInt());
			}
		} else if(layer.get("type", "").asString() == "objectgroup") {
			_objectLayers.push_back(layer);
		}
	}

	return true;
}


}
