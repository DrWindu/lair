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


#ifndef _LAIR_EC_SPRITE_COMPONENT_H
#define _LAIR_EC_SPRITE_COMPONENT_H


#include <cstring>
#include <vector>
#include <list>

#include <lair/core/lair.h>

#include <lair/meta/property.h>
#include <lair/meta/property_list.h>
#include <lair/meta/with_properties.h>

#include <lair/sys_sdl2/image_loader.h>

#include <lair/render_gl3/context.h>
#include <lair/render_gl3/texture.h>
#include <lair/render_gl3/texture_set.h>
#include <lair/render_gl3/render_pass.h>
#include <lair/render_gl3/renderer.h>

#include <lair/ec/entity.h>
#include <lair/ec/component.h>
#include <lair/ec/component_manager.h>
#include <lair/ec/dense_component_manager.h>
#include <lair/ec/sprite_renderer.h>


namespace lair
{


class OrthographicCamera;
class Texture;

class _Entity;
class EntityManager;

class SpriteComponentManager;


class SpriteComponent : public Component, WithProperties<SpriteComponent> {
public:
	typedef SpriteComponentManager Manager;

public:
	SpriteComponent(Manager* manager, _Entity* entity);
	SpriteComponent(const SpriteComponent&) = delete;
	SpriteComponent(SpriteComponent&&)      = default;
	~SpriteComponent();

	SpriteComponent& operator=(const SpriteComponent&) = delete;
	SpriteComponent& operator=(SpriteComponent&&)      = default;

	Manager* manager();

	inline TextureSetCSP textureSet() const { return _textureSet; }
	inline void setTextureSet(TextureSetCSP textureSet) { _textureSet = textureSet; }
	void setTextureSet(const TextureSet& textureSet);

	TextureAspectSP texture() const;
	inline const Path& texturePath() const {
		TextureAspectSP tex = texture();
		return tex? tex->asset()->logicPath(): emptyPath;
	}

	void setTexture(TextureAspectSP texture);
	void setTexture(AssetSP texture);
	void setTexture(const Path& logicPath);

	inline const Vector2& anchor() const { return _anchor; }
	inline void setAnchor(const Vector2& anchor) { _anchor = anchor; }

	inline const Vector4& color() const { return _color; }
	inline void setColor(const Vector4& color) { _color = color; }

	inline const Vector2i& tileGridSize() const { return _tileGridSize; }
	inline void setTileGridSize(const Vector2i& size) { _tileGridSize = size; }

	inline unsigned tileIndex() const { return _tileIndex; }
	inline void setTileIndex(unsigned index) { _tileIndex = index; }

	inline const Box2& view() const { return _view; }
	inline void setView(const Box2& view) { _view = view; }

	inline BlendingMode blendingMode() const { return _blendingMode; }
	inline void setBlendingMode(BlendingMode bm) { _blendingMode = bm; }

	static const PropertyList& properties();

	Box2 _texCoords() const;

	static bool _renderCompare(SpriteComponent* c0, SpriteComponent* c1);

protected:
	TextureSetCSP   _textureSet;
	Vector2         _anchor;
	Vector4         _color;
	Vector2i        _tileGridSize;
	unsigned        _tileIndex;
	Box2            _view;
	BlendingMode    _blendingMode;
};


class SpriteComponentManager : public DenseComponentManager<SpriteComponent> {
public:
	SpriteComponentManager(AssetManager* assetManager,
	                       LoaderManager* loaderManager,
	                       RenderPass* renderPass,
	                       SpriteRenderer* spriteRenderer,
	                       size_t componentBlockSize = 1024);

	SpriteComponentManager(const SpriteComponentManager&) = delete;
	SpriteComponentManager(SpriteComponentManager&&)      = delete;

	virtual ~SpriteComponentManager();

	SpriteComponentManager& operator=(const SpriteComponentManager&) = delete;
	SpriteComponentManager& operator=(SpriteComponentManager&&)      = delete;

//	void render(float interp, const OrthographicCamera& camera);
	void render(EntityRef entity, float interp, const OrthographicCamera& camera);

	AssetManager* assets();
	LoaderManager* loader();
	SpriteRenderer* spriteRenderer();

protected:
	void _render(EntityRef entity, float interp, const OrthographicCamera& camera);

protected:
	AssetManager*    _assets;
	LoaderManager*   _loader;
	SpriteRenderer*  _spriteRenderer;
	RenderPass*      _renderPass;

	RenderPass::DrawStates _states;
};


}


#endif
