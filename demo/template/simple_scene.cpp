/*
 *  Copyright (C) 2015-2018 Simon Boyé
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


#include <lair/sys_sdl2/audio_module.h>

#include "simple_scene.h"


SimpleScene::SimpleScene(MainState* mainState)
    : Scene(mainState) {
}


void SimpleScene::load() {
	EntityRef test_map = entities().createEntity(root(), "test_map");

	TileMap::ObjectsLoader loadObjects = [this, &test_map](LdlParser& parser) {
		Variant var;
		if(ldlRead(parser, var)) {
			return entities().loadEntities(var, test_map);
		}
		return false;
	};

	AssetSP tileMapAsset = loader()->load<TileMapLoader>("test_map.ldl", loadObjects)->asset();
	_tileMap = tileMapAsset->aspect<TileMapAspect>();

//	loadEntities("entities.ldl", root());

	loader()->load<lair::SoundLoader>("sound.ogg");
	//loader()->load<MusicLoader>("music.ogg");

	loader()->waitAll();

	updateDepth(root());

	log().log("Done loading, dump entities and go !");

	Variant var;
	entities().saveEntities(var, root());

	std::ofstream out("entities.ldl");
	ErrorList errors;
	LdlWriter writer(&out, "<debug>", &errors);
	ldlWrite(writer, var);
	errors.log(log());
}


void SimpleScene::unload() {
}


void SimpleScene::start() {
	_modelRoot = entity("__model__");

	//audio()->playMusic(assets()->getAsset("music.ogg"));
	audio()->playSound(asset("sound.ogg"), 2);
}


void SimpleScene::stop() {

}


void SimpleScene::updateTick() {

}


void SimpleScene::updateFrame() {

}
