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


#ifndef _LAIR_RENDER_GL2_RENDERER_H
#define _LAIR_RENDER_GL2_RENDERER_H


#include <unordered_map>
#include <string>
#include <mutex>

#include <lair/core/lair.h>
#include <lair/core/log.h>
#include <lair/core/path.h>
#include <lair/core/asset_manager.h>

#include <lair/render_gl3/context.h>
#include <lair/render_gl3/glsl_source.h>
#include <lair/render_gl3/vertex_array.h>
#include <lair/render_gl3/shader_object.h>
#include <lair/render_gl3/program_object.h>
#include <lair/render_gl3/texture.h>


namespace lair
{


class Image;

class VertexAttribSet;
class RenderModule;


class Renderer {
public:
	Renderer(RenderModule* module, AssetManager* assetManager);
	Renderer(const Renderer&) = delete;
	Renderer(Renderer&&)      = delete;
	~Renderer();

	Renderer& operator=(const Renderer&) = delete;
	Renderer& operator=(Renderer&&)      = delete;

	Context* context();

	VertexArraySP createVertexArray(GLsizei sizeInBytes,
	                                const VertexAttrib* attribs,
	                                BufferObject* indices);

	ShaderObject compileShader(const char* name, GLenum type,
	                           const GlslSource& source);
	ProgramObject compileProgram(const char* name,
	                             const VertexAttribSet* attribs,
	                             const ShaderObject* vert,
	                             const ShaderObject* frag);

	TextureAspectSP createTexture(AssetSP asset);
	void enqueueToUpload(TextureAspectSP texture);
	void uploadPendingTextures();

	inline TextureAspectSP defaultTexture() {
		return _defaultTexture;
	}

	Logger& log();

protected:
	typedef std::vector<TextureAspectSP> TextureList;

protected:
	void _createDefaultTexture();

protected:
	RenderModule*   _module;
	AssetManager*   _assetManager;

	Context*        _context;

	unsigned        _vertexArrayCount;

	TextureList     _pendingTextures;
	TextureAspectSP _defaultTexture;
};


}


#endif