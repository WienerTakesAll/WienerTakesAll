#include "UIObject.h"

UIObject::UIObject(glm::vec2 origin /* = glm::vec2(0.0f)*/, glm::vec3 colour /* = glm::vec3(0.0f)*/, float width /* = 0.0f*/, float height /*= 0.0f*/)
    : origin_(origin)
    , colour_(colour)
    , width_(width)
    , height_(height)
    , visible_(true) {
}

void UIObject::render() const {
    glBegin(GL_QUADS);
    glVertex2f(origin_.x - width_, origin_.y - height_);
    glVertex2f(origin_.x + width_, origin_.y - height_);
    glVertex2f(origin_.x + width_, origin_.y + height_);
    glVertex2f(origin_.x - width_, origin_.y + height_);
    glEnd();
}
