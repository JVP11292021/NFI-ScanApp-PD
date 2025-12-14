#include <iostream>

#include <Ray.hpp>

#include <CameraSystem.hpp>

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

#include <filesystem>
#include <chrono>
#include <memory>
#include <stdexcept>
#include <array>
#include <vector>

class CameraAdapter {
public:
	explicit CameraAdapter(sfm::CameraSystem& advancedCamera)
		: camera_(advancedCamera) {
	}

	const glm::mat4& getView() {
		view_ = camera_.getViewMatrix();
		//view_ = cvToEngineRotation() * view_;
		inverseView_ = glm::inverse(view_);
		return view_;
	}

	const glm::mat4& getProjection() {
		projection_ = camera_.getProjMatrix();
		return projection_;
	}

	const glm::mat4& getInverseView() {
		return inverseView_;
	}

private:
	sfm::CameraSystem& camera_;
	glm::mat4 view_{ 1.0f };
	glm::mat4 projection_{ 1.0f };
	glm::mat4 inverseView_{ 1.0f };
};

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
		//vle::Camera camera{};

		sfm::CameraSystem cam{
			glm::vec3(0.f, 0.f, 2.5f),
			glm::vec3(0.f, 0.f, 1.f)
		};
		CameraAdapter camAdapter{ cam };
		//std::vector<cv::Mat> intrinsicsCv{};
		//for (size_t i = 0; i < images.size(); ++i)
		//	intrinsicsCv.push_back(intrinsicsToCvMat(cam.getCameraIntrinsics()));

		//std::cout << "images size: " << images.size() << ", cam instrinsics: " << intrinsicsCv.size() << "\n";
		//sfm::SfM3D sfm(intrinsicsCv);
		//sfm.addImages(images);
		//sfm.extractFeatures();

		//std::cout << "feature size: " << sfm.features().size() << "\n";
		//sfm.matchFeatures(); // <-
		//std::cout << "Image pairs: " << sfm.matches().size() << "\n";
		//for (const auto& m : sfm.matches()) {
		//	std::cout << "Pair (" << m.img1 << "," << m.img2
		//		<< ") matches: " << m.matches.size() << "\n";
		//}
		//sfm.reconstruct(); // <-
		//std::cout << "World points size: " << sfm.points().size() << "\n";
		//renderSfMPoints(sfm.points(), objects);

		//auto viewerObject = vle::Object::create();
		//vle::KeyboardMovementController cameraController{};

		auto currentTime = std::chrono::high_resolution_clock::now();

		while (!this->win.shouldClose()) {
			glfwPollEvents();

			auto newTime = std::chrono::high_resolution_clock::now();
			float frameTimeElapsed = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
			currentTime = newTime;

			constexpr auto MAX_FRAME_TIME_ELAPSED = 10000.f;
			frameTimeElapsed = glm::min(frameTimeElapsed, MAX_FRAME_TIME_ELAPSED);

			//cameraController.moveInPlainXZ(this->win.getGLFWwindow(), frameTimeElapsed, viewerObject);
			//camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);
			if (glfwGetKey(win.getGLFWwindow(), GLFW_KEY_W) == GLFW_PRESS)
				cam.processKeyboard(sfm::FORWARD, frameTimeElapsed);
			if (glfwGetKey(win.getGLFWwindow(), GLFW_KEY_S) == GLFW_PRESS)
				cam.processKeyboard(sfm::BACKWARD, frameTimeElapsed);
			if (glfwGetKey(win.getGLFWwindow(), GLFW_KEY_A) == GLFW_PRESS)
				cam.processKeyboard(sfm::LEFT, frameTimeElapsed);
			if (glfwGetKey(win.getGLFWwindow(), GLFW_KEY_D) == GLFW_PRESS)
				cam.processKeyboard(sfm::RIGHT, frameTimeElapsed);

			auto aspect = this->renderer.getAspectRatio();
			//camera.setPerspectiveProjection(glm::radians(50.f), aspect, .1f, 25.f);

			if (auto commandBuffer = this->renderer.beginFrame()) {
				std::int32_t frameIndex = this->renderer.getFrameIndex();
				vle::FrameInfo frameInfo{
					frameIndex,
					frameTimeElapsed,
					commandBuffer,
					//camera,
					globalDescriptorSets[frameIndex],
					this->objects
				};

				// Update Phase
				vle::GlobalUbo ubo{};
				ubo.projection = camAdapter.getProjection();
				ubo.view = camAdapter.getView();
				ubo.inverseView = camAdapter.getInverseView();
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
		const int cols = 1;        // number of objects on X axis

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

		//std::shared_ptr<vle::ShaderModel> roomModel = vle::ShaderModel::createModelFromFile(this->device, "models/test_sfm.obj");
		//auto room = vle::Object::create();
		//room.model = roomModel;		
		//room.transform.translation = { 0.f, .5f, 0.f };
		//this->objects.emplace(room.getId(), std::move(room));
	}

private:
	vle::EngineWindow win{ WIDTH, HEIGHT, "Hello Vulkan" };
	vle::EngineDevice device{ win };
	vle::sys::Renderer renderer{ win, device };

	std::unique_ptr<vle::DescriptorPool> globalPool{};
	vle::ObjectMap objects;
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