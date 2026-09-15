#pragma once

#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <list>
#include <memory>

#include <model.h>

#include <camera.h>

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

struct Plane {
  glm::vec3 normal = {0.f, 1.f, 0.f};
  float distance = 0.f;
  Plane() = default;

  Plane(const glm::vec3 &p1, const glm::vec3 &norm)
      : normal(glm::normalize(norm)), distance(glm::dot(normal, p1)) {}

  float getSignedDistanceToPlane(const glm::vec3 &point) const {
    return glm::dot(normal, point) - distance;
  }
};

struct Frustum {
  Plane topFace;
  Plane bottomFace;

  Plane leftFace;
  Plane rightFace;

  Plane farFace;
  Plane nearFace;
};

struct BoundingVolume {
  virtual bool isOnFrustum(const Frustum &camFrustum,
                           const Transform &modelTransform) const = 0;
  virtual bool isOnorForwardPlane(const Plane &plane) const = 0;

  bool isOnFrustum(const Frustum &camFrustum) const {
    return (isOnorForwardPlane(camFrustum.leftFace) &&
            isOnorForwardPlane(camFrustum.rightFace) &&
            isOnorForwardPlane(camFrustum.topFace) &&
            isOnorForwardPlane(camFrustum.bottomFace) &&
            isOnorForwardPlane(camFrustum.nearFace) &&
            isOnorForwardPlane(camFrustum.farFace));
  }
};

struct Sphere : public BoundingVolume {
  glm::vec3 center{0.f};
  float radius = 0.f;

  Sphere(const glm::vec3 &inCenter, float inRadius)
      : BoundingVolume{}, center{inCenter}, radius{inRadius} {}

  bool isOnorForwardPlane(const Plane &plane) const {
    return plane.getSignedDistanceToPlane(center) > -radius;
  }

  bool isOnFrustum(const Frustum &camFrustum,
                   const Transform &transform) const final {
    const glm::vec3 globalScale = transform.getGlobalScale();
    const glm::vec3 globalCenter{transform.getModelMatrix() *
                                 glm::vec4(center, 1.f)};
    const float maxScale =
        std::max(std::max(globalScale.x, globalScale.y), globalScale.z);

    Sphere globalSphere(globalCenter, radius * (maxScale * 0.5f));
    return (globalSphere.isOnorForwardPlane(camFrustum.leftFace) &&
            globalSphere.isOnorForwardPlane(camFrustum.rightFace) &&
            globalSphere.isOnorForwardPlane(camFrustum.farFace) &&
            globalSphere.isOnorForwardPlane(camFrustum.nearFace) &&
            globalSphere.isOnorForwardPlane(camFrustum.topFace) &&
            globalSphere.isOnorForwardPlane(camFrustum.bottomFace));
  }
};

struct AABB : public BoundingVolume {
  glm::vec3 center{0.f, 0.f, 0.f};
  glm::vec3 extents{0.f, 0.f, 0.f};

  AABB(const glm::vec3 &min, const glm::vec3 &max)
      : BoundingVolume{}, center{(max + min) * 0.5f},
        extents{max.x - center.x, max.y - center.y, max.z - center.z} {}

  AABB(const glm::vec3 &inCenter, float iI, float iJ, float iK)
      : BoundingVolume{}, center{inCenter}, extents{iI, iJ, iK} {}

};

class Entity : public Model {
public:
  std::list<std::unique_ptr<Entity>> children;
  Entity *parent = nullptr;

  Transform transform;

  Model *pModel = nullptr;
  Frustum createFrustumFromCamera(Camera &cam, float aspect, float znear,
                                  float zfar) {
    Frustum frustum;
    const float halfVside = zfar * tanf(cam.getFov() * .5f);
    const float halfHside = halfVside * aspect;
    const glm::vec3 frontMultFar = zfar * cam.getFront();

    glm::vec3 camPos = cam.getPos();
    glm::vec3 camFront = cam.getFront();
    glm::vec3 camUp = cam.getUp();

    glm::vec3 camRight = glm::cross(camFront, camUp);

    frustum.nearFace = {camPos + znear * camFront, camFront};
    frustum.farFace = {camPos + frontMultFar, -camFront};
    frustum.rightFace = {
        camPos, glm::cross(frontMultFar - camRight * halfHside, camUp)};
    frustum.leftFace = {camPos,
                        glm::cross(camUp, frontMultFar + camRight * halfHside)};
    frustum.topFace = {camPos,
                       glm::cross(camRight, frontMultFar - camUp * halfVside)};
    frustum.bottomFace = {
        camPos, glm::cross(frontMultFar + camUp * halfVside, camRight)};

    return frustum;
  }

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
