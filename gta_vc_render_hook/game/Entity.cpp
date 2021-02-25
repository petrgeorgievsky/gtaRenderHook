//
// Created by peter on 23.05.2020.
//

#include "Entity.h"
#include <injection_utils/InjectorHelpers.h>

void Entity::CreateRwObject() { InMemoryVirtualFunc<void, 5>( this ); }

void Entity::PreRender() { InMemoryVirtualFunc<void, 12>( this ); }

void Entity::Render() { InMemoryVirtualFunc<void, 13>( this ); }