#pragma once

#include <functional>
#include <list>
#include <unordered_map>

namespace colmap {

// Reimplementation of threading with thread-related functions outside
// controller Following util/threading.h
// BaseController that supports templating in ControllerThread at
// util/controller_thread.h
class BaseController {
 public:
  BaseController();
  virtual ~BaseController() = default;

  // Set callbacks that can be triggered within the main run function.
  void AddCallback(int id, std::function<void()> func);

  // Call back to the function with the specified name, if it exists.
  void Callback(int id) const;

  // This is the main run function to be implemented by the child class.
  virtual void Run() = 0;

  // check if the thread is stopped
  void SetCheckIfStoppedFunc(std::function<bool()> func);
  bool CheckIfStopped();

 protected:
  // Register a new callback. Note that only registered callbacks can be
  // set/reset and called from within the thread. Hence, this method should be
  // called from the derived thread constructor.
  void RegisterCallback(int id);

 private:
  // list of callbacks
  std::unordered_map<int, std::list<std::function<void()>>> callbacks_;
  // check_if_stop function
  std::function<bool()> check_if_stopped_fn_;
};

}  // namespace colmap
