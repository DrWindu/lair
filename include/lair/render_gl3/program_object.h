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


#ifndef _LAIR_RENDER_GL3_PROGRAM_OBJECT_H
#define _LAIR_RENDER_GL3_PROGRAM_OBJECT_H


#include <ostream>

#include <lair/render_gl3/context.h>


namespace lair {


class Context;
class Renderer;
class ShaderObject;


class ProgramObject {
public:
	ProgramObject(Renderer* renderer=nullptr);
	ProgramObject(const ProgramObject&) = delete;
	ProgramObject(ProgramObject&& other);
	~ProgramObject();

	ProgramObject& operator=(ProgramObject other);

	bool isValid() const;
	bool isGenerated() const;
	bool isLinked() const;

	GLuint id() const;

	void generateObject();
	void deleteObject();

	void attachShader(const ShaderObject& shader);
	void detachShader(const ShaderObject& shader);
	void detachAllShaders();

	void bindAttributeLocation(const GLchar* name, GLuint index) ;

	bool link();
	bool validate();

	void use() const;

	GLint nbActiveAttributes() const;
	GLint nbActiveUniforms() const;

	GLint getAttributeLocation(const GLchar* name) const;
	GLint getUniformLocation(const GLchar* name) const;

	void getLog(std::string& out) const;
	void dumpInfo(std::ostream& out) const;

	friend void swap(ProgramObject& p0, ProgramObject& p1);

private:
	Context*  _context;
	Renderer* _renderer;
	GLuint    _id;
	GLint     _link_status;
};


}

#endif