#include "unified_render/rectangle.hpp"

using namespace UnifiedRender;

Rectangle Rectangle::intersection(Rectangle rect) {
    float i_left = glm::max(this->left, rect.left);
    float i_top = glm::max(this->top, rect.top);
    float i_right = glm::min(this->right, rect.right);
    float i_bottom = glm::min(this->bottom, rect.bottom);
    return Rectangle{i_left, i_top, i_right - i_left, i_bottom - i_top};
}