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
#include <cstdint>
#include <iostream>
#include <fstream>
#include <chrono>
#include <vector>

#include <Eigen/Dense>

#include <lair/core/lair.h>
#include <lair/core/log.h>

#include <lair/utils/input.h>
#include <lair/utils/image.h>

#include <lair/sys_sdl2/sys_module.h>
#include <lair/sys_sdl2/window.h>

#include <lair/render_gl2/shader_object.h>
#include <lair/render_gl2/program_object.h>
#include <lair/render_gl2/texture.h>

#include <lair/ec/entity_manager.h>
#include <lair/ec/sprite_component.h>


#ifndef SIMPLE_WINDOW_DATA_DIR
#define SIMPLE_WINDOW_DATA_DIR "."
#endif


typedef Eigen::Vector4f Vec4;
typedef Eigen::Vector2f Vec2;

bool running = true;
std::string dataDir;
lair::Logger glog("main");


bool quit() {
	running = false;
	return true;
}


int main(int /*argc*/, char** /*argv*/) {
	bool ok = true;

	lair::MasterLogger logger;

	lair::OStreamLogger clogBackend(std::clog, true);
	logger.addBackend(&clogBackend);

	glog.setLevel(lair::LogLevel::Debug);
	glog.setMaster(&logger);
//	glog.fatal  ("Fatal");
//	glog.error  ("Error");
//	glog.warning("Warning");
//	glog.log    ("Log");
//	glog.info   ("Info");
//	glog.debug  ("Debug");

	// ////////////////////////////////////////////////////////////////////////

//	dataDir = SIMPLE_WINDOW_DATA_DIR;
//	glog.log("Data directory: \"", dataDir, "\"");

//	lair::Loader loader;
//	lair::NativeFileSystem nativeFs(dataDir.data());
//	loader.mountFilesystem(&nativeFs);

	// ////////////////////////////////////////////////////////////////////////

	lair::SysModule sys(&logger);
	sys.log().setLevel(lair::LogLevel::Info);
	ok = sys.initialize();
	if(!ok) abort();
	sys.onQuit = quit;

	lair::Window* w = sys.createWindow("simple_window", 800, 600);
//	w->setFullscreen(true);

	// VSync works only with OpenGL.
	sys.setVSyncEnabled(true);
	glog.info("VSync: ", sys.isVSyncEnabled());

	lair::InputManager inputs(&sys, &glog);
	lair::Input* space = inputs.addInput("space");
	inputs.mapScanCode(space, SDL_SCANCODE_SPACE);
	lair::Input* left = inputs.addInput("left");
	inputs.mapScanCode(left, SDL_SCANCODE_LEFT);
	lair::Input* right = inputs.addInput("right");
	inputs.mapScanCode(right, SDL_SCANCODE_RIGHT);
	lair::Input* up = inputs.addInput("up");
	inputs.mapScanCode(up, SDL_SCANCODE_UP);
	lair::Input* down = inputs.addInput("down");
	inputs.mapScanCode(down, SDL_SCANCODE_DOWN);


	lair::ShaderObject vertShader;
	vertShader.generateObject(GL_VERTEX_SHADER);
	if(!vertShader.compileFromFile(SIMPLE_WINDOW_DATA_DIR "/vertex.glsl")) {
		std::cerr << "Vertex shader compilation failed:\n";
		vertShader.dumpLog(std::cerr);
		return false;
	}

	lair::ShaderObject fragShader;
	fragShader.generateObject(GL_FRAGMENT_SHADER);
	if(!fragShader.compileFromFile(SIMPLE_WINDOW_DATA_DIR "/fragment.glsl")) {
		std::cerr << "Fragment shader compilation failed:\n";
		fragShader.dumpLog(std::cerr);
		return false;
	}

	lair::ProgramObject shader;
	shader.generateObject();
	shader.attachShader(vertShader);
	shader.attachShader(fragShader);
	if(!shader.link()) {
		std::cerr << "Shader link failed:\n";
		shader.dumpLog(std::cerr);
		return false;
	}

	size_t imgSize = 128;
	Eigen::Vector2f center = Eigen::Vector2f(imgSize, imgSize) / 2;
	std::vector<lair::uint8> imgData;
	imgData.reserve(imgSize * imgSize * 4);
	for(size_t y = 0; y < imgSize; ++y) {
		for(size_t x = 0; x < imgSize; ++x) {
			Eigen::Vector2f pos = Eigen::Vector2f(x, y) + Eigen::Vector2f(.5, .5);
			float v = 1. - 2. * (center - pos).norm() / imgSize;
			lair::uint8 vi = std::min(std::max(v * 256, 0.f), 255.f);
			imgData.insert(imgData.end(), { vi, vi, vi, 255 });
		}
	}
	lair::Image img(imgSize, imgSize, lair::Image::FormatRGBA8, imgData.data());
	lair::Texture texture;
	texture.upload(img);


	Eigen::AlignedBox3f viewBox(Eigen::Vector3f(-400, -300, -1), Eigen::Vector3f(400, 300, 1));

	float rpl = viewBox.max()(0) + viewBox.min()(0);
	float tpb = viewBox.max()(1) + viewBox.min()(1);
	float fpn = viewBox.max()(2) + viewBox.min()(2);

	float rml = viewBox.max()(0) - viewBox.min()(0);
	float tmb = viewBox.max()(1) - viewBox.min()(1);
	float fmn = viewBox.max()(2) - viewBox.min()(2);

	Eigen::Matrix4f viewMatrix;
	viewMatrix << 2.f/rml,       0,        0, -rpl/rml,
	                    0, 2.f/tmb,        0, -tpb/tmb,
	                    0,       0, -2.f/fmn,  fpn/fmn,
	                    0,       0,        0,        1;

	std::cout << viewMatrix << "\n";


	lair::EntityManager entityManager;

	lair::EntityRef testSprite = entityManager.createEntity(entityManager.root(), "test");
	entityManager.addSpriteComponent(testSprite);
	testSprite.sprite()->setTexture(&texture);
	testSprite.setTransform(lair::Transform(
	                            lair::Translation(-lair::Vector3(imgSize, imgSize, 0) / 2)));


	shader.use();
	glUniformMatrix4fv(glGetUniformLocation(shader.id(), "viewMatrix"),
	                   1, false, viewMatrix.data());
	glUniform1i(glGetUniformLocation(shader.id(), "texture"), 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture._glId());

	while(running) {
//		sys.waitAndDispatchSystemEvents();
		sys.dispatchPendingSystemEvents();
		inputs.sync();
		if(space->justPressed()) glog.log("Space pressed");
		if(space->justReleased()) glog.log("Space released");

		float speed = 1;
		lair::Transform trans = testSprite.transform();
		if( left->isPressed()) trans.translate(lair::Vector3(-speed, 0, 0));
		if(right->isPressed()) trans.translate(lair::Vector3( speed, 0, 0));
		if(   up->isPressed()) trans.translate(lair::Vector3(0,  speed, 0));
		if( down->isPressed()) trans.translate(lair::Vector3(0, -speed, 0));
		testSprite.setTransform(trans);

		entityManager.updateWorldTransform();
		entityManager.render();
		w->swapBuffers();

		LAIR_LOG_OPENGL_ERRORS_TO(glog);
	}

	// ////////////////////////////////////////////////////////////////////////

	sys.shutdown();
	return EXIT_SUCCESS;
}