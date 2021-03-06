/*
 *  Copyright (C) 2018 Simon Boyé
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


#include <lair/core/lair.h>
#include <lair/core/log.h>

#include <lair/ldl/read.h>

#include "lair/ldl/ldl_variant_loader.h"


namespace lair
{


LdlVariantLoader::LdlVariantLoader(LoaderManager* manager, AspectSP aspect)
    : Loader(manager, aspect) {
}


void LdlVariantLoader::commit() {
	VariantAspectSP aspect = static_pointer_cast<VariantAspect>(_aspect);
	aspect->_set(std::move(_variant));
	Loader::commit();
}


void LdlVariantLoader::loadSyncImpl(Logger& log) {
	_variant.reset(new Variant);
	parseLdl(*_variant, file(), asset()->logicPath(), log);
}


}
