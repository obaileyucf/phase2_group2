#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <functional>

#include <controller_interface/controller_interface.hpp>
#include <controller_interface/helpers.hpp>
#include <pluginlib/class_list_macros.hpp>
#include <realtime_tools/realtime_buffer.hpp>
#include <std_msgs/msg/float64_multi_array.hpp>

namespace unitree_guide {

class Go1JointController : public controller_interface::ControllerInterface{
    using CallbackReturn = controller_interface::CallbackReturn;
    using ArrayMessage = std_msgs::msg::Float64MultiArray;
    static constexpr std::size_t jointCount = 12;
    static constexpr std::size_t fieldsPerJoint = 5;

    struct Command{
        std::array<double, jointCount * fieldsPerJoint> targets{};
        int64_t received = 0;
    };

public:
    CallbackReturn on_init() override{
        auto_declare<std::vector<std::string>>("joints", {});
        return CallbackReturn::SUCCESS;
    }

    controller_interface::InterfaceConfiguration command_interface_configuration() const override{
        controller_interface::InterfaceConfiguration config{
            controller_interface::interface_configuration_type::INDIVIDUAL, {}};
        for(const auto &joint : _joints){
            config.names.push_back(joint + "/effort");
        }
        return config;
    }

    controller_interface::InterfaceConfiguration state_interface_configuration() const override{
        controller_interface::InterfaceConfiguration config{
            controller_interface::interface_configuration_type::INDIVIDUAL, {}};
        for(const auto &joint : _joints){
            config.names.push_back(joint + "/position");
            config.names.push_back(joint + "/velocity");
        }
        return config;
    }

    CallbackReturn on_configure(const rclcpp_lifecycle::State &) override{
        const auto node = get_node();
        _joints = node->get_parameter("joints").as_string_array();
        if(_joints.size() != jointCount){
            RCLCPP_ERROR(node->get_logger(), "The Go1 servo requires 12 joints in Unitree motor order.");
            return CallbackReturn::ERROR;
        }
        _subscription = node->create_subscription<ArrayMessage>(
            "~/commands", 1, [this](const ArrayMessage &msg){
                Command command;
                if(msg.data.size() != command.targets.size() ||
                        !std::all_of(msg.data.begin(), msg.data.end(), [](double value){
                            return std::isfinite(value);
                        })){
                    return;
                }
                std::copy(msg.data.begin(), msg.data.end(), command.targets.begin());
                command.received = get_node()->now().nanoseconds();
                _command.writeFromNonRT(command);
            });
        return CallbackReturn::SUCCESS;
    }

    CallbackReturn on_activate(const rclcpp_lifecycle::State &) override{
        _effort.clear();
        _position.clear();
        _velocity.clear();
        _command.writeFromNonRT(Command{});
        const bool ordered = controller_interface::get_ordered_interfaces(
            command_interfaces_, _joints, "effort", _effort)
            && controller_interface::get_ordered_interfaces(
                state_interfaces_, _joints, "position", _position)
            && controller_interface::get_ordered_interfaces(
                state_interfaces_, _joints, "velocity", _velocity);
        return ordered ? CallbackReturn::SUCCESS : CallbackReturn::ERROR;
    }

    CallbackReturn on_deactivate(const rclcpp_lifecycle::State &) override{
        for(auto &joint : _effort){
            (void)joint.get().set_value(0.0);
        }
        return CallbackReturn::SUCCESS;
    }

    controller_interface::return_type update(
        const rclcpp::Time &time, const rclcpp::Duration &) override{
        const auto &command = *_command.readFromRT();
        constexpr int64_t timeout = 100000000; // 100 ms of simulation time
        const bool current = command.received > 0
            && std::abs(time.nanoseconds() - command.received) < timeout;
        // Go1 URDF effort limits in hip, thigh, calf order.
        constexpr std::array<double, 3> limits{23.7, 23.7, 35.55};

        for(std::size_t i = 0; i < jointCount; ++i){
            const auto q = _position[i].get().get_optional();
            const auto dq = _velocity[i].get().get_optional();
            double effort = 0.0;
            if(current && q && dq && std::isfinite(*q) && std::isfinite(*dq)){
                // Unitree's servo law evaluated on the current Gazebo physics step.
                const double *target = command.targets.data() + i * fieldsPerJoint;
                const double torque = target[4] + target[2] * (target[0] - *q)
                    + target[3] * (target[1] - *dq);
                const double limit = limits[i % limits.size()];
                if(std::isfinite(torque)){
                    effort = std::clamp(torque, -limit, limit);
                }
            }
            (void)_effort[i].get().set_value(effort);
        }
        return controller_interface::return_type::OK;
    }

private:
    std::vector<std::string> _joints;
    std::vector<std::reference_wrapper<hardware_interface::LoanedCommandInterface>> _effort;
    std::vector<std::reference_wrapper<hardware_interface::LoanedStateInterface>> _position, _velocity;
    realtime_tools::RealtimeBuffer<Command> _command{Command{}};
    rclcpp::Subscription<ArrayMessage>::SharedPtr _subscription;
};

}  // namespace unitree_guide

PLUGINLIB_EXPORT_CLASS(unitree_guide::Go1JointController, controller_interface::ControllerInterface)
