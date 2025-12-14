#include "PickingSystem.h"

Ray PickingSystem::buildRay(float pointX, float pointY, int windowWidth, int windowHeight, const vle::Camera& camera)
{
	float x = (2.0f * pointX) / windowWidth - 1.0f;
	float y = 1.0f - (2.0f * pointY) / windowHeight;

	glm::vec4 rayClip = glm::vec4(x, y, -1.0f, 1.0f);

	glm::vec4 rayEye = glm::inverse(camera.getProjection()) * rayClip;
	rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

	glm::vec4 rayWorld4 = glm::inverse(camera.getView()) * glm::vec4(rayEye.x, rayEye.y, rayEye.z, 0.0f);
	glm::vec3 rayWorld = glm::normalize(glm::vec3(rayWorld4));

	Ray ray{camera.getPosition(), rayWorld};

	return ray;
}

PickResult PickingSystem::intersectModel(const Ray& ray, const std::shared_ptr<vle::ShaderModel>& model, const glm::mat4& modelMatrix)
{
	PickResult result;
	
	return result;
}

bool PickingSystem::intersectTriangle(const Ray& ray, const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2, float& outDistance, glm::vec3& outPosition)
{
	// Möller–Trumbore intersection algorithm
	const float EPSILON = 1e-8f;
	glm::vec3 edge1 = v1 - v0;
	glm::vec3 edge2 = v2 - v0;
	glm::vec3 h = glm::cross(ray.direction, edge2);
	float a = glm::dot(edge1, h);

	// If a is near zero, the ray lies in the plane of the triangle
	if (fabs(a) < EPSILON)
	{
		return false;
	}

	// Calculate the barycentric coordinates
	float f = 1.0f / a;
	glm::vec3 s = ray.origin - v0;
	float u = f * glm::dot(s, h);
	// Check if the intersection is outside the triangle
	if (u < 0.0f || u > 1.0f)
	{
		return false;
	}

	// Continue calculating barycentric coordinates
	glm::vec3 q = glm::cross(s, edge1);
	float v = f * glm::dot(ray.direction, q);
	// Check if the intersection is outside the triangle
	if (v < 0.0f || u + v > 1.0f)
	{
		return false;
	}

	// Calculate the distance along the ray to the intersection point
	float t = f * glm::dot(edge2, q);
	// If t is positive, we have an intersection
	if (t > EPSILON)
	{
		outDistance = t;
		outPosition = ray.origin + ray.direction * t;
		return true;
	}
	return false;
}

bool PickingSystem::gaussianIntersect(
	const Ray& ray,
	const glm::vec3& mean,
	const glm::mat3& covariance,
	const glm::mat4& modelMatrix,
	float& outDistance,
	glm::vec3& outPosition)
{
	// --- Transform Gaussian into world space ---
	// Gaussian ellipsoids in world space are M * Σ * M^T
	glm::mat3 M = glm::mat3(modelMatrix);
	glm::mat3 worldCov = M * covariance * glm::transpose(M);

	// Inverse covariance
	glm::mat3 invCov = glm::inverse(worldCov);

	// Gaussian center in world space
	glm::vec3 worldMean = glm::vec3(modelMatrix * glm::vec4(mean, 1.0f));

	// --- Move ray origin relative to Gaussian center ---
	glm::vec3 p = ray.origin - worldMean;
	const glm::vec3& d = ray.direction;

	// --- Compute quadratic coefficients ---
	float a = glm::dot(d, invCov * d);
	float b = 2.0f * glm::dot(p, invCov * d);
	float c = glm::dot(p, invCov * p) - 1.0f;  // ellipsoid boundary

	// --- Compute discriminant  ---
	float discriminant = b * b - 4.0f * a * c;
	if (discriminant < 0.0f)
		return false;

	float sqrtDisc = sqrtf(glm::max(discriminant, 0.0f));

	float t1 = (-b - sqrtDisc) / (2 * a);
	float t2 = (-b + sqrtDisc) / (2 * a);

	// --- Select the smallest positive t ---
	float t = -1.0f;
	if (t1 > 0.0f) t = t1;
	else if (t2 > 0.0f) t = t2;

	if (t <= 0.0f)
		return false;

	// --- Output intersection ---
	outDistance = t;
	outPosition = ray.origin + d * t;

	return true;
}
