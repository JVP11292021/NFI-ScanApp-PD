#include <iostream>

#include <Ray.hpp>

#include <Renderer.hpp>
#include <ObjectRenderSystem.hpp>
#include <PointLightSystem.hpp>

#include <Device.hpp>
#include <defs.hpp>
#include <Window.hpp>
#include <Model.hpp>
#include <Object.hpp>
#include <Camera.hpp>
#include <HID.hpp>
#include <Buffer.hpp>
#include <Descriptors.hpp>
#include <eutils.hpp>

#include <chrono>
#include <memory>
#include <stdexcept>
#include <array>
#include <vector>
#include <PickingSystem.h>
#include <fstream>
#include <sstream>
#include <string>
//#include <iostream>

class FirstApp {
public:
	static constexpr std::int32_t WIDTH = 800;
	static constexpr std::int32_t HEIGHT = 600;

	FirstApp() {
		this->globalPool = vle::DescriptorPool::Builder(this->device)
			.setMaxSets(vle::EngineSwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, vle::EngineSwapChain::MAX_FRAMES_IN_FLIGHT)
			.build();

		this->loadObjects();
	}

	~FirstApp() {}

	FirstApp(const FirstApp&) = delete;
	void operator=(const FirstApp&) = delete;

	void run() {
		std::vector<std::unique_ptr<vle::Buffer>> uboBuffers(vle::EngineSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (std::int32_t i = 0; i < uboBuffers.size(); i++) {
			uboBuffers[i] = std::make_unique<vle::Buffer>(
				this->device,
				sizeof(vle::GlobalUbo),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			uboBuffers[i]->map();
		}

		auto globalSetLayout = vle::DescriptorSetLayout::Builder(this->device)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.build();

		std::vector<VkDescriptorSet> globalDescriptorSets(vle::EngineSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (std::int32_t i = 0; i < globalDescriptorSets.size(); i++) {
			auto bufferInfo = uboBuffers[i]->descriptorInfo();
			vle::DescriptorWriter(*globalSetLayout, *this->globalPool)
				.writeBuffer(0, &bufferInfo)
				.build(globalDescriptorSets[i]);
		}

		vle::sys::ObjectRenderSystem objectRenderSystem{ this->device, this->renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout() };
		vle::sys::PointLightSystem pointLigthSystem{ this->device, this->renderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout() };
		vle::Camera camera{};

		auto viewerObject = vle::Object::create();
		vle::KeyboardMovementController cameraController{};

		auto currentTime = std::chrono::high_resolution_clock::now();

		while (!this->win.shouldClose()) {
			glfwPollEvents();

			auto newTime = std::chrono::high_resolution_clock::now();
			float frameTimeElapsed = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
			currentTime = newTime;

			constexpr auto MAX_FRAME_TIME_ELAPSED = 10000.f;
			frameTimeElapsed = glm::min(frameTimeElapsed, MAX_FRAME_TIME_ELAPSED);

			cameraController.moveInPlainXZ(this->win.getGLFWwindow(), frameTimeElapsed, viewerObject);
			camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

			auto aspect = this->renderer.getAspectRatio();
			camera.setPerspectiveProjection(glm::radians(50.f), aspect, .1f, 25.f);

			updateMarkerRotations(viewerObject.transform.translation);

			static int prevMouseState = GLFW_RELEASE;
			int mouseState = glfwGetMouseButton(this->win.getGLFWwindow(), GLFW_MOUSE_BUTTON_LEFT);
			if (mouseState == GLFW_PRESS && prevMouseState == GLFW_RELEASE) {

				double mouseX, mouseY;
				glfwGetCursorPos(this->win.getGLFWwindow(), &mouseX, &mouseY);

				printf("Mouse Position: %f, %f\n", mouseX, mouseY);
				Ray ray = PickingSystem::buildRay((float)mouseX, (float)mouseY, WIDTH, HEIGHT, camera);

				// 4. Build the model matrix for the large model
				auto& room = this->objects.begin()->second; // example: get your large model
				glm::mat4 modelMatrix(1.0f);
				modelMatrix = glm::translate(modelMatrix, room.transform.translation);
				modelMatrix = glm::rotate(modelMatrix, room.transform.rotation.y, glm::vec3(0, 1, 0));
				modelMatrix = glm::rotate(modelMatrix, room.transform.rotation.x, glm::vec3(1, 0, 0));
				modelMatrix = glm::rotate(modelMatrix, room.transform.rotation.z, glm::vec3(0, 0, 1));
				modelMatrix = glm::scale(modelMatrix, room.transform.scale);

				// 5. Call intersection function (to implement next)
				PickResult result = pickingSystem.intersectModel(ray, room.model, modelMatrix);

				if (result.hit) {
					std::cout << "Hit at: " << result.position.x << ", " << result.position.y << ", " << result.position.z << "\n";
				}
			}
			prevMouseState = mouseState;

			if (auto commandBuffer = this->renderer.beginFrame()) {
				std::int32_t frameIndex = this->renderer.getFrameIndex();
				vle::FrameInfo frameInfo{
					frameIndex,
					frameTimeElapsed,
					commandBuffer,
					camera,
					globalDescriptorSets[frameIndex],
					this->objects
				};

				// Update Phase
				vle::GlobalUbo ubo{};
				ubo.projection = camera.getProjection();
				ubo.view = camera.getView();
				ubo.inverseView = camera.getImverseView();
				pointLigthSystem.update(frameInfo, ubo);
				uboBuffers[frameIndex]->writeToBuffer(&ubo);
				uboBuffers[frameIndex]->flush();

				// Render Phase
				this->renderer.beginSwapChainRenderPass(commandBuffer);
				objectRenderSystem.render(frameInfo);
				pointLigthSystem.render(frameInfo);
				this->renderer.endSwapChainRenderPass(commandBuffer);
				this->renderer.endFrame();
			}
		}

		vkDeviceWaitIdle(this->device.device());
	}
private:

	void loadObjects() {
		std::shared_ptr<vle::ShaderModel> model =
			vle::ShaderModel::createModelFromFile(this->device, "models/smooth_vase.obj");

		const int rows = 1;        // number of objects on Y axis
		const int cols = 8;        // number of objects on X axis

		const float startX = -2.0f;
		const float startY = 0.5f;
		const float zPos = 2.5f;

		const float spacingX = 0.6f;
		const float spacingY = 0.6f;

		for (int y = 0; y < rows; y++) {
			for (int x = 0; x < cols; x++) {

				auto obj = vle::Object::create();
				obj.model = model;

				obj.transform.translation = {
					startX + x * spacingX,
					startY + y * spacingY,
					zPos
				};

				obj.transform.scale = { 3.f, 1.5f, 3.f };

				this->objects.emplace(obj.getId(), std::move(obj));
			}
		}

		//std::shared_ptr<vle::ShaderModel> quadModel =
		//	vle::ShaderModel::createModelFromFile(this->device, "models/quad.obj");
		//auto obj = vle::Object::create();
		//obj.model = quadModel;
		//obj.transform.translation = { 0.f, .5f, 0.f };
		//obj.transform.scale = { 3.f, 1.0f, 3.f };
		//this->objects.emplace(obj.getId(), std::move(obj));

		std::vector<glm::vec3> lightColors{
			 {1.f, .1f, .1f},
			 {.1f, .1f, 1.f},
			 {.1f, 1.f, .1f},
			 {1.f, 1.f, .1f},
			 {.1f, 1.f, 1.f},
			 {1.f, 1.f, 1.f}  //
		};

		for (std::int32_t i = 0; i < lightColors.size(); i++) {
			auto pointLight = vle::Object::createPointLight(1.f);
			pointLight.color = lightColors[i];
			auto rotHeight = glm::rotate(
				glm::mat4(1.f),
				(i * glm::two_pi<float>()) / lightColors.size(),
				{ 0.f, 1.f, 0.f });
			pointLight.transform.translation = glm::vec3(rotHeight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
			this->objects.emplace(pointLight.getId(), std::move(pointLight));
		}

		std::shared_ptr<vle::ShaderModel> roomModel = vle::ShaderModel::createModelFromFile(this->device, "models/room1.obj");
		auto room = vle::Object::create();
		room.model = roomModel;		
		room.transform.translation = { 0.f, .5f, 0.f };
		this->objects.emplace(room.getId(), std::move(room));

		loadMarkersFromTxt("models/markers.txt", device, objects);
	}

	void loadMarkersFromTxt(const std::string& filePath, vle::EngineDevice& device, vle::ObjectMap& objects) {
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
			obj.transform.scale = {0.4f, 0.4f, 0.4f};
			obj.transform.rotation = { glm::pi<float>(), 0.f, 0.f };

			markerIds.push_back(obj.getId());

			objects.emplace(obj.getId(), std::move(obj));
		}
	}

	void updateMarkerRotations(const glm::vec3& cameraPosition) {
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
private:
	vle::EngineWindow win{ WIDTH, HEIGHT, "Hello Vulkan" };
	vle::EngineDevice device{ win };
	vle::sys::Renderer renderer{ win, device };

	std::unique_ptr<vle::DescriptorPool> globalPool{};
	vle::ObjectMap objects;
	PickingSystem pickingSystem;

	std::vector<vle::id_t> markerIds;
};

int main() {
	FirstApp app;
	try {
		app.run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << "\n";
		return EXIT_FAILURE;
	}
	return EXIT_SUCCESS;
}