#ifndef RT_PICKING_SYSTEM_H
#define RT_PICKING_SYSTEM_H

#include <glm/glm.hpp>
#include <vector>

#include <RenderSystem.hpp>
#include <CameraSystem.hpp>

struct Ray{
	glm::vec3 origin;
	glm::vec3 direction;
};

struct PickResult{
	bool hit = false;
	float distance = 0.f;
	glm::vec3 position{0.f};
};

class PickingSystem /*: public vle::sys::RenderSystem<>*/ {
public:
	PickingSystem() /*= default*/;
	~PickingSystem() /*override = default*/;

	//void update(vle::FrameInfo& frameInfo, vle::GlobalUbo& ubo) override {}
	//void render(vle::FrameInfo& frameInfo) override {}

	static Ray buildRay(float pointX, float pointY, int windowWidth, int windowHeight, const vle::sys::CameraSystem &camera);

	PickResult intersectModel(const Ray &ray, const std::shared_ptr<vle::ShaderModel>& model, const glm::mat4& modelMatrix);

private:
	static bool intersectTriangle(const Ray &ray, const glm::vec3 &v0, const glm::vec3 &v1, const glm::vec3 &v2, float &outDistance, glm::vec3 &outPosition);

	static bool gaussianIntersect(const Ray &ray, const glm::vec3 &mean, const glm::mat3 &covariance, const glm::mat4 &modelMatrix, float &outDistance, glm::vec3 &outPosition);
};

#endif // RT_PICKING_SYSTEM_H