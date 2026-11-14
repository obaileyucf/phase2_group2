cmake_minimum_required(VERSION 3.14)
project(unitree_guide)

find_package(ament_cmake REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(rclcpp REQUIRED)
find_package(tf2_ros REQUIRED)

file(GLOB_RECURSE GUIDE_SOURCES CONFIGURE_DEPENDS "src/*/*.cpp" "src/*/*.cc")

add_library(unitree_guide STATIC ${GUIDE_SOURCES})
target_compile_features(unitree_guide PUBLIC cxx_std_17)
target_compile_definitions(unitree_guide PUBLIC
    ROBOT_TYPE_Go1 COMPILE_WITH_SIMULATION COMPILE_WITH_MOVE_BASE)
target_include_directories(unitree_guide PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
    "$<INSTALL_INTERFACE:include/${PROJECT_NAME}>")
target_link_libraries(unitree_guide PUBLIC Eigen3::Eigen)
ament_target_dependencies(unitree_guide PUBLIC geometry_msgs nav_msgs rclcpp tf2_ros)

install(DIRECTORY include/ DESTINATION include/${PROJECT_NAME}/src)
install(TARGETS unitree_guide EXPORT export_unitree_guide ARCHIVE DESTINATION lib)
ament_export_targets(export_unitree_guide)
ament_export_dependencies(Eigen3 geometry_msgs nav_msgs rclcpp tf2_ros)
ament_package()
