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


#ifndef _LAIR_CORE_JSON_H
#define _LAIR_CORE_JSON_H


#include <istream>

#include <json/json.h>

#include <lair/core/lair.h>
#include <lair/core/log.h>
#include <lair/core/path.h>


namespace lair {


bool parseJson(Json::Value& value, std::istream& in, const Path& localPath, Logger& log);
bool parseJson(Json::Value& value, const Path& realPath, const Path& localPath, Logger& log);

Matrix4 parseMatrix4(const Json::Value& json, bool* ok = nullptr);


}

#endif
