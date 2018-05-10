/**
 * @brief avia link plugin
 * @file avia_link.cpp
 * @author birdman <jjbird@gmail.com>
 *
 * @addtogroup plugin
 * @{
 */

#include <mavros/mavros_plugin.h>
#include <boost/algorithm/string.hpp>

#include <std_msgs/String.h>
#include <robots_common/UInt8ArrayStamped.h>

namespace mavros {
namespace extra_plugins {

/**
 * @brief avia_link plugin
 *
 * Sends and receives avia data from the FCU
 * @see avia_link_cb()
 */
class AviaLinkPlugin : public plugin::PluginBase {
public:
	AviaLinkPlugin() : PluginBase(),
		avia_link_nh("~avia_link")
	{ }

	void initialize(UAS &uas_)
	{
		PluginBase::initialize(uas_);

		// subscribers
                avia_link_sub = avia_link_nh.subscribe(
                    "avia/to", 10, &AviaLinkPlugin::avia_link_cb, this);
                
                // publishers
                avia_link_pub = avia_link_nh.advertise<robots_common::UInt8ArrayStamped>("avia/from", 10);

	}

	Subscriptions get_subscriptions()
	{
		return { 
			    make_handler(&AviaLinkPlugin::handle_encapsulated_data),
                        };
	}

private:
	ros::NodeHandle avia_link_nh;
	ros::Subscriber avia_link_sub;
        ros::Publisher avia_link_pub;

	/**
	 * @brief avia msgs to the FCU.
	 *
	 * Message specification: https://mavlink.io/en/messages/common.html#ENCAPSULATED_DATA
	 * @param req	received UInt8ArrayStamped message with avia packet
	 */
	void avia_link_cb(const robots_common::UInt8ArrayStamped::ConstPtr &avia_packet)
	{
		mavlink::common::msg::ENCAPSULATED_DATA msg {};

                msg.seqnr = 0;
                for (uint8_t idx=0; idx<avia_packet->data.size(); idx++) {
                    msg.data[idx] = avia_packet->data[idx];
                }

		// send avia msg
		UAS_FCU(m_uas)->send_message_ignore_drop(msg);
	}

	void handle_encapsulated_data(const mavlink::mavlink_message_t *msg, mavlink::common::msg::ENCAPSULATED_DATA &avia_packet)
	{
                //std_msgs::String avia_msg;
                robots_common::UInt8ArrayStamped avia_msg;
                for (uint8_t idx=0; idx<253; idx++) {
                    avia_msg.data.push_back(avia_packet.data[idx]);
                } 
		//dv_msg->header.stamp = m_uas->synchronise_stamp(debug.time_boot_ms);

		avia_link_pub.publish(avia_msg);
	}
};
}	// namespace extra_plugins
}	// namespace mavros

#include <pluginlib/class_list_macros.h>
PLUGINLIB_EXPORT_CLASS(mavros::extra_plugins::AviaLinkPlugin, mavros::plugin::PluginBase)
