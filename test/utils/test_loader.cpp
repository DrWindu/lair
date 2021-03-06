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


#include <cstdlib>
#include <chrono>
#include <thread>

#include <gtest/gtest.h>

#include <lair/utils/loader.h>


#define TEST_LOAD_COUNT 16

using namespace lair;


class TestLoader : public Loader {
public:
	TestLoader(LoaderManager* manager, const std::string &path)
	    : Loader(manager, path) {}

	virtual void loadSyncImpl(Logger& log) {
		std::this_thread::sleep_for(std::chrono::milliseconds(20 + std::rand() % 60));
		_success(1);
	}
};


TEST(LoaderTest, Startup) {
	MasterLogger mlog;
	OStreamLogger coutBackend(std::cout, true);
	mlog.addBackend(&coutBackend);
	Logger log("test", &mlog, LogLevel::Debug);

	LoaderManager lm(1 << 26, 2, log);

	ASSERT_EQ(2, lm.nThread());
	ASSERT_EQ(0, lm.cacheSize());
	ASSERT_EQ(0, lm.nToLoad());
}


TEST(LoaderTest, TestLoad) {
	std::srand(std::time(NULL));

	MasterLogger mlog;
	OStreamLogger coutBackend(std::cout, true);
	mlog.addBackend(&coutBackend);
	Logger log("test", &mlog, LogLevel::Debug);

	LoaderManager lm(1 << 26, 0, log);

	ASSERT_EQ(0, lm.nThread());

	typedef std::shared_ptr<TestLoader> LPtr;
	LPtr loaders[TEST_LOAD_COUNT];
	for(unsigned i = 0; i < TEST_LOAD_COUNT; ++ i) {
		std::string path = "file_" + std::to_string(i);
		loaders[i] = lm.load<TestLoader>(path);
		ASSERT_EQ(path, loaders[i]->realPath());
		ASSERT_EQ(0, loaders[i]->size());
	}
	ASSERT_EQ(TEST_LOAD_COUNT, lm.nToLoad());

	lm.setNThread(2);

	for(unsigned i = 0; i < TEST_LOAD_COUNT; ++ i) {
		if(!loaders[i]->isLoaded()) {
			log.info("Wait for ", loaders[i]->realPath(), "...");
			loaders[i]->wait();
			ASSERT_TRUE(loaders[i]->isLoaded());
			ASSERT_EQ(1, loaders[i]->size());
		}
		log.info("Done loading ", loaders[i]->realPath(), ".");
	}

	ASSERT_EQ(TEST_LOAD_COUNT, lm.cacheSize());
}
