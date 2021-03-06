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
#include <functional>

#include <SDL.h>

#include <lair/core/lair.h>
#include <lair/core/log.h>
#include <lair/core/path.h>

#include <lair/fs/abstract_file_system.h>


extern "C" {
typedef union  SDL_Event SDL_Event;
}

namespace lair
{


class Window;

/// \defgroup sys_sdl2 sys_sdl2


SDL_RWops* sdlRwFromFile(const VirtualFile& file);


/**
 * \brief The sys_sdl2 module object.
 *
 * \ingroup sys_sdl2
 */
class SysModule {
public:
	typedef void* (*GlGetProcAddressFunction)(const char*);

	typedef std::function<void(const SDL_MouseMotionEvent&)> MouseMoveCallback;
	typedef std::function<void(const SDL_MouseButtonEvent&)> MouseButtonCallback;
	typedef std::function<void(const SDL_MouseWheelEvent&)>  MouseWheelCallback;

	typedef std::function<void()> QuitCallback;

	typedef std::function<void(unsigned scancode,
	                           unsigned keycode,
	                           uint16 mod,
	                           bool pressed,
	                           bool repeat)> KeyCallback;
	typedef std::function<void(const String& text)> TextInputCallback;
	typedef std::function<void(const String& text, int start, int length)> TextEditCallback;

public:
	/// \{
	/// \name Constructor, destructor, initialization and shutdown.

	SysModule(MasterLogger* logger = nullptr, LogLevel level = LogLevel::Log);
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
	/// \name OpenGL.

	GlGetProcAddressFunction glGetProcAddressFunction() const;

	/// \}

	/// \{
	/// \name Event management.

	void waitAndDispatchSystemEvents();
	void dispatchPendingSystemEvents();

	/// \}

	/// \{
	/// \name Keyboard.

	uint8 getKeyState(unsigned scancode);

	bool isTextInputActive() const;
	void startTextInput();
	void stopTextInput();

	/// \}

	/// \{
	/// \name File loading

	const Path& basePath();
	const Path getPrefPath(const char* org, const char* app);

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
	MouseMoveCallback   onMouseMove;
	MouseButtonCallback onMousePress;
	MouseButtonCallback onMouseRelease;
	MouseWheelCallback  onMouseWheel;

	QuitCallback onQuit;

	KeyCallback       onKeyDown;
	KeyCallback       onKeyUp;
	TextInputCallback onTextInput;
	TextEditCallback  onTextEdit;


private:
	typedef std::unique_ptr<Window> WindowPtr;
	typedef std::pair<unsigned, WindowPtr> WindowPair;
	typedef std::unordered_map<unsigned, WindowPtr> WindowMap;


private:
	Window* _windowFromID(unsigned windowID);

private:
	Logger              _log;

	bool                _initialized;
	WindowMap           _windowMap;

	Path                _basePath;
};


}


#endif
