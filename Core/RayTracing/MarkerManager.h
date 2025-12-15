#ifndef RT_MARKER_MANAGER_H
#define RT_MARKER_MANAGER_H

#include <string>
#include <Renderer.hpp>
#include <Object.hpp>
#include <CameraSystem.hpp>
#include <glm/glm.hpp>
#include <vector>
#include <fstream>
#include <sstream>

class MarkerManager
{
public:
	MarkerManager();
	~MarkerManager();
	void updateMarkerRotations(const glm::vec3& cameraPosition, vle::ObjectMap& objects);
	void loadMarkersFromTxt(const std::string& filePath, vle::EngineDevice& device, vle::ObjectMap& objects);

private:
	std::vector<vle::id_t> markerIds;
};

#endif // RT_MARKER_MANAGER_H
