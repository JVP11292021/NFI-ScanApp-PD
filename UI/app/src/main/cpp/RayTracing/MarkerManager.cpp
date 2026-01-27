#include "MarkerManager.h"

MarkerManager::MarkerManager(std::string evidenceId)
    : evidenceId(evidenceId)
{
    VLE_LOGD("MarkerManager initialized with evidenceId: ", this->evidenceId.c_str());
}

MarkerManager::~MarkerManager()
{
}

void MarkerManager::updateMarkerRotations(const glm::vec3& cameraPosition, vle::ObjectMap &objects)
{
    for (auto markerObjectId : markerIds) {
        auto it = objects.find(markerObjectId);
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
    currentFilePath = filePath;
    
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::ofstream createFile(filePath);
        createFile.close();
        return;
    }

    if (!markerPinModel) {
        markerPinModel = vle::ShaderModel::createModelFromFile(device, "markerPin.obj", vle::ModelLoadMode::ASSET_MANAGER);
    }

    std::string line;
    int maxMarkerNumber = 0;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        std::istringstream iss(line);
        std::string markerLabel, fileEvidenceId;
        float x, y, z;
        if (!(iss >> markerLabel >> x >> y >> z >> fileEvidenceId)) continue;

        try {
            int markerNumber = std::stoi(markerLabel.substr(6)); // Extract number from "MarkerN"
            if (markerNumber > maxMarkerNumber) maxMarkerNumber = markerNumber;
        } catch (...) {}

        auto obj = vle::Object::create();
        obj.model = markerPinModel;
        if (fileEvidenceId == evidenceId) {
            obj.color = selectedMarkerColor;

        } else {
            obj.color = defaultMarkerColor;
        }
        obj.transform.translation = { x, y, z };
        obj.transform.scale = { 1.f, 0.8f, 1.f };
        obj.transform.rotation = { glm::pi<float>(), 0.f, 0.f };

        vle::id_t objId = obj.getId();
        markerIds.push_back(objId);
        markerEvidenceIds[objId] = fileEvidenceId;  // Store the original evidence ID

        objects.emplace(objId, std::move(obj));
    }
    
    nextMarkerNumber = maxMarkerNumber + 1;
}

void MarkerManager::createMarker(const glm::vec3& position, vle::EngineDevice& device, vle::ObjectMap& objects)
{
    if (!markerPinModel) {
        markerPinModel = vle::ShaderModel::createModelFromFile(device, "markerPin.obj", vle::ModelLoadMode::ASSET_MANAGER);
    }

    auto obj = vle::Object::create();
    obj.model = markerPinModel;
    obj.color = selectedMarkerColor;
    obj.transform.translation = position;
    obj.transform.scale = { 1.f, 0.8f, 1.f };
    obj.transform.rotation = { glm::pi<float>(), 0.f, 0.f };

    vle::id_t newMarkerObjectId = obj.getId();
    markerIds.push_back(newMarkerObjectId);
    markerEvidenceIds[newMarkerObjectId] = evidenceId;  // Store current evidence ID

    objects.emplace(newMarkerObjectId, std::move(obj));

    if (!currentFilePath.empty()) {
        saveMarkersToTxt(currentFilePath, objects);
    }
}

void MarkerManager::destroyMarker(vle::id_t markerObjectId, vle::ObjectMap& objects)
{
    // Check if this marker belongs to the current evidence
    auto evidIt = markerEvidenceIds.find(markerObjectId);
    if (evidIt == markerEvidenceIds.end()) {
        VLE_LOGW("Marker not found in evidence ID map");
        return;
    }

    if (evidIt->second != evidenceId) {
        VLE_LOGW("Cannot delete marker: belongs to evidence ID '", evidIt->second.c_str(),
                 "' but current evidence ID is '", evidenceId.c_str(), "'");
        return;
    }

    // Remove from marker list
    auto it = std::find(markerIds.begin(), markerIds.end(), markerObjectId);
    if (it != markerIds.end()) {
        markerIds.erase(it);
    }

    markerEvidenceIds.erase(markerObjectId);  // Remove evidence ID mapping

    objects.erase(markerObjectId);

    if (!currentFilePath.empty()) {
        saveMarkersToTxt(currentFilePath, objects);
    }

    VLE_LOGI("Successfully deleted marker from evidence ID: ", evidenceId.c_str());
}

bool MarkerManager::isMarker(vle::id_t objectId) const
{
    return std::find(markerIds.begin(), markerIds.end(), objectId) != markerIds.end();
}

void MarkerManager::saveMarkersToTxt(const std::string& filePath, const vle::ObjectMap& objects)
{
    std::ofstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file for writing: " + filePath);
    }

    int markerNumber = 1;
    for (auto markerObjectId : markerIds) {
        auto it = objects.find(markerObjectId);
        if (it != objects.end()) {
            const auto& marker = it->second;
            const auto& pos = marker.transform.translation;
            
            // Get the stored evidence ID for this marker
            auto evidIt = markerEvidenceIds.find(markerObjectId);
            std::string markerEvidenceId = (evidIt != markerEvidenceIds.end())
                ? evidIt->second
                : evidenceId;  // Fallback to current evidenceId if not found

            file << "Marker" << markerNumber++ << " "
                 << pos.x << " " << pos.y << " " << pos.z << " "
                 << markerEvidenceId << "\n";
        }
    }

    file.close();
}

void MarkerManager::clearMarkers(vle::ObjectMap &objects) {
    for (auto markerObjectId : markerIds) {
        objects.erase(markerObjectId);
    }
    markerIds.clear();
    markerEvidenceIds.clear();

    if (!currentFilePath.empty()) {
        saveMarkersToTxt(currentFilePath, objects);
    }
}

bool MarkerManager::hasMarkers() const {
    return !markerIds.empty();
}

std::string MarkerManager::getMarkerEvidenceId(vle::id_t objectId) const {
    auto it = markerEvidenceIds.find(objectId);
    if (it != markerEvidenceIds.end()) {
        return it->second;
    }
    return "";
}

glm::vec3 MarkerManager::getMarkerPosition(vle::id_t objectId, const vle::ObjectMap& objects) const {
    auto it = objects.find(objectId);
    if (it != objects.end()) {
        return it->second.transform.translation;
    }
    return glm::vec3(0.0f, 0.0f, 0.0f);  // Return zero vector if marker not found
}

