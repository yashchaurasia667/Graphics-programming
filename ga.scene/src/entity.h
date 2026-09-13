#pragma once

#include <list>
#include <memory>
#include <model.h>

#include <glm/glm.hpp>
#include <string>

class Transform {
protected:
  glm::vec3 pos = {0.0f, 0.0f, 0.0f};
  glm::vec3 eulerRot = {0.0f, 0.0f, 0.0f};
  glm::vec3 scale = {1.0f, 1.0f, 1.0f};

  glm::mat4 modelMatrix = glm::mat4(1.0f);
  bool is_dirty = true;

  glm::mat4 getLocalModelMatrix() {
    const glm::mat4 transformX = glm::rotate(
        glm::mat4(1.0f), glm::radians(eulerRot.x), glm::vec3(1.0f, 0.0f, 0.0f));
    const glm::mat4 transformY = glm::rotate(
        glm::mat4(1.0f), glm::radians(eulerRot.y), glm::vec3(0.0f, 1.0f, 0.0f));
    const glm::mat4 transformZ = glm::rotate(
        glm::mat4(1.0f), glm::radians(eulerRot.z), glm::vec3(0.0f, 0.0f, 1.0f));

    const glm::mat4 rotationMatrix = transformY * transformX * transformZ;

    return glm::translate(glm::mat4(1.0f), pos) * rotationMatrix *
           glm::scale(glm::mat4(1.0f), scale);
  }

public:
  void computeModelMatrix() {
    modelMatrix = getLocalModelMatrix();
    is_dirty = false;
  }

  void computeModelMatrix(const glm::mat4 &parentGlobalModelMatrix) {
    modelMatrix = parentGlobalModelMatrix * getLocalModelMatrix();
    is_dirty = false;
  }

  void setLocalPosition(const glm::vec3 &newPosition) {
    pos = newPosition;
    is_dirty = true;
  }

  void setLocalRotation(const glm::vec3 &newRotation) {
    eulerRot = newRotation;
    is_dirty = true;
  }

  void setLocalScale(const glm::vec3 &newScale) {
    scale = newScale;
    is_dirty = true;
  }

  const glm::vec3 &getGlobalPosition() const { return modelMatrix[3]; }

  const glm::vec3 &getLocalPosition() const { return pos; }

  const glm::vec3 &getLocalRotation() const { return eulerRot; }

  const glm::vec3 &getLocalScale() const { return scale; }

  const glm::mat4 &getModelMatrix() const { return modelMatrix; }

  glm::vec3 getRight() const { return modelMatrix[0]; }

  glm::vec3 getUp() const { return modelMatrix[1]; }

  glm::vec3 getBackward() const { return modelMatrix[2]; }

  glm::vec3 getForward() const { return -modelMatrix[2]; }

  glm::vec3 getGlobalScale() const {
    return {glm::length(getRight()), glm::length(getUp()),
            glm::length(getBackward())};
  }

  bool isDirty() const { return is_dirty; }
};

class Entity : public Model {
public:
  std::list<std::unique_ptr<Entity>> children;
  Entity *parent = nullptr;

  Transform transform;

  Model *pModel = nullptr;

  Entity(std::string const &path, bool gamma = false)
      : Model(path, glm::vec3(0.0f), glm::vec3(0.0f), glm::vec3(1.0f), gamma) {}

  template <typename... TArgs> void addChild(const TArgs... args) {
    children.emplace_back(std::make_unique<Entity>(args...));
    children.back()->parent = this;
  }

  void updateSelfAndChild() {
    if (transform.isDirty()) {
      forceUpdateSelfAndChild();
      return;
    }

    for (auto &&child : children) {
      child->updateSelfAndChild();
    }
  }

  void forceUpdateSelfAndChild() {
    if (parent)
      transform.computeModelMatrix(parent->transform.getModelMatrix());
    else
      transform.computeModelMatrix();

    for (auto &&child : children) {
      child->forceUpdateSelfAndChild();
    }
  }
};
