#include "franka/robot_state.h"

#include <algorithm>
#include <cstring>
#include <iterator>

namespace franka {

namespace {
template <class T, size_t N>
std::ostream& operator<<(std::ostream& ostream, const std::array<T, N>& array) {
  ostream << "[";
  std::copy(array.cbegin(), array.cend() - 1, std::ostream_iterator<T>(ostream, ","));
  std::copy(array.cend() - 1, array.cend(), std::ostream_iterator<T>(ostream));
  ostream << "]";
  return ostream;
}
}  // anonymous namespace

std::ostream& operator<<(std::ostream& ostream, const franka::RobotState& robot_state) {
  ostream << "{O_T_EE: " << robot_state.O_T_EE << ", elbow: " << robot_state.elbow
          << ", O_T_EE_d: " << robot_state.O_T_EE_d << ", elbow_d: " << robot_state.elbow_d
          << ", EE_T_K: " << robot_state.EE_T_K << ", tau_J: " << robot_state.tau_J
          << ", dtau_J: " << robot_state.dtau_J << ", q: " << robot_state.q
          << ", dq: " << robot_state.dq << ", q_d: " << robot_state.q_d
          << ", joint_contact: " << robot_state.joint_contact
          << ", cartesian_contact: " << robot_state.cartesian_contact
          << ", joint_collision: " << robot_state.joint_collision
          << ", cartesian_collision: " << robot_state.cartesian_collision
          << ", tau_ext_hat_filtered: " << robot_state.tau_ext_hat_filtered
          << ", O_F_ext_hat_K: " << robot_state.O_F_ext_hat_K
          << ", K_F_ext_hat_K: " << robot_state.K_F_ext_hat_K
          << ", sequence_number: " << robot_state.sequence_number << "}";
  return ostream;
}

}  // namespace franka
