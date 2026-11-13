# Unitree Guide for ROS 2 Jazzy

This is a Go1 simulation port of [Unitree Robotics' Unitree Guide](https://github.com/unitreerobotics/unitree_guide/tree/fdf4d23de6affe8ee38fb4d892f61053fa1fcbcb). It uses Ubuntu 24.04, ROS 2 Jazzy, and Gazebo Harmonic. The controller is built as the `unitree_guide` library. The `go1_gazebo` package contains the simulation launcher, effort plugin, controller executable, and gait profiles. The BSD-3-Clause license applies to the Unitree Guide code.

## Build

For setup from a fresh GitHub clone, see [RUN_GO1.md](RUN_GO1.md).

With ROS 2 Jazzy installed, run these commands from `go1_sim/`.

```bash
sudo apt update
sudo apt install ros-dev-tools libeigen3-dev ros-jazzy-ros-gz \
  ros-jazzy-gz-ros2-control ros-jazzy-controller-interface ros-jazzy-realtime-tools \
  ros-jazzy-joint-state-broadcaster ros-jazzy-robot-state-publisher \
  ros-jazzy-tf2-ros

source /opt/ros/jazzy/setup.bash
colcon build --base-paths unitree_guide go1_gazebo --cmake-args -DCMAKE_BUILD_TYPE=Release
source install/setup.bash
```

The build command selects both ROS 2 packages in this directory.

## Fetch the Go1 model

The simulator uses the official Go1 URDF and meshes from [Unitree Robotics' unitree_ros](https://github.com/unitreerobotics/unitree_ros/tree/725b59b6de39a26c425632120c89f23c49a36f1e/robots/go1_description). A sparse checkout keeps the model separate from the ROS 2 controller.

```bash
git clone --filter=blob:none --sparse https://github.com/unitreerobotics/unitree_ros.git ~/unitree_ros_models
git -C ~/unitree_ros_models switch --detach 725b59b6de39a26c425632120c89f23c49a36f1e
git -C ~/unitree_ros_models sparse-checkout set robots/go1_description
```

## Run

From `go1_sim/` in terminal 1, launch Gazebo and spawn the Go1.

```bash
source /opt/ros/jazzy/setup.bash
source install/setup.bash
ros2 launch go1_gazebo gazeboSim.launch.py \
  go1_description:="$HOME/unitree_ros_models/robots/go1_description"
```

Set **gui:=false** to run the same simulation without opening the Gazebo window. Wait for the joint state broadcaster and effort controller to activate.

From `go1_sim/` in terminal 2, start the interactive controller.

```bash
source /opt/ros/jazzy/setup.bash
source install/setup.bash
ros2 run go1_gazebo junior_ctrl
```

In terminal 2, press **2** to stand. After the robot finishes standing, press **4**, then press **W** a few times to request forward movement. Each press increments the speed. **Space** centers the command, and **1** returns to passive mode. Press **5** from fixed stand to accept velocity commands on **/cmd_vel**. Restart both terminals after a Gazebo world reset.

## File map

| File | ROS 2 port |
|---|---|
| `unitree_guide/CMakeLists.txt` and `go1_gazebo/CMakeLists.txt` | Build the controller library, executable, and plugin with ament. Each package declares its dependencies in `package.xml`. |
| `go1_gazebo/src/main.cpp` and `unitree_guide/src/FSM/FSM.cpp` | Schedule the existing control loop on the simulation clock. |
| `go1_gazebo/src/interface/IOROS.cpp` | Read joint states and IMU and send Unitree's joint position, velocity, gains, and torque targets. |
| `unitree_guide/src/FSM/State_move_base.cpp` and `unitree_guide/src/control/Estimator.cpp` | Receive velocity commands and publish odometry and TF through ROS 2. |
| `unitree_guide/src/Gait/WaveGenerator.cpp` | Use simulation time for the existing contact and phase calculation. |
| `go1_gazebo/src/Go1JointController.cpp` | Apply the original Unitree servo law to the current simulated position and velocity on each physics step. |
| `go1_gazebo/launch/gazeboSim.launch.py` and `go1_gazebo/config/` | Start Gazebo Harmonic, configure the official Go1 model for effort control, and bridge the clock and IMU. |

## Four-beat crawl

Select the gait when starting the controller. Running `junior_ctrl` without a gait argument selects Unitree's stock trot.

```bash
ros2 run go1_gazebo junior_ctrl --ros-args -p gait:=walk
```

Press **2** to stand and wait for the pose to settle. Press **4** to enter walking mode, then tap **W** for forward motion or **S** for backward motion. **Space** centers the command. The walk profile caps the backward speed command at 0.10 m/s and limits forward and backward command changes to 0.30 m/s². To select a gait again, restart the controller with the desired gait argument.

The nominal schedule uses a 1.10 s period, 0.825 s of stance per leg, and 0.275 s of swing per leg. Touchdown is measured from the start of a stride.

| Leg | Touchdown phase | Lift-off phase | Swing interval |
|---|---:|---:|---:|
| Front right | 0.00 | 0.75 | 0.75–1.00 |
| Rear left | 0.25 | 0.00 | 0.00–0.25 |
| Front left | 0.50 | 0.25 | 0.25–0.50 |
| Rear right | 0.75 | 0.50 | 0.50–0.75 |

Unitree's cycloidal swing trajectory lifts the foot by 5 cm and uses Cartesian position and velocity feedback to track the target. During stance the controller anchors the foot target in the world frame and allocates ground reaction forces with the existing balance solver. At a nominal handoff one foot touches down as another lifts, so three legs are scheduled in stance. The force solver updates the active contacts at that handoff and penalizes changes from its previous force solution.

The contact schedule follows the 75% wave crawl in the [Springer Handbook of Robotics, Fig. 16.26](https://users.dimi.uniud.it/~antonio.dangelo/Robotica/2019/helper/Handbook-leggedRobot.pdf). `go1_gazebo/src/Gait/Walk.cpp` contains the crawl settings.
