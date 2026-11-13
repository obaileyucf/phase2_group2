# Run the Go1 simulation from a clone

The `unitree_guide` package builds the shared locomotion controller. The `go1_gazebo` package links against it and provides the launcher, simulated joint effort controller, and walk and trot profiles.

## Install dependencies
Ubuntu 24.04 Install via WSL
```bash
wsl --install -d Ubuntu-24.04
```

Prepare Ubuntu and add the ROS 2 package repository
```bash
cd ~
lsb_release -ds

sudo apt update
sudo apt upgrade -y
sudo apt install -y git curl locales software-properties-common
sudo locale-gen en_US.UTF-8
sudo update-locale LANG=en_US.UTF-8
export LANG=en_US.UTF-8

sudo add-apt-repository -y universe
```
```bash
ros_apt_version="$(curl -fsSL \
  https://api.github.com/repos/ros-infrastructure/ros-apt-source/releases/latest \
  | python3 -c 'import json, sys; print(json.load(sys.stdin)["tag_name"])')"

curl -fsSL -o /tmp/ros2-apt-source.deb \
  "https://github.com/ros-infrastructure/ros-apt-source/releases/download/${ros_apt_version}/ros2-apt-source_${ros_apt_version}.noble_all.deb" &&
sudo apt install -y /tmp/ros2-apt-source.deb

sudo apt update
```
Install ROS 2 Jazzy, Gazebo Harmonic, and Build Tools
```bash
sudo apt install -y \
  ros-jazzy-ros-base ros-dev-tools libeigen3-dev \
  ros-jazzy-ros-gz ros-jazzy-gz-ros2-control \
  ros-jazzy-controller-interface ros-jazzy-realtime-tools \
  ros-jazzy-joint-state-broadcaster ros-jazzy-robot-state-publisher \
  ros-jazzy-tf2-ros

source /opt/ros/jazzy/setup.bash

echo "$ROS_DISTRO"
gz sim --version
```
Clone Repo and Download Go1 Model
```bash
git clone https://github.com/YOUR-ORG/YOUR-REPO.git "$HOME/go1_sim"

git clone --filter=blob:none --sparse \
  https://github.com/unitreerobotics/unitree_ros.git "$HOME/unitree_ros_models"

git -C "$HOME/unitree_ros_models" switch --detach \
  725b59b6de39a26c425632120c89f23c49a36f1e

git -C "$HOME/unitree_ros_models" sparse-checkout set robots/go1_description
```
Build Packages
```bash
cd "$HOME/go1_sim"
source /opt/ros/jazzy/setup.bash

colcon build --base-paths unitree_guide go1_gazebo \
  --cmake-args -DCMAKE_BUILD_TYPE=Release
```

## Terminal 1 — Start Gazebo
```bash
cd "$HOME/go1_sim"
source /opt/ros/jazzy/setup.bash
source install/setup.bash

ros2 launch go1_gazebo gazeboSim.launch.py \
  go1_description:="$HOME/unitree_ros_models/robots/go1_description"
```

## Terminal 2 — Start the gait controller

```bash
cd "$HOME/go1_sim"
source /opt/ros/jazzy/setup.bash
source install/setup.bash

ros2 run go1_gazebo junior_ctrl --ros-args -p gait:=walk
```

Press **2** to stand, then **4** after the robot settles. Use **W** and **S** to move and **Space** to center the velocity command. Use ```gait:=trot``` for trotting.
