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


#include <lair/core/lair.h>
#include <lair/core/log.h>
#include <lair/core/json.h>

#include "lair/ec/collision_component.h"


namespace lair
{


Shape::Shape(ShapeType type, float radius)
	: _type  (type)
	, _radius(radius)
	, _points()
{
}


ShapeSP Shape::newAlignedBox(const Box2& box) {
	ShapeSP shape = std::make_shared<Shape>(SHAPE_ALIGNED_BOX);
	shape->_points.push_back(box.min());
	shape->_points.push_back(box.max());
	return shape;
}


ShapeSP Shape::newCircle(const Vector2& center, float radius) {
	ShapeSP shape = std::make_shared<Shape>(SHAPE_CIRCLE, radius);
	shape->_points.push_back(center);
	return shape;
}


bool ldlRead(LdlParser& parser, ShapeSP& value) {
	if(!parser.isValueTyped()) {
		parser.error("Type annotation is required for Shape objects");
		parser.skip();
		return false;
	}

	const String& type = parser.getValueTypeName();

	bool success = true;
	if(type == "ABox") {
		if(parser.valueType() != LdlParser::TYPE_MAP) {
			parser.error("ABox shape must be of type VarMap, got ", parser.valueTypeName());
			parser.skip();
			return false;
		}

		Vector2 min;
		Vector2 size;
		bool hasMin = false;
		bool hasSize = false;

		parser.enter();
		while(parser.valueType() != LdlParser::TYPE_END) {
			const String& key = parser.getKey();
			if(key == "min") {
				if(hasMin) {
					parser.warning("Duplicate key \"", key, "\": ignoring");
				}
				else {
					hasMin = ldlRead(parser, min);
					success &= hasMin;
				}
			}
			else if(key == "size") {
				if(hasSize) {
					parser.warning("Duplicate key \"", key, "\": ignoring");
				}
				else {
					hasSize = ldlRead(parser, size);
					success &= hasSize;
				}
			}
			else {
				parser.warning("Unexpected key \"", key, "\": ignoring");
			}
		}

		if(!hasMin) {
			parser.error("Key \"min\" is required for ABox shapes");
			success = false;
		}
		if(!hasSize) {
			parser.error("Key \"size\" is required for ABox shapes");
			success = false;
		}

		if(success) {
			Box2 box(min, min + size);
			value = Shape::newAlignedBox(box);
		}

		parser.leave();
	}
	else if(type == "Circle") {
		if(parser.valueType() != LdlParser::TYPE_MAP) {
			parser.error("Circle shapes must be of type VarMap, got ", parser.valueTypeName());
			parser.skip();
			return false;
		}

		Vector2 center;
		float radius;
		bool hasCenter = false;
		bool hasRadius = false;

		parser.enter();
		while(parser.valueType() != LdlParser::TYPE_END) {
			const String& key = parser.getKey();
			if(key == "center") {
				if(hasCenter) {
					parser.warning("Duplicate key \"", key, "\": ignoring");
				}
				else {
					hasCenter = ldlRead(parser, center);
					success &= hasCenter;
				}
			}
			else if(key == "radius") {
				if(hasRadius) {
					parser.warning("Duplicate key \"", key, "\": ignoring");
				}
				else {
					hasRadius = ldlRead(parser, radius);
					success &= hasRadius;
				}
			}
			else {
				parser.warning("Unexpected key \"", key, "\": ignoring");
			}
		}

		if(!hasCenter) {
			parser.error("Key \"center\" is required for Circle shapes");
			success = false;
		}
		if(!hasRadius) {
			parser.error("Key \"Radius\" is required for Circle shapes");
			success = false;
		}

		if(success) {
			value = Shape::newCircle(center, radius);
		}

		parser.leave();
	}
	else if(type == "Polygon") {
		// TODO:
		parser.error("Polygon shapes reader not implemented");
		return false;
	}

	return success;
}


bool ldlWrite(LdlWriter& writer, const ShapeSP& value) {
	bool success = true;

	switch(value->type()) {
	case SHAPE_ALIGNED_BOX: {
		writer.openMap(LdlWriter::CF_SINGLE_LINE, "ABox");
		writer.writeKey("min");
		success &= ldlWrite(writer, value->point(0));
		writer.writeKey("size");
		Vector2 size = value->point(1) - value->point(0);
		success &= ldlWrite(writer, size);
		writer.close();
		break;
	}
	case SHAPE_CIRCLE: {
		writer.openMap(LdlWriter::CF_SINGLE_LINE, "Circle");
		writer.writeKey("center");
		success &= ldlWrite(writer, value->point(0));
		writer.writeKey("radius");
		success &= ldlWrite(writer, value->radius());
		writer.close();
		break;
	}
	case SHAPE_POLYGON: {
		writer.openList(LdlWriter::CF_MULTI_LINE, "Polygon");
		for(unsigned i = 0; i < value->nPoints(); ++i)
			success &= ldlWrite(writer, value->point(i));
		writer.close();
		break;
	}
	}

	return success;
}



CollisionComponent::CollisionComponent(Manager* manager, _Entity* entity, ShapeSP shape)
	: Component   (manager, entity)
	, _shape      (shape)
	, _hitMask    (1u)
	, _ignoreMask (0u)
	, _penetration()
{
	for(int i = 0; i < N_DIRECTIONS; ++i)
		_penetration[i] = 0;
}


const PropertyList& CollisionComponent::properties() {
	static PropertyList props;
	if(props.nProperties() == 0) {
		props.addProperty("shape",
		                  &CollisionComponent::shape,
		                  &CollisionComponent::setShape);
		props.addProperty("hit_mask",
		                  &CollisionComponent::hitMask,
		                  &CollisionComponent::setHitMask);
		props.addProperty("ignore_mask",
		                  &CollisionComponent::ignoreMask,
		                  &CollisionComponent::setIgnoreMask);
	}
	return props;
}


Box2 CollisionComponent::worldAlignedBox() const {
	lairAssert(_shape && _shape->type() == SHAPE_ALIGNED_BOX);
	Vector2 pos = _entityPtr->worldTransform.translation().head<2>();
	return Box2(pos + _shape->point(0), pos + _shape->point(1));
}



CollisionComponentManager::CollisionComponentManager(size_t componentBlockSize)
	: DenseComponentManager<CollisionComponent>("collision", componentBlockSize)
{
}


CollisionComponent* CollisionComponentManager::addComponentFromJson(
        EntityRef entity, const Json::Value& json, const Path& /*cd*/) {
	CollisionComponent* comp = addComponent(entity);

	if(json.isMember("abox")) {
		Json::Value box = json["abox"];
		if(box.isArray() || box.size() == 4) {
			comp->setShape(Shape::newAlignedBox(Box2(
			        Vector2(box[0].asFloat(), box[1].asFloat()),
			        Vector2(box[2].asFloat(), box[3].asFloat()))));
		} /*else {
			log().warning("Invalid anchor field while loading entity \"", entity.name(), "\".");
		}*/
	}

	if(json.isMember("hit_mask")) {
		comp->setHitMask(json["hit_mask"].asUInt());
	}

	if(json.isMember("ignore_mask")) {
		comp->setIgnoreMask(json["ignore_mask"].asUInt());
	}

	return comp;
}


CollisionComponent* CollisionComponentManager::cloneComponent(EntityRef base, EntityRef entity) {
	CollisionComponent* baseComp = get(base);
	CollisionComponent* comp = _addComponent(entity, baseComp);
	comp->setShape     (baseComp->shape());
	comp->setHitMask   (baseComp->hitMask());
	comp->setIgnoreMask(baseComp->ignoreMask());
	return comp;
}


void CollisionComponentManager::findCollisions(HitEventQueue& hitQueue) {
//	compactArray();

	// TODO: Less brute-force approach.
	// TODO: Support shapes other than aligned boxes
	HitEvent hit;
	for(int ci0 = 0; ci0 < int(nComponents()); ++ci0) {
		CollisionComponent& c0 = _components[ci0];
		if(!c0.isAlive() || !c0.entity().isEnabledRec() || !c0.isEnabled()
		|| !c0.shape() || c0.shape()->type() != SHAPE_ALIGNED_BOX)
			continue;
		hit.entities[0] = c0.entity();

		for(int ci1 = ci0 + 1; ci1 < int(nComponents()); ++ci1) {
			CollisionComponent& c1 = _components[ci1];

			if(!c1.shape() || c1.shape()->type() != SHAPE_ALIGNED_BOX
			|| (c0.hitMask() & c1.hitMask())    == 0
			|| (c0.hitMask() & c1.ignoreMask()) != 0
			|| (c1.hitMask() & c0.ignoreMask()) != 0
			|| !c1.entity().isEnabledRec() || !c1.isEnabled())
				continue;
			hit.entities[1] = c1.entity();

			hit.boxes[0] = c0.worldAlignedBox();
			hit.boxes[1] = c1.worldAlignedBox();

			if(!hit.boxes[0].intersection(hit.boxes[1]).isEmpty())
				hitQueue.push_back(hit);
		}
	}
}


bool CollisionComponentManager::hitTest(std::deque<EntityRef>& hits, const Box2& box, unsigned hitMask) {
	bool found = false;
	for(int ci = 0; ci < int(nComponents()); ++ci) {
		CollisionComponent& c = _components[ci];
		if(!c.entity().isEnabledRec() || !c.isEnabled()
		|| !c.shape() || c.shape()->type() != SHAPE_ALIGNED_BOX
		|| (hitMask & c.hitMask())    == 0
		|| (hitMask & c.ignoreMask()) != 0)
			continue;

		Box2    cbox = c.worldAlignedBox();
		if(!cbox.intersection(box).isEmpty()) {
			hits.push_back(c.entity());
			found = true;
		}
	}
	return found;
}


bool CollisionComponentManager::hitTest(std::deque<EntityRef>& hits, const Vector2& p, unsigned hitMask) {
	return hitTest(hits, Box2(p, p), hitMask);
}


}
