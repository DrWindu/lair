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


#ifndef _LAIR_UTILS_LOADER_H
#define _LAIR_UTILS_LOADER_H


#include <memory>

#include <deque>
#include <unordered_map>

#include <string>

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include <lair/core/lair.h>
#include <lair/core/log.h>

#include <lair/utils/path.h>


namespace lair
{


enum {
	MAX_LOADER_THREADS = 8
};


class LoaderManager;


class Loader {
public:
	Loader(LoaderManager* manager, const std::string& path);
	Loader(const Loader&) = delete;
	Loader(Loader&&)      = delete;
	virtual ~Loader();

	Loader& operator=(const Loader&) = delete;
	Loader& operator=(Loader&&)      = delete;

	bool               isLoaded();
	size_t             size();
	const std::string& path() const { return _path; }

	void wait();

	virtual void loadSync(Logger& log);

	void _done(Logger& log, size_t size);

protected:
	LoaderManager*          _manager;
	bool                    _isLoaded;
	size_t                  _size;
	std::string             _path;
	std::mutex              _mutex;
	std::condition_variable _cv;
};


class _LoaderThread {
public:
	_LoaderThread();
	~_LoaderThread();

	void setManager(LoaderManager* manager);

	void start();
	void stop();
	void wait();

private:
	void _run();

private:
	Logger           _logger;
	LoaderManager*   _manager;
	std::thread      _thread;
	std::atomic_bool _running;
};


class LoaderManager {
public:
	typedef std::shared_ptr<Loader> LoaderPtr;

public:
	LoaderManager(size_t maxCacheSize, unsigned nThread = 1,
	              Logger& log = noopLogger);
	LoaderManager(const LoaderManager&) = delete;
	LoaderManager(LoaderManager&&)      = delete;
	~LoaderManager();

	LoaderManager& operator=(const LoaderManager&) = delete;
	LoaderManager& operator=(LoaderManager&&)      = delete;

	unsigned    nThread() const { return _nThread; }
	size_t      cacheSize();
	unsigned    nToLoad();
	const Path& basePath() const { return _basePath; }

	void setNThread(unsigned count);
	void setBasePath(const Path& path);

	template < typename L, typename... Args >
	std::shared_ptr<L> load(const std::string& file, Args&&... args) {
		auto it = _cache.find(file);
		if(it != _cache.end()) {
			log().debug("Serve ", file, " from cache.");
			return std::dynamic_pointer_cast<L>(it->second);
		}

		log().info("Request ", file, " loading...");
		// TODO: use a FileSystem to access file
		auto loader = std::make_shared<L>(this, (_basePath / file).c_str(),
		                                  std::forward<Args>(args)...);
		_enqueueLoader(loader);
		_cache.emplace(file, loader);
		return loader;
	}

	void clearCache();

	Logger& log() { return *_logger; }

	void _enqueueLoader(LoaderPtr loader);
	LoaderPtr _popLoader();
	void _doneLoading(Loader* loader, size_t size);

private:
	typedef std::deque<LoaderPtr> Queue;
	typedef std::unordered_map<std::string, LoaderPtr> Cache;

private:
	Logger*       _logger;

	std::mutex    _queueLock;
	std::condition_variable
	              _queueCv;
	Queue         _queue;

	std::mutex    _cacheLock;
	Cache         _cache;
	size_t        _cacheSize;
	size_t        _maxCacheSize;

	unsigned      _nThread;
	_LoaderThread _threadPool[MAX_LOADER_THREADS];

	Path          _basePath;
};


//class ImageLoader : public Loader {
//public:
//	ImageLoader();
//	virtual ~ImageLoader();

//	const Image* getImage() const;

//private:
//	Image* _image;
//};


}


#endif
