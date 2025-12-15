#include "MarkerManager.h"

MarkerManager::MarkerManager()
{
}

MarkerManager::~MarkerManager()
{
}

void MarkerManager::updateMarkerRotations(const glm::vec3& cameraPosition, vle::ObjectMap &objects)
{
	for (auto markerId : markerIds) {
		auto it = objects.find(markerId);
		if (it != objects.end()) {
			auto& marker = it->second;

			glm::vec3 direction = cameraPosition - marker.transform.translation;
			direction.y = 0.0f;

			float angle = std::atan2(direction.x, direction.z);

			marker.transform.rotation = { glm::pi<float>(), angle, 0.f };
		}
	}
}

void MarkerManager::loadMarkersFromTxt(const std::string& filePath, vle::EngineDevice& device, vle::ObjectMap& objects)
{
	std::ifstream file(filePath);
	if (!file.is_open()) throw std::runtime_error("Cannot open file");

	std::shared_ptr<vle::ShaderModel> markerPinModel =
		vle::ShaderModel::createModelFromFile(device, "models/markerPin.obj");

	std::string line;
	while (std::getline(file, line)) {
		std::istringstream iss(line);
		std::string markerId, SINnumber;
		float x, y, z;
		if (!(iss >> markerId >> x >> y >> z >> SINnumber)) continue;

		auto obj = vle::Object::create();
		obj.model = markerPinModel;
		obj.color = { 1.f, .1f, .1f };
		obj.transform.translation = { x, y, z };
		obj.transform.scale = { 0.4f, 0.3f, 0.4f };
		obj.transform.rotation = { glm::pi<float>(), 0.f, 0.f };

		markerIds.push_back(obj.getId());

		objects.emplace(obj.getId(), std::move(obj));
	}
}

void MarkerManager::createMarker()
{
}

void MarkerManager::destroyMarker()
{
}


