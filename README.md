# Go1 Simulation Framework — ROS 2 Jazzy

This project is a **ROS 2 port and adaptation of [Unitree Robotics' Unitree Guide](https://github.com/unitreerobotics/unitree_guide)** for simulating the **Unitree Go1** on **Ubuntu 24.04, ROS 2 Jazzy, and Gazebo Harmonic**.

The original Unitree Guide supplies the underlying locomotion architecture, including its finite-state machine, gait generation, state estimation, kinematics, and balance/force control. This port preserves that foundation while replacing ROS 1/Gazebo Classic integration with ROS 2 communication and Gazebo Harmonic simulation interfaces.

This is an independently adapted simulation project, **not an official Unitree ROS 2 release** or a validated controller for a physical Go1. Credit for the original framework belongs to Unitree Robotics. The included Unitree code retains its **BSD-3-Clause** license; third-party components retain their own notices (including QuadProg++'s MIT notice). See [`LICENSE`](LICENSE) and the source-file headers.

## File map

The table highlights **port-specific additions and adapted Unitree Guide files**. Paths are relative to the repository root.

| File(s) | Role in the ROS 2 port |
| --- | --- |
| `unitree_guide/CMakeLists.txt`, `unitree_guide/package.xml` | Build and export the original locomotion algorithms as an `ament_cmake` library with ROS 2 dependencies. |
| `go1_gazebo/CMakeLists.txt`, `go1_gazebo/package.xml` | Build the `junior_ctrl` executable and Gazebo joint-control plugin; register ROS 2 package dependencies. |
| `go1_gazebo/src/main.cpp`, `unitree_guide/src/FSM/FSM.cpp` | Run the existing finite-state-machine/control loop using simulation time, with a nominal **2 ms (500 Hz)** controller period. |
| `go1_gazebo/src/interface/IOROS.cpp` | Bridge ROS 2 `/joint_states` and `/trunk_imu` into Unitree's low-level state format; publish joint position, velocity, gain, and feedforward-torque targets. |
| `unitree_guide/src/FSM/State_move_base.cpp`, `unitree_guide/src/control/Estimator.cpp` | Adapt velocity-command handling (`/cmd_vel`), state estimation, odometry, and TF publishing to ROS 2. |
| `unitree_guide/src/FSM/State_Trotting.cpp` | Reuse Unitree Guide's locomotion state for the selected gait; apply swing/stance control, balance forces, and gait-specific command limits. |
| `unitree_guide/src/Gait/WaveGenerator.cpp` | Compute each leg's contact state and stance/swing phase from the Gazebo simulation clock. |
| `go1_gazebo/src/Gait/Walk.cpp`, `go1_gazebo/src/Gait/Trot.cpp` | Define the walk and trot periods, duty factors, leg-phase offsets, and swing heights. |
| `go1_gazebo/src/Go1JointController.cpp` | Implement the ROS 2 `ros2_control` plugin: apply Unitree-style PD + feedforward torque commands against live joint states, with torque limits and a command timeout. |
| `go1_gazebo/launch/gazeboSim.launch.py`, `go1_gazebo/config/` | Load the official Go1 model into Gazebo Harmonic, set up effort-control interfaces, spawn controllers, and bridge simulation clock and IMU messages. |
| `unitree_guide/src/quadProgpp/QuadProg++.cc` | Keep the original quadratic-programming solver usable with a modern C++ toolchain (including removal of obsolete `register` keywords). |

## Gait profiles
### Walk
The ** walk** staggers the four legs by a quarter-cycle. Three legs are nominally in stance while the fourth swings; at each phase boundary, one leg finishes its swing as another begins. The touchdown order is **FR → RL → FL → RR**.

1.10 s period, 75% duty cycle (0.825 s stance, 0.275 s swing), 0.05 m foot lift, and touchdown offsets [0.00, 0.50, 0.75, 0.25] for [FR, FL, RR, RL].

| Leg | Touchdown phase | Lift-off phase | Swing interval |
| --- | ---: | ---: | --- |
| Front right (FR) | 0.00 | 0.75 | 0.75–1.00 |
| Rear left (RL) | 0.25 | 0.00 | 0.00–0.25 |
| Front left (FL) | 0.50 | 0.25 | 0.25–0.50 |
| Rear right (RR) | 0.75 | 0.50 | 0.50–0.75 |

### Trot timing

The **trot** alternates diagonal leg pairs: **FR + RL** move together, then **FL + RR**. Each pair spends half a cycle in stance and half in swing. With the current profile, each half-cycle lasts **0.225 s**.

Trot: 0.45 s period, 50% duty cycle (0.225 s stance, 0.225 s swing), 0.08 m foot lift, and touchdown offsets [0.00, 0.50, 0.50, 0.00] for [FR, FL, RR, RL].

| Leg pair | Touchdown phase | Swing interval |
| --- | ---: | --- |
| FR + RL | 0.00 | 0.50–1.00 |
| FL + RR | 0.50 | 0.00–0.50 |

## Build and run

For first-time installation and model setup, follow **[RUN_GO1.md](RUN_GO1.md)**. Once the environment and official Go1 description are installed, build from the repository root:

```bash
source /opt/ros/jazzy/setup.bash
colcon build --base-paths unitree_guide go1_gazebo --cmake-args -DCMAKE_BUILD_TYPE=Release
source install/setup.bash
```

Launch Gazebo:

```bash
ros2 launch go1_gazebo gazeboSim.launch.py \
  go1_description:="$HOME/unitree_ros_models/robots/go1_description"
```

In a **second terminal**, source the workspace and choose a gait:

```bash
source /opt/ros/jazzy/setup.bash
source install/setup.bash
ros2 run go1_gazebo junior_ctrl --ros-args -p gait:=walk
# Or: -p gait:=trot
```

Press **2** to stand, **4** to enter locomotion, **W/S** to change forward/backward speed, and **Space** to stop the motion command. The default gait, if no parameter is supplied, is **trot**.

## Upstream references

- [Unitree Robotics — Unitree Guide](https://github.com/unitreerobotics/unitree_guide): original controller and algorithms.
- [Unitree Robotics — unitree_ros](https://github.com/unitreerobotics/unitree_ros): official Go1 robot description and meshes, obtained separately for simulation.
