// Symphony of Empires
// Copyright (C) 2021, Symphony of Empires contributors
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
// ----------------------------------------------------------------------------
// Name:
//      unit.cpp
//
// Abstract:
//      Does some important stuff.
// ----------------------------------------------------------------------------

#include "unit.hpp"
#include "province.hpp"
#include "world.hpp"
#include <cmath>

//
// Unit type
//

/*
UnitType::UnitType(void) {

}

UnitType::~UnitType(void) {

}
//*/

//
// Unit trait
//

/*
UnitTrait::UnitTrait(void) {

}

UnitTrait::~UnitTrait(void) {
    
}
//*/

//
// Unit
//
Unit::Unit(void) {

}

Unit::~Unit(void) {
    if(province != nullptr) {
        auto it = std::find(province->units.begin(), province->units.end(), this);
        province->units.erase(it);
    }
}

void Unit::attack(Unit& enemy) {
    // TODO: Better attack algorithm
    
    /*
    // Calculate the attack of our unit
    float attack_mod = 0.f;
    for(const auto& trait: this->traits) {
        attack_mod *= trait->attack_mod;
    }
    const float attack = this->type->attack * attack_mod;

    // Calculate the defense of the enemy
    float defense_mod = 0.f;
    for(const auto& trait: this->traits) {
        defense_mod *= trait->defense_mod;
    }
    const float enemy_defense = std::max<float>(0.1f, enemy.type->defense * defense_mod);

    // Calculate the total damage dealt by our unit to the enemy
    const float damage_dealt = this->size * std::min<float>(10.f, std::max<float>(.05f, this->experience))
        * (attack / std::pow(std::min<float>(0.f, enemy_defense), 2))
        * std::max<float>(0.1f, this->morale) * this->supply
    ;
    
    // Deal with the morale loss of the enemy
    float enemy_fanaticism = 0.f;
    for(const auto& trait: enemy.traits) {
        enemy_fanaticism *= trait->morale_mod;
    }
    enemy.morale -= 10.f * enemy_fanaticism * damage_dealt / enemy.size;

    // Our unit receives half of the morale
    this->morale += 5.f * enemy_fanaticism * damage_dealt / enemy.size;

    // Deal the damage
    enemy.size -= std::min<size_t>(enemy.size, damage_dealt);
    */

    const float damage = (type->attack / enemy.type->defense) * size;
    enemy.size -= std::min<size_t>(enemy.size, damage);
}

std::pair<float, float> Unit::get_pos(void) const {
    return province->get_pos();
}

void Unit::set_target(Province& _province) {
    target = &_province;
    move_progress = std::sqrt(std::abs((province->max_x + ((province->max_x - province->min_x) / 2.f)) - (target->max_x + ((target->max_x - target->min_x) / 2.f))) + std::abs((province->max_y + ((province->max_y - province->min_y) / 2.f)) - (target->max_y + ((target->max_y - target->min_y) / 2.f))));
}

float Unit::get_speed(const Province& _province) const {
    auto start_pos = province->get_pos();
    auto end_pos = _province.get_pos();

    // Get the linear distance from the current deduced position of the unit and the target
    // the current position of the unit is relative to the move progress it has done (so if it's
    // halfway thru a province it will then be placed at half of the distance)
    const float x_dist = (end_pos.first - start_pos.first);
    const float y_dist = (end_pos.second - start_pos.second);
    const float angle = std::atan2(x_dist, y_dist);

    // TODO: The comment above makes no sense since we don't do (max_move_progress / move_progress)
    const float dist_div = move_progress;

    //const float linear_dist = std::fabs(std::sqrt(x_dist * x_dist + y_dist * y_dist) / dist_div);
    
    const float speed = type->speed / _province.terrain_type->movement_penalty;
    float radius_scale = std::cos(M_PI / (2 * World::get_instance().height) * (2 * (y_dist / dist_div) - World::get_instance().height));
    float x_scale = 1 / (std::fabs(radius_scale) + 0.001f);
    float speed_scale = std::sqrt(std::pow(std::sin(angle), 2) + std::pow(std::cos(angle) * x_scale, 2));
    return (speed * speed_scale) / 100.f;
}

float Unit::get_speed(void) const {
    return get_speed(*target);
}

void Unit::set_province(Province& _province) {
    // Delete the unit from the previous cache list of units
    if(province != nullptr) {
        auto it = std::find(province->units.begin(), province->units.end(), this);
        province->units.erase(it);
    }

    province = &_province;

    // Add unit to "cache list" of units
    province->units.push_back(this);
}