#ifndef SUB_TIMING_ADVERTISE_NODE_HPP_
#define SUB_TIMING_ADVERTISE_NODE_HPP_

#include <set>
#include "rclcpp/node.hpp"
#include "rclcpp/visibility_control.hpp"
#include "rclcpp/node_interfaces/get_node_topics_interface.hpp"

#include "path_info_msg/msg/topic_info.hpp"

namespace pathnode
{
/// PoC of `every sub talks sub timing`
class SubTimingAdvertiseNode : public rclcpp::Node
{
  using TopicInfoPublisher = rclcpp::Publisher<path_info_msg::msg::TopicInfo>::SharedPtr;

public:
  RCLCPP_PUBLIC
  explicit SubTimingAdvertiseNode(
    const std::string & node_name,
    const rclcpp::NodeOptions & options = rclcpp::NodeOptions());

  RCLCPP_PUBLIC
  explicit SubTimingAdvertiseNode(
    const std::string & node_name,
    const std::string & namespace_,
    const rclcpp::NodeOptions & options = rclcpp::NodeOptions());

  RCLCPP_PUBLIC
  virtual ~SubTimingAdvertiseNode();

    /// create custom subscription
  /**
   * This is the implementation of `first node only send path_info` strategy.
   */
  template<
    typename MessageT,
    typename CallbackT,
    typename AllocatorT = std::allocator<void>,
    typename CallbackMessageT =
    typename rclcpp::subscription_traits::has_message_type<CallbackT>::type,
    typename CallbackArgT =
    typename rclcpp::function_traits::function_traits<CallbackT>::template argument_type<0>,
    typename SubscriptionT = rclcpp::Subscription<CallbackMessageT, AllocatorT>,
    typename MessageMemoryStrategyT = rclcpp::message_memory_strategy::MessageMemoryStrategy<
      CallbackMessageT,
      AllocatorT
    >
  >
  std::shared_ptr<SubscriptionT>
  create_timing_advertise_subscription(
    const std::string & topic_name,
    const rclcpp::QoS & qos,
    CallbackT && callback,
    const rclcpp::SubscriptionOptionsWithAllocator<AllocatorT> & options =
    rclcpp::SubscriptionOptionsWithAllocator<AllocatorT>(),
    typename MessageMemoryStrategyT::SharedPtr msg_mem_strat = (
      MessageMemoryStrategyT::create_default()
    ))
  {
    using rclcpp::node_interfaces::get_node_topics_interface;
    auto node_topics_interface = get_node_topics_interface(this);
    auto resolved_topic_name = node_topics_interface->resolve_topic_name(topic_name);

    auto topic_info_name = resolved_topic_name + "_info";
    auto topic_info_pub = create_publisher<path_info_msg::msg::TopicInfo>(
        topic_info_name,
        rclcpp::QoS(1));
    topic_info_pubs_[topic_info_name] = topic_info_pub;

    auto main_topic_callback
        = [this, resolved_topic_name, topic_info_name, callback](CallbackArgT msg) -> void
          {
            auto m = std::make_unique<path_info_msg::msg::TopicInfo>();
            m->node_fqn = get_fully_qualified_name();
            m->topic_name = resolved_topic_name;
            m->callback_start = now();
            topic_info_pubs_[topic_info_name]->publish(std::move(m));

            // finally, call original function
            callback(std::forward<CallbackArgT>(msg));
          };

    return create_subscription<MessageT>(
        topic_name,
        qos,
        main_topic_callback,
        options,
        msg_mem_strat);
  }

private:
  /// topic name vs TopicInfoPublisher
  std::map<std::string, TopicInfoPublisher> topic_info_pubs_;
  rclcpp::Time now() const;
  rcl_clock_type_t CLOCK_TYPE;
};

} // namespace pathnode

#endif // SUB_TIMING_ADVERTISE_NODE_HPP_
