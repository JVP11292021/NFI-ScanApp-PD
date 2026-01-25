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
    currentFilePath = filePath;
    
    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::ofstream createFile(filePath);
        createFile.close();
        return;
    }

    if (!markerPinModel) {
        markerPinModel = vle::ShaderModel::createModelFromFile(device, "markerPin.obj");
    }

    std::string line;
    int maxMarkerNum = 0;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        std::istringstream iss(line);
        std::string markerId, SINnumber;
        float x, y, z;
        if (!(iss >> markerId >> x >> y >> z >> SINnumber)) continue;

        try {
            int markerNum = std::stoi(markerId.substr(6));
            if (markerNum > maxMarkerNum) maxMarkerNum = markerNum;
        } catch (...) {}

        auto obj = vle::Object::create();
        obj.model = markerPinModel;
        obj.color = defaultMarkerColor;
        obj.transform.translation = { x, y, z };
        obj.transform.scale = { 1.f, 0.8f, 1.f };
        obj.transform.rotation = { glm::pi<float>(), 0.f, 0.f };

        markerIds.push_back(obj.getId());

        objects.emplace(obj.getId(), std::move(obj));
    }
    
    nextMarkerNumber = maxMarkerNum + 1;
}

void MarkerManager::createMarker(const glm::vec3& position, vle::EngineDevice& device, vle::ObjectMap& objects)
{
    if (!markerPinModel) {
        markerPinModel = vle::ShaderModel::createModelFromFile(device, "markerPin.obj");
    }

    auto obj = vle::Object::create();
    obj.model = markerPinModel;
    obj.color = defaultMarkerColor;
    obj.transform.translation = position;
    obj.transform.scale = { 1.f, 0.8f, 1.f };
    obj.transform.rotation = { glm::pi<float>(), 0.f, 0.f };

    vle::id_t newMarkerId = obj.getId();
    markerIds.push_back(newMarkerId);

    objects.emplace(newMarkerId, std::move(obj));

    if (!currentFilePath.empty()) {
        saveMarkersToTxt(currentFilePath, objects);
    }
}

void MarkerManager::destroyMarker(vle::id_t markerId, vle::ObjectMap& objects)
{
    auto it = std::find(markerIds.begin(), markerIds.end(), markerId);
    if (it != markerIds.end()) {
        markerIds.erase(it);
    }

    objects.erase(markerId);

    if (!currentFilePath.empty()) {
        saveMarkersToTxt(currentFilePath, objects);
    }
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

    int markerNum = 1;
    for (auto markerId : markerIds) {
        auto it = objects.find(markerId);
        if (it != objects.end()) {
            const auto& marker = it->second;
            const auto& pos = marker.transform.translation;
            
            file << "Marker" << markerNum++ << " "
                 << pos.x << " " << pos.y << " " << pos.z << " "
                 << "SIN000" << markerNum - 1 << "\n";
        }
    }

    file.close();
}


