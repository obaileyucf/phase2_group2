cmake_minimum_required(VERSION 3.8)
project(unitree_gazebo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ament_cmake REQUIRED)

# ---------------------------------------------------------------------------
# The ROS1 package only built two Gazebo *Classic* plugins:
#   unitreeFootContactPlugin  (plugin/foot_contact_plugin.cc)
#   unitreeDrawForcePlugin    (plugin/draw_force_plugin.cc)
# They use the Gazebo Classic C++ API and cannot be built against Gazebo
# Harmonic (Jazzy) just by changing this file, so they are not built here.
# Suggested replacements:
#   foot contact -> gz-sim contact sensor in the URDF/SDF + ros_gz_bridge
#   draw force   -> RViz markers published from the bridged contact data
#
# If you later write a gz-sim system plugin, this is the shape of it:
#
# find_package(gz-sim8 REQUIRED)          # Gazebo Harmonic = gz-sim8
# add_library(UnitreeFootContact SHARED plugin/foot_contact_system.cc)
# target_link_libraries(UnitreeFootContact PRIVATE gz-sim8::gz-sim8)
# install(TARGETS UnitreeFootContact DESTINATION lib/${PROJECT_NAME})
# ---------------------------------------------------------------------------

# ROS1 found launch files in the source tree. In ROS2 they must be installed,
# or `ros2 launch unitree_gazebo ...` will not see them.
foreach(dir launch worlds config rviz models)
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${dir})
    install(DIRECTORY ${dir} DESTINATION share/${PROJECT_NAME})
  endif()
endforeach()

ament_package()
