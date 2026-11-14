
cmake_minimum_required(VERSION 3.14)
project(go1_gazebo)

find_package(ament_cmake REQUIRED)
find_package(controller_interface REQUIRED)
find_package(pluginlib REQUIRED)
find_package(rclcpp REQUIRED)
find_package(realtime_tools REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(std_msgs REQUIRED)
find_package(unitree_guide REQUIRED)

set(UNITREE_GUIDE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/../unitree_guide"
)

# Main Unitree controller
add_executable(junior_ctrl
    src/main.cpp

    # Gait profiles
    src/Gait/Trot.cpp
    src/Gait/Walk.cpp
    src/Gait/Canter.cpp
    src/Gait/Gallop.cpp

    # ROS interface
    src/interface/IOROS.cpp
    src/interface/KeyBoard.cpp
)

target_compile_features(junior_ctrl PRIVATE cxx_std_17)

target_include_directories(junior_ctrl PRIVATE
    include
    "${UNITREE_GUIDE_DIR}/include"
)

# Link Unitree Guide
target_link_libraries(junior_ctrl
    unitree_guide::unitree_guide
)

ament_target_dependencies(junior_ctrl
    rclcpp
    sensor_msgs
    std_msgs
)

# Go1 joint controller plugin
add_library(go1_joint_controller SHARED
    src/Go1JointController.cpp
)

target_compile_features(go1_joint_controller PRIVATE cxx_std_17)

ament_target_dependencies(go1_joint_controller
    controller_interface
    pluginlib
    rclcpp
    realtime_tools
    std_msgs
)

pluginlib_export_plugin_description_file(
    controller_interface
    go1_joint_controller.xml
)

# Installation
install(TARGETS go1_joint_controller
    LIBRARY DESTINATION lib
)

install(TARGETS junior_ctrl
    DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY include/
    DESTINATION include
)

install(DIRECTORY config launch
    DESTINATION share/${PROJECT_NAME}
)

ament_package()
