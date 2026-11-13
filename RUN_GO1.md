# Run the Go1 simulation from a clone

The `unitree_guide` package builds the shared locomotion controller. The `go1_gazebo` package links against it and provides the launcher, simulated joint effort controller, and walk and trot profiles.

## Install dependencies

Use Ubuntu 24.04 with ROS 2 Jazzy. On a fresh installation, follow the ROS guide to add the ROS apt repository and install Jazzy. `ros-jazzy-ros-gz` provides the Gazebo Harmonic integration for Jazzy. [ROS 2 Jazzy installation](https://docs.ros.org/en/jazzy/Installation/Ubuntu-Install-Debs.html) · [Gazebo with ROS](https://gazebosim.org/docs/harmonic/ros_installation/)

```bash
sudo apt update
sudo apt install -y git ros-dev-tools libeigen3-dev ros-jazzy-ros-gz \
  ros-jazzy-gz-ros2-control ros-jazzy-controller-interface ros-jazzy-realtime-tools \
  ros-jazzy-joint-state-broadcaster ros-jazzy-robot-state-publisher ros-jazzy-tf2-ros
```

## Clone and build

Commit the `go1_sim/` folder from the archive inside the group repository. Replace the example URL and branch name with your own. If you use the archive contents as the repository root, omit `/go1_sim` from the project path in all three commands below.

```bash
git clone --branch go1-jazzy-walk-trot \
  https://github.com/YOUR-ORG/YOUR-REPO.git "$HOME/group-project"

git clone --filter=blob:none --sparse \
  https://github.com/unitreerobotics/unitree_ros.git "$HOME/unitree_ros_models"
git -C "$HOME/unitree_ros_models" switch --detach 725b59b6de39a26c425632120c89f23c49a36f1e
git -C "$HOME/unitree_ros_models" sparse-checkout set robots/go1_description

cd "$HOME/group-project/go1_sim"
source /opt/ros/jazzy/setup.bash
colcon build --base-paths unitree_guide go1_gazebo --cmake-args -DCMAKE_BUILD_TYPE=Release
```

If you already have the model at the pinned revision, reuse that checkout and pass its `robots/go1_description` path below. If you are replacing an earlier one-package layout in the same checkout, start from a fresh clone or build with a fresh `build/`, `install/`, and `log/` directory.

## Terminal 1 — Start Gazebo

```bash
cd "$HOME/group-project/go1_sim"
source /opt/ros/jazzy/setup.bash
source install/setup.bash
ros2 launch go1_gazebo gazeboSim.launch.py \
  go1_description:="$HOME/unitree_ros_models/robots/go1_description"
```

Wait for `joint_state_broadcaster` and `go1_effort_controller` to activate. Append `gui:=false` to the launch command to run headless.

## Terminal 2 — Start the gait controller

```bash
cd "$HOME/group-project/go1_sim"
source /opt/ros/jazzy/setup.bash
source install/setup.bash
ros2 run go1_gazebo junior_ctrl --ros-args -p gait:=walk
```

Press **2** to stand, then **4** after the robot settles. Use **W** and **S** to move and **Space** to center the velocity command. Start `junior_ctrl` with `gait:=trot` to use the trot.
