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


#ifndef _LAIR_EC_TILE_LAYER_COMPONENT_H
#define _LAIR_EC_TILE_LAYER_COMPONENT_H


#include <lair/core/lair.h>

#include <lair/meta/property.h>
#include <lair/meta/property_list.h>
#include <lair/meta/with_properties.h>

#include <lair/render_gl3/render_pass.h>
#include <lair/render_gl3/texture.h>
#include <lair/render_gl3/texture_set.h>

#include <lair/ec/component.h>
#include <lair/ec/dense_component_manager.h>

#include <lair/utils/tile_map.h>


namespace lair
{


class OrthographicCamera;

class SpriteRenderer;

class TileLayerComponentManager;



class TileLayerComponent : public Component, WithProperties<TileLayerComponent> {
public:
	typedef TileLayerComponentManager Manager;

public:
	TileLayerComponent(Manager* manager, _Entity* entity);
	TileLayerComponent(const TileLayerComponent&) = delete;
	TileLayerComponent(TileLayerComponent&&)      = default;
	~TileLayerComponent() = default;

	TileLayerComponent& operator=(const TileLayerComponent&) = delete;
	TileLayerComponent& operator=(TileLayerComponent&&)      = default;

	Manager* manager();

	inline TileMapAspectSP tileMap() const { return _tileMap; }
	inline const Path& tileMapPath() const {
		return tileMap()? tileMap()->asset()->logicPath(): emptyPath;
	}
	inline void setTileMap(TileMapAspectSP tileMap) { _tileMap = tileMap; _bufferDirty = true; }
	void setTileMap(AssetSP tileMap);
	void setTileMap(const Path& logicPath);

	inline TextureSetCSP textureSet() const { return _textureSet; }
	inline void setTextureSet(const TextureSetCSP textureSet) { _textureSet = textureSet; }
	void setTextureSet(const TextureSet& textureSet);

	TextureAspectSP tileSet() const;
	void _setTileSet(TextureAspectSP tileSet);

	inline unsigned layerIndex() const { return _layerIndex; }
	inline void setLayerIndex(unsigned index) { _layerIndex = index; _bufferDirty = true; }

	inline BlendingMode blendingMode() const { return _blendingMode; }
	inline void setBlendingMode(BlendingMode bm) { _blendingMode = bm; }

	static const PropertyList& properties();


protected:
	TileMapAspectSP _tileMap;
	TextureSetCSP   _textureSet;
	unsigned        _layerIndex;
	BlendingMode    _blendingMode;

public:
	bool         _bufferDirty;
	unsigned     _vertexCount;
	std::shared_ptr<VertexArray>  _vertexArray;
	std::unique_ptr<BufferObject> _vBuffer;
	std::unique_ptr<BufferObject> _iBuffer;
};


class TileLayerComponentManager : public DenseComponentManager<TileLayerComponent> {
public:
	TileLayerComponentManager(LoaderManager* loaderManager,
	                          RenderPass* renderPass,
	                          SpriteRenderer* spriteRenderer,
	                          size_t componentBlockSize = 16);

	TileLayerComponentManager(const TileLayerComponentManager&) = delete;
	TileLayerComponentManager(TileLayerComponentManager&&)      = delete;

	virtual ~TileLayerComponentManager() = default;

	TileLayerComponentManager& operator=(const TileLayerComponentManager&) = delete;
	TileLayerComponentManager& operator=(TileLayerComponentManager&&)      = delete;

	void createTextures();

	void render(EntityRef entity, float interp, const OrthographicCamera& camera);

	SpriteRenderer* spriteRenderer();
	LoaderManager* loader();

protected:
	unsigned _fillBuffer(BufferObject& vBuffer, BufferObject& iBuffer,
	                     const TileMap& tileMap, unsigned layerIndex,
	                     const Matrix4& wt) const;
	void _render(EntityRef entity, float interp, const OrthographicCamera& camera);

protected:
	SpriteRenderer*  _spriteRenderer;
	LoaderManager*   _loader;
	RenderPass*      _renderPass;

	RenderPass::DrawStates _states;
};


}


#endif
