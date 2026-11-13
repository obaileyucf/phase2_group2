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

add_executable(junior_ctrl
    src/main.cpp
    src/Gait/Trot.cpp
    src/Gait/Walk.cpp
    src/interface/IOROS.cpp
    src/interface/KeyBoard.cpp)
target_include_directories(junior_ctrl PRIVATE include)
target_link_libraries(junior_ctrl PRIVATE unitree_guide::unitree_guide)
ament_target_dependencies(junior_ctrl PUBLIC rclcpp sensor_msgs std_msgs)

add_library(go1_joint_controller SHARED src/Go1JointController.cpp)
target_compile_features(go1_joint_controller PRIVATE cxx_std_17)
ament_target_dependencies(go1_joint_controller
    controller_interface pluginlib rclcpp realtime_tools std_msgs)
pluginlib_export_plugin_description_file(controller_interface go1_joint_controller.xml)

install(TARGETS go1_joint_controller LIBRARY DESTINATION lib)
install(TARGETS junior_ctrl DESTINATION lib/${PROJECT_NAME})
install(DIRECTORY config launch DESTINATION share/${PROJECT_NAME})
ament_package()
