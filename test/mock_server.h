#pragma once

#include <condition_variable>
#include <mutex>
#include <string>
#include <thread>
#include <queue>

#include <franka/robot_state.h>
#include <research_interface/service_types.h>
#include <research_interface/rbk_types.h>

class MockServer {
 private:
  struct Socket {
    std::function<void(const void*, size_t)> sendBytes;
    std::function<void(void*, size_t)> receiveBytes;
  };
  
 public:
  using ConnectCallbackT = std::function<research_interface::Connect::Response(const research_interface::Connect::Request&)>;
  using StartMotionGeneratorCallbackT = std::function<research_interface::StartMotionGenerator::Response(const research_interface::StartMotionGenerator::Request&)>;
  using StopMotionGeneratorCallbackT = std::function<research_interface::StopMotionGenerator::Response(const research_interface::StopMotionGenerator::Request&)>;
  using StartControllerCallbackT = std::function<research_interface::StartController::Response(const research_interface::StartController::Request&)>;
  using StopControllerCallbackT = std::function<research_interface::StopController::Response(const research_interface::StopController::Request&)>;
  using SendRobotStateAlternativeCallbackT = std::function<void(research_interface::RobotState&)>;
  using SendRobotStateCallbackT = std::function<research_interface::RobotState()>;
  using ReceiveRobotCommandCallbackT = std::function<void(const research_interface::RobotCommand&)>;

  MockServer();
  ~MockServer();

  MockServer& sendEmptyRobotState();

  template <typename TResponse>
  MockServer& sendResponse(std::function<TResponse()> create_response);

  MockServer& onConnect(ConnectCallbackT on_connect);
  MockServer& onStartMotionGenerator(StartMotionGeneratorCallbackT on_start_motion_generator);
  MockServer& onStopMotionGenerator(StopMotionGeneratorCallbackT on_stop_motion_generator);
  MockServer& onStartController(StartControllerCallbackT on_start_motion_generator);
  MockServer& onStopController(StopControllerCallbackT on_stop_motion_generator);

  MockServer& onSendRobotState(SendRobotStateCallbackT on_send_robot_state);
  MockServer& onSendRobotState(SendRobotStateAlternativeCallbackT on_send_robot_state);
  MockServer& onReceiveRobotCommand(ReceiveRobotCommandCallbackT on_receive_robot_command);

  template <typename T>
  void handleCommand(Socket& tcp_socket, std::function<typename T::Response(const typename T::Request&)> callback) {
    std::array<uint8_t, sizeof(typename T::Request)> buffer;
    tcp_socket.receiveBytes(buffer.data(), buffer.size());
    typename T::Request request(*reinterpret_cast<typename T::Request*>(buffer.data()));
    typename T::Response response = callback(request);
    tcp_socket.sendBytes(&response, sizeof(response));
  }

  template <typename T>
  MockServer& waitForCommand(std::function<typename T::Response(const typename T::Request&)> callback) {
    using namespace std::string_literals;

    std::lock_guard<std::mutex> _(mutex_);
    std::string name = "waitForCommand<"s + typeid(typename T::Request).name() + ", " + typeid(typename T::Response).name();
    commands_.emplace(name, [this,callback](Socket& tcp_socket, Socket&) {
      handleCommand<T>(tcp_socket, callback);
    });
    return *this;
  }

  void spinOnce(bool block = false);

private:
  void serverThread();

  std::condition_variable cv_;
  std::mutex mutex_;
  std::thread server_thread_;
  bool shutdown_;
  bool continue_;
  bool initialized_;

  ConnectCallbackT on_connect_;
  std::queue<std::pair<std::string, std::function<void(Socket&, Socket&)>>> commands_;
};

template <typename TResponse>
MockServer& MockServer::sendResponse(std::function<TResponse()> create_response) {
  using namespace std::string_literals;

  std::lock_guard<std::mutex> _(mutex_);
  commands_.emplace("sendResponse<"s + typeid(TResponse).name() + ">" , [=](Socket& tcp_socket, Socket&) {
    TResponse response = create_response();
    tcp_socket.sendBytes(&response, sizeof(response));
  });
  return *this;
}