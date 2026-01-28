#include "base_controller.h"
#include "logging.h"

namespace colmap {

BaseController::BaseController() {}

void BaseController::AddCallback(const int id, std::function<void()> func) {
//  CHECK(func);
//  CHECK_GT(callbacks_.count(id), 0) << "Callback not registered";
  callbacks_.at(id).push_back(std::move(func));
}

void BaseController::RegisterCallback(const int id) {
  callbacks_.emplace(id, std::list<std::function<void()>>());
}

void BaseController::Callback(const int id) const {
//  CHECK_GT(callbacks_.count(id), 0) << "Callback not registered";
  for (const auto& callback : callbacks_.at(id)) {
    callback();
  }
}

void BaseController::SetCheckIfStoppedFunc(std::function<bool()> func) {
  check_if_stopped_fn_ = std::move(func);
}

bool BaseController::CheckIfStopped() {
  if (check_if_stopped_fn_)
    return check_if_stopped_fn_();
  else
    return false;
}

}  // namespace colmap
