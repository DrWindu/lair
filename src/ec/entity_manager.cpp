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


#include <cstring>
#include <cmath>

#include <lair/core/lair.h>
#include <lair/core/log.h>

#include <lair/ec/sprite_component.h>

#include "lair/ec/entity_manager.h"


namespace lair
{


EntityManager::EntityManager(size_t entityBlockSize)
    : _root           (nullptr),
      _firstFree      (nullptr),
      _entityBlockSize(entityBlockSize),
      _nEntities      (0),
      _nZombieEntities(0),
      _entities       (),
      _spriteManager  (new SpriteComponentManager()){
	_root = _createDetachedEntity("__root__");
}


EntityManager::~EntityManager() {
}


size_t EntityManager::entityCapacity() const {
	size_t cap = 0;
	for(const EntityBlock& block: _entities) {
		cap += block.size();
	}
	return cap;
}


EntityRef EntityManager::createEntity(EntityRef parent, const char* name) {
	lairAssert(parent.isValid());
	_Entity* entity = _createDetachedEntity(name);
	_addChild(parent._get(), entity);
	return EntityRef(entity);
}

void EntityManager::destroyEntity(EntityRef entity) {
	lairAssert(entity.isValid() && entity._get() != _root);

	while(entity.firstChild().isValid()) {
		destroyEntity(entity.firstChild());
	}

	if(entity.sprite()) {
		removeSpriteComponent(entity);
	}

	_detach(entity._get());
	entity._get()->reset();
	--_nEntities;
	++_nZombieEntities;
}

void EntityManager::_releaseEntity(_Entity* entity) {
	entity->nextSibling = _firstFree;
	_firstFree = entity;
	--_nZombieEntities;
}

void EntityManager::moveEntity(EntityRef& entity, EntityRef& newParent) {
	lairAssert(entity.isValid() && newParent.isValid());

	_detach(entity._get());
	_addChild(newParent._get(), entity._get());
}


void EntityManager::addSpriteComponent(EntityRef& entity) {
	lairAssert(entity.isValid());
	_spriteManager->addComponent(entity._get());
}


void EntityManager::removeSpriteComponent(EntityRef& entity) {
	lairAssert(entity.isValid());
	_spriteManager->removeComponent(entity._get());
}


void EntityManager::updateWorldTransform() {
	// TODO: Update this algorithm when using homogenous arrays to make it
	// more cache-firendly.

	_updateWorldTransformHelper(_root, Transform::Identity());
}


void EntityManager::render() {
	_spriteManager->render();
}


void EntityManager::_addEntityBlock() {
	_entities.emplace_back(_entityBlockSize);
	EntityBlock& block = _entities.back();
	std::memset(block.data(), 0, sizeof(_Entity) * block.size());
	for(_Entity& e: block) {
		e.nextSibling = &e + 1;
	}
	block.back().nextSibling = _firstFree;
	_firstFree = &block.front();
}

_Entity* EntityManager::_createDetachedEntity(const char* name) {
	// Do this first so there is no side effect in case of bad_alloc.
	std::unique_ptr<char> ownedName;
	if(name) {
		ownedName.reset(new char[std::strlen(name) + 1]);
		std::strcpy(ownedName.get(), name);
	}

	if(!_firstFree) {
		_addEntityBlock();
	}

	_Entity* entity = _firstFree;
	lairAssert(entity && entity->flags == 0 && entity->weakRefCount == 0);
	_firstFree = entity->nextSibling;
	entity->nextSibling = nullptr;

	entity->manager = this;
	entity->setAlive();
	++_nEntities;
	entity->transform.setIdentity();

	// It is safe to release the unique_pointer now, as the object is alive it
	// will be correctly deleted.
	entity->name = ownedName.release();

	return entity;
}

void EntityManager::_addChild(_Entity* parent, _Entity* child) {
	lairAssert(!child->parent && !child->nextSibling);
	child->parent = parent;
	child->nextSibling = parent->firstChild;
	parent->firstChild = child;
}

void EntityManager::_detach(_Entity* child) {
	lairAssert(child->parent);

	_Entity* prevSibling = nullptr;
	_Entity* e = child->parent->firstChild;
	lairAssert(e);
	while(e != child) {
		prevSibling = e;
		e = e->nextSibling;
		lairAssert(e);
	}

	if(prevSibling) {
		prevSibling->nextSibling  = child->nextSibling;
	} else {
		child->parent->firstChild = child->nextSibling;
	}

	child->parent = nullptr;
	child->nextSibling = nullptr;
}

void EntityManager::_updateWorldTransformHelper(
        _Entity* entity, const Transform& parentTransform) {
	entity->worldTransform = parentTransform * entity->transform;

	_Entity* child = entity->firstChild;
	while(child) {
		_updateWorldTransformHelper(child, entity->worldTransform);
		child = child->nextSibling;
	}
}


}
