// Copyright 2021 Research Institute of Systems Planning, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <chrono>
#include <cstdio>
#include <memory>
#include <string>
#include <utility>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_components/register_node_macro.hpp"

#include "sensor_msgs/msg/point_cloud2.hpp"

#include "tilde/tilde_node.hpp"
#include "tilde/tilde_publisher.hpp"

using namespace std::chrono_literals;

namespace tilde_sample
{
// Create a Talker class that subclasses the generic rclcpp::Node base class.
// The main function below will instantiate the class as a ROS node.
class P2Publisher : public tilde::TildeNode
{
public:
  explicit P2Publisher(const rclcpp::NodeOptions & options)
  : TildeNode("talker", options)
  {
    const std::string TIMER_MS = "timer_ms";

    declare_parameter<int64_t>(TIMER_MS, (int64_t) 10);
    auto timer_ms = get_parameter(TIMER_MS).get_value<int64_t>();
    std::cout << "timer_ms: " << timer_ms << std::endl;

    // Create a function for when messages are to be sent.
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);
    auto publish_message =
      [this]() -> void
      {
        RCLCPP_INFO(this->get_logger(), "Publishing: '%ld'", count_);
        count_++;

        msg_pc_ = std::make_unique<sensor_msgs::msg::PointCloud2>();
        msg_pc_->header.stamp = this->now();
        pub_pc_->publish(std::move(msg_pc_));
      };
    // Create a publisher with a custom Quality of Service profile.
    rclcpp::QoS qos(rclcpp::KeepLast(7));
    pub_pc_ = this->create_tilde_publisher<sensor_msgs::msg::PointCloud2>("out", qos);

    // Use a timer to schedule periodic message publishing.
    auto dur = std::chrono::duration<int64_t, std::milli>(timer_ms);
    timer_ = this->create_wall_timer(dur, publish_message);
  }

private:
  size_t count_ = 1;
  std::unique_ptr<sensor_msgs::msg::PointCloud2> msg_pc_;
  tilde::TildePublisher<sensor_msgs::msg::PointCloud2>::SharedPtr pub_pc_;
  rclcpp::TimerBase::SharedPtr timer_;
};

}  // namespace tilde_sample

RCLCPP_COMPONENTS_REGISTER_NODE(tilde_sample::P2Publisher)