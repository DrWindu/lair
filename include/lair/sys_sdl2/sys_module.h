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


#ifndef _LAIR_SYS_SDL2_SYS_MODULE_H
#define _LAIR_SYS_SDL2_SYS_MODULE_H


#include <unordered_map>

#include <lair/core/lair.h>
#include <lair/core/log.h>


extern "C" {
typedef struct Window    Window;
typedef union  SDL_Event SDL_Event;
}

namespace lair
{


class Window;

/// \defgroup sys_sdl2 sys_sdl2

/**
 * \brief The sys_sdl2 module object.
 *
 * \ingroup sys_sdl2
 */
class SysModule {
public:
	typedef bool (*QuitCallback)();

public:
	/// \{
	/// \name Constructor, destructor, initialization and shutdown.

	SysModule(MasterLogger* logger = 0);
	SysModule(const SysModule&) = delete;
	SysModule(SysModule&&) = delete;
	~SysModule();

	SysModule& operator=(const SysModule&) = delete;
	SysModule& operator=(SysModule&&) = delete;

	bool initialize();
	void shutdown();

	/// \}

	/// \{
	/// \name System-related stuff.

	bool isScreensaverEnabled() const;
	void setScreensaverEnabled(bool enableScreensaver);

	bool isVSyncEnabled() const;
	void setVSyncEnabled(bool enableVSync);

	// TODO: Add screens/displayMode management stuff.

	/// \}

	/// \{
	/// \name Windows management.

	Window* createWindow(const char* utf8Title, int width, int height);
	void destroyAllWindows();

	/// \}

	/// \{
	/// \name Event management.

	void waitAndDispatchSystemEvents();
	void dispatchPendingSystemEvents();

	/// \}

	/// \{
	/// \name Keyboard.

	uint8 getKeyState(unsigned scancode);

	/// \}

	/// \{
	/// \name Utility

	inline Logger& log() {
		return _log;
	}

	/// \}

	/// \{
	/// \name Time

	uint64 getTimeNs() const;
	void   waitNs(uint64 ns) const;

	/// \}

	/// \private
	void _removeWindow(unsigned windowID);

	/// \private
	const char* _sdlEventName(unsigned type) const;

	/// \private
	void _dispatchSystemEvent(const SDL_Event& event);


public:
	QuitCallback onQuit;


private:
	typedef std::pair<unsigned, Window*> WindowPair;
	typedef std::unordered_map<unsigned, Window*> WindowMap;


private:
	Window* _windowFromID(unsigned windowID);

private:
	bool _initialized;
	WindowMap _windowMap;

	Logger _log;
};


}


#endif