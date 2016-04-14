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


#include <vector>
#include <algorithm>

#include <lair/core/lair.h>
#include <lair/core/log.h>
#include <lair/core/image.h>

#include <lair/sys_sdl2/image_loader.h>

#include "lair/render_gl2/render_module.h"

#include "lair/render_gl2/renderer.h"


namespace lair
{


Renderer::Renderer(RenderModule* module, AssetManager* assetManager)
    : _module(module),
      _assetManager(assetManager),
      _context(module? module->context(): nullptr),
      _passStatesDirty(true),
      _defaultTexture() {
	lairAssert(_module);
	lairAssert(_assetManager);

//	_createDefaultTexture();
}


Renderer::~Renderer() {
}


Context* Renderer::context() {
	return _context;
}


PassStates* Renderer::currentPassStates() {
	return &_currentPassStates;
}


ShaderObject Renderer::compileShader(const char* name, GLenum type,
                                     const GlslSource& source) {
	ShaderObject shader(this, type);
	shader.generateObject();
	if(!shader.compile(source)) {
		std::string sLog;
		shader.getLog(sLog);
		log().error("Failed to compile ",
		            (type == gl::VERTEX_SHADER)? "vertex": "fragment",
		            " shader object \"", name, "\":\n", sLog);
	}
	return shader;
}


ProgramObject Renderer::compileProgram(const char* name,
                                       const VertexFormat* format,
                                       const ShaderObject* vert,
                                       const ShaderObject* frag) {
	ProgramObject prog(this);
	prog.generateObject();
	prog.attachShader(*vert);
	prog.attachShader(*frag);

	for(const VertexAttrib& attrib: *format) {
		prog.bindAttributeLocation(attrib.name, attrib.index);
		++format;
	}

	if(!prog.link()) {
		std::string sLog;
		prog.getLog(sLog);
		log().error("Failed to link shader \"", name, "\":\n", sLog);
	}
	prog.detachAllShaders();
	return prog;
}


TextureAspectSP Renderer::createTexture(AssetSP asset) {
	lairAssert(bool(asset));
	TextureAspectSP aspect = asset->aspect<TextureAspect>();
	if(!aspect) {
		aspect = asset->createAspect<TextureAspect>();
		enqueueToUpload(aspect);
	}
	return aspect;
}


void Renderer::enqueueToUpload(TextureAspectSP texture) {
	_pendingTextures.push_back(texture);
}


void Renderer::uploadPendingTextures() {
	for(TextureAspectSP texture: _pendingTextures) {
		ImageAspectSP image = texture->asset()->aspect<ImageAspect>();
		if(image && !texture->texture()) {
			log().info("Upload texture \"", texture->asset()->logicPath(), "\"...");
			TextureSP tex = std::make_shared<Texture>(this);
			tex->_upload(*image->image());
			texture->_setTexture(tex);
		}
	}

	auto end = std::remove_if(_pendingTextures.begin(), _pendingTextures.end(),
	                          [](TextureAspectSP tex) { return bool(tex->texture()); });
	_pendingTextures.erase(end, _pendingTextures.end());
}


Logger& Renderer::log() {
	return _module->log();
}


void Renderer::_createDefaultTexture() {
	unsigned size = 32;
	uint32 fillColor = 0xffffffff;
	uint32 bordColor = 0xffff00ff;
	uint32 lineColor = 0xff0000ff;

	std::vector<Byte> buffer(size * size * 4);
	uint32* pixels = reinterpret_cast<uint32*>(buffer.data());
	for(unsigned y = 0; y < size; ++y) {
		for(unsigned x = 0; x < size; ++x) {
			pixels[y * size + x] = fillColor;
		}
	}
	for(unsigned x = 0; x < size; ++x) {
		if(x > 0) {
			pixels[x * size + x - 1]    = lineColor;
			pixels[x * size + size - x] = lineColor;
		}
		pixels[x * size + x]            = lineColor;
		pixels[x * size + size - x - 1] = lineColor;
		if(x < size - 1) {
			pixels[x * size + x + 1]        = lineColor;
			pixels[x * size + size - x - 2] = lineColor;
		}
	}

	for(unsigned x = 0; x < size; ++x) {
		pixels[x * size + 0] = bordColor;
		pixels[x * size + 1] = bordColor;
		pixels[x * size + size - 1] = bordColor;
		pixels[x * size + size - 2] = bordColor;
		pixels[(0) * size + x] = bordColor;
		pixels[(1) * size + x] = bordColor;
		pixels[(size - 1) * size + x] = bordColor;
		pixels[(size - 2) * size + x] = bordColor;
	}

	Image img(size, size, Image::Format::FormatRGBA8, buffer.data());
	_defaultTexture._upload(img);
	_defaultTexture._setFlags(Texture::BILINEAR | Texture::REPEAT);
}


}
