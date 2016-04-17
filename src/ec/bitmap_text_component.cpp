/*
 *  Copyright (C) 2015, 2016 Simon Boyé
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


#include <lair/core/json.h>
#include <lair/core/bitmap_font.h>

#include <lair/sys_sdl2/image_loader.h>

#include <lair/render_gl2/texture.h>

#include <lair/ec/sprite_renderer.h>

#include "lair/ec/bitmap_text_component.h"


namespace lair {


BitmapFontLoader::BitmapFontLoader(LoaderManager* manager, AspectSP aspect)
    : Loader(manager, aspect) {
}


BitmapFontLoader::~BitmapFontLoader() {
}


void BitmapFontLoader::loadSyncImpl(Logger& log) {
	Json::Value json;
	if(!parseJson(json, realPath(), asset()->logicPath(), log)) {
		return;
	}

	Json::Value imgPathValue = json.get("image", Json::nullValue);
	Path imgPath = imgPathValue.isString()? imgPathValue.asString():
	                                        asset()->logicPath().withExtension("png");
	imgPath = make_absolute(asset()->logicPath().dir(), imgPath);

	AssetSP imgAsset = _manager->loadSync<ImageLoader>(imgPath);

	BitmapFontAspectSP aspect = std::static_pointer_cast<BitmapFontAspect>(_aspect);
	aspect->_set(std::make_shared<BitmapFont>(json, imgAsset));

	_success();
}


//---------------------------------------------------------------------------//


BitmapTextComponent::BitmapTextComponent(_Entity* entity,
                             SparseComponentManager<BitmapTextComponent>* manager)
    : Component(entity),
      _manager(static_cast<BitmapTextComponentManager*>(manager)),
      _font(),
      _texture(),
      _text(),
      _color(1, 1, 1, 1),
      _size(0, 0),
      _anchor(0, 0) {
}


void BitmapTextComponent::destroy() {
	_manager->removeComponent(EntityRef(_entity()));
}


void BitmapTextComponent::clone(EntityRef& target) {
	_manager->cloneComponent(EntityRef(_entity()), target);
}


void BitmapTextComponent::setFont(BitmapFontAspectSP font) {
	_font = font;
}


void BitmapTextComponent::setFont(AssetSP font) {
	setFont(font? font->aspect<BitmapFontAspect>(): BitmapFontAspectSP());
}


void BitmapTextComponent::setFont(const Path& logicPath) {
	AssetSP asset = _manager->loader()->loadAsset<BitmapFontLoader>(logicPath);
	setFont(asset);
}


//---------------------------------------------------------------------------//


BitmapTextComponentManager::BitmapTextComponentManager(
        LoaderManager* loaderManager,
        SpriteRenderer* spriteRenderer)
	: SparseComponentManager("text"),
      _loader(loaderManager),
      _spriteRenderer(spriteRenderer) {
}


BitmapTextComponentManager::~BitmapTextComponentManager() {
}


void BitmapTextComponentManager::addComponentFromJson(
        EntityRef entity, const Json::Value& json, const Path& cd) {
	addComponent(entity);
	BitmapTextComponent* comp = get(entity);
	if(json.isMember("font")) {
		comp->setFont(make_absolute(cd, json["font"].asString()));
	}
	if(json.isMember("text")) {
		comp->setText(json["text"].asString());
	}
	if(json.isMember("color")) {
		Json::Value color = json["color"];
		if(color.isArray() || color.size() == 4) {
			comp->setColor(Vector4(color[0].asFloat(), color[1].asFloat(),
			               color[2].asFloat(), color[3].asFloat()));
		} /*else {
			log().warning("Invalid anchor field while loading entity \"", entity.name(), "\".");
		}*/
	}
	if(json.isMember("size")) {
		Json::Value size = json["size"];
		if(size.isArray() || size.size() == 2) {
			comp->setSize(Vector2i(size[0].asInt(), size[1].asInt()));
		} /*else {
			log().warning("Invalid anchor field while loading entity \"", entity.name(), "\".");
		}*/
	}
	if(json.isMember("anchor")) {
		Json::Value anchor = json["color"];
		if(anchor.isArray() || anchor.size() == 2) {
			comp->setAnchor(Vector2(anchor[0].asFloat(), anchor[1].asFloat()));
		} /*else {
			log().warning("Invalid anchor field while loading entity \"", entity.name(), "\".");
		}*/
	}
}


void BitmapTextComponentManager::cloneComponent(EntityRef base, EntityRef entity) {
	addComponent(entity);
	BitmapTextComponent* baseComp = get(base);
	BitmapTextComponent* comp = get(entity);
	comp->setFont( baseComp->font());
	comp->setText( baseComp->text());
	comp->setColor(baseComp->color());
}


void BitmapTextComponentManager::render(float interp) {
	_collectGarbages();
	for(auto& entityComp: *this) {
		BitmapTextComponent& comp = entityComp.second;

		if(!comp._alive
		|| !comp.font() || !comp.font()->get() || !comp.font()->get()->isValid()) {
			continue;
		}
		BitmapFontSP font = comp.font()->get();

		if(!comp.texture()) {
			comp._setTexture(_spriteRenderer->createTexture(font->image()));
		}

		Matrix4 wt = lerp(interp,
						  comp._entity()->prevWorldTransform.matrix(),
						  comp._entity()->worldTransform.matrix());

		int width = (comp.size()(0) > 0)? comp.size()(0): 999999;
		TextLayout layout = font->layoutText(comp.text(), width);
		for(unsigned i = 0; i < layout.nGlyphs(); ++i) {
			unsigned cp = layout.glyph(i).codepoint;
			Vector2 pos = layout.glyph(i).pos;
			const BitmapFont::Glyph& glyph = font->glyph(cp);

			Vector2 size = glyph.size;

			pos(0) += glyph.offset(0);
			pos(1) += font->height() - size(1) - glyph.offset(1)
			        + layout.box().sizes()(1);
			pos -= layout.box().sizes().cwiseProduct(comp.anchor());
			Box2 coords(pos, pos + size);

			TextureSP tex = comp.texture()->_get();
			_spriteRenderer->addSprite(wt, coords, comp.color(), glyph.region,
									   tex,
									   Texture::NEAREST | Texture::CLAMP,
									   BLEND_ALPHA);
		}
	}
}


LoaderManager* BitmapTextComponentManager::loader() {
	return _loader;
}


SpriteRenderer* BitmapTextComponentManager::spriteRenderer() const {
	return _spriteRenderer;
}


}
