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
//      eng3d/ui/unit_widget.cpp
//
// Abstract:
//      Does some important stuff.
// ----------------------------------------------------------------------------

#include "eng3d/ui/image.hpp"
#include "eng3d/ui/progress_bar.hpp"
#include "eng3d/camera.hpp"
#include "eng3d/color.hpp"

#include "client/interface/unit_widget.hpp"
#include "client/map.hpp"
#include "client/game_state.hpp"
#include "unit.hpp"
#include "nation.hpp"

using namespace Interface;

UnitWidget::UnitWidget(Unit* unit, Map* _map, UI::Widget* parent)
    : UI::Div(0, 0, 100, 30, parent), unit{ unit }, map{ _map }
{
    Eng3D::Camera* camera = map->camera;
    auto unit_pos = this->unit->get_pos();
    auto screen_pos = camera->get_tile_screen_pos(glm::vec2(unit_pos.first, unit_pos.second));

    this->x = screen_pos.x - this->width / 2;
    this->y = screen_pos.y - this->height / 2;

    this->background_color = Eng3D::Color(1, 1, 1, 1);

    auto nation_flag = map->nation_flags[unit->owner->cached_id];
    this->flag_img = new UI::Image(1, 1, 38, 28, nation_flag, this);

    this->size_label = new UI::Div(41, 1, 48, 28, this);
    auto unit_size = ((int)unit->size) / 1000;
    this->size_label->text(std::to_string(unit_size));
    this->size_label->text_align_x = UI::Align::END;
    this->size_label->background_color = Eng3D::Color(0.41f, 0.84f, 0.36f, 1.f);

    this->morale_bar = new UI::ProgressBar(91, 1, 8, 28, 0, 1);
    this->morale_bar->set_value(unit->morale);
}

void UnitWidget::set_unit(Unit* _unit) {
    unit = _unit;
    Eng3D::Camera* camera = map->camera;
    auto unit_pos = this->unit->get_pos();
    auto screen_pos = camera->get_tile_screen_pos(glm::vec2(unit_pos.first, unit_pos.second));

    this->x = screen_pos.x - this->width / 2;
    this->y = screen_pos.y - this->height / 2;

    auto nation_flag = map->nation_flags[unit->owner->cached_id];
    this->flag_img->current_texture = nation_flag;

    auto unit_size = ((int)unit->size) / 1000;
    this->size_label->text(std::to_string(unit_size));

    this->morale_bar->set_value(unit->morale);
}

UnitWidget::~UnitWidget() {

}