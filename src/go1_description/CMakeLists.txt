
cmake_minimum_required(VERSION 3.8)
project(go1_description)

find_package(ament_cmake REQUIRED)

# Install robot description resources
install(
  DIRECTORY
    urdf
    xacro
    meshes
    launch
    config
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
