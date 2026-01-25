#ifndef MARKER_MANAGER_H
#define MARKER_MANAGER_H

#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <Device.hpp>
#include <Object.hpp>
#include <Model.hpp>

class MarkerManager
{
public:
    MarkerManager();
    ~MarkerManager();

    void updateMarkerRotations(const glm::vec3& cameraPosition, vle::ObjectMap& objects);
    void loadMarkersFromTxt(const std::string& filePath, vle::EngineDevice& device, vle::ObjectMap& objects);
    
    void createMarker(const glm::vec3& position, vle::EngineDevice& device, vle::ObjectMap& objects);
    void destroyMarker(vle::id_t markerId, vle::ObjectMap& objects);
    bool isMarker(vle::id_t objectId) const;
    std::string getEvidenceId() const { return evidenceId; }

    void saveMarkersToTxt(const std::string& filePath, const vle::ObjectMap& objects);

private:
    std::vector<vle::id_t> markerIds;
    std::string currentFilePath;
    std::shared_ptr<vle::ShaderModel> markerPinModel;
    int nextMarkerNumber = 1;
    std::string evidenceId = "000002";
    glm::vec3 defaultMarkerColor = { 0.9f, .9f, .9f };
    glm::vec3 selectedMarkerColor = { 1.f, .1f, .1f };
};

#endif
