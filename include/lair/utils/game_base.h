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


#ifndef _LAIR_UTILS_GAME_H
#define _LAIR_UTILS_GAME_H


#include <fstream>
#include <list>

#include <lair/core/lair.h>
#include <lair/core/log.h>
#include <lair/core/json.h>
#include <lair/core/path.h>
#include <lair/core/property.h>
#include <lair/core/asset_manager.h>
#include <lair/core/loader.h>

#include <lair/sys_sdl2/sys_module.h>
#include <lair/sys_sdl2/window.h>
#include <lair/sys_sdl2/audio_module.h>

#include <lair/render_gl2/render_module.h>
#include <lair/render_gl2/renderer.h>

//#include "sound_player.h"


namespace lair {


class GameState;


class GameConfigBase {
public:
	GameConfigBase();
	~GameConfigBase() = default;

	virtual void setFromArgs(int& argc, char** argv);
	virtual const PropertyList& properties() const;

	static const PropertyList& staticProperties();

public:
	bool     fullscreen;
	bool     vSync;
	Vector2i windowSize;
};


class GameBase {
public:
	GameBase(int argc, char** argv);
	GameBase(const GameBase&)  = delete;
	GameBase(      GameBase&&) = delete;
	~GameBase();

	GameBase& operator=(const GameBase&)  = delete;
	GameBase& operator=(      GameBase&&) = delete;

	Path dataPath() const;

	LdlPropertySerializer& serializer();

	SysModule*     sys();
	Window*        window();

	AssetManager*  assets();
	LoaderManager* loader();

	RenderModule*  renderModule();
	Renderer*      renderer();

	AudioModule*   audio();

	int argc() const;
	char** argv() const;

	void initialize(GameConfigBase& config);
	void shutdown();

	void setNextState(GameState* state);
	void run();
	void quit();

	Logger& log() { return _logger; }


protected:
	MasterLogger  _mlogger;
	std::ofstream _logStream;
//#ifndef _WIN32
	OStreamLogger _stdlogBackend;
//#endif
	OStreamLogger _fileBackend;
	Logger        _logger;

	int           _argc;
	char**        _argv;

	Path          _dataPath;

	LdlPropertySerializer _serializer;

	std::unique_ptr<SysModule>
	              _sys;
	Window*       _window;

	std::unique_ptr<RenderModule>
	              _renderModule;
	Renderer*     _renderer;

	std::unique_ptr<AudioModule>
	              _audio;

	std::unique_ptr<AssetManager>
	              _assets;
	std::unique_ptr<LoaderManager>
	              _loader;

	GameState*    _nextState;
	GameState*    _currentState;
};


}


#endif
