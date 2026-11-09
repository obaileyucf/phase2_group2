import os

import xacro
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import (
    AppendEnvironmentVariable,
    DeclareLaunchArgument,
    IncludeLaunchDescription,
    OpaqueFunction,
    RegisterEventHandler,
)
from launch.event_handlers import OnProcessExit
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node

# Controllers to spawn after joint_state_broadcaster (space separated, override
# with controllers:="..."). The names must match config/robot_control.yaml.
# joint_trajectory_controller = PD position hold using the effort interface.
# Use controllers:=effort_controller for raw torque commands instead.
# The ROS1 per-joint FL_hip_controller etc. only come back once
# unitree_legged_control is ported.
DEFAULT_CONTROLLERS = 'joint_trajectory_controller'

CONTROLLER_MANAGER = '/controller_manager'


def _flag(context, name):
    return LaunchConfiguration(name).perform(context).lower() == 'true'


def launch_setup(context):
    rname = LaunchConfiguration('rname').perform(context)
    wname = LaunchConfiguration('wname').perform(context)
    use_sim_time = _flag(context, 'use_sim_time')
    controllers = LaunchConfiguration('controllers').perform(context).split()

    desc_share = get_package_share_directory(f'{rname}_description')

    # World: prefer worlds/<wname>.sdf from unitree_gazebo, otherwise let Gazebo
    # resolve "<wname>.sdf" itself (e.g. the built-in empty.sdf).
    world_file = os.path.join(
        get_package_share_directory('unitree_gazebo'), 'worlds', f'{wname}.sdf')
    world = world_file if os.path.exists(world_file) else f'{wname}.sdf'

    # gazebo_ros empty_world.launch args -> gz sim command line flags
    flags = []
    if not _flag(context, 'paused'):
        flags.append('-r')  # run on start
    if _flag(context, 'headless') or not _flag(context, 'gui'):
        flags.append('-s')  # server only, no GUI
    if _flag(context, 'headless'):
        flags.append('--headless-rendering')
    if _flag(context, 'debug'):
        flags.append('-v 4')
    gz_args = ' '.join(flags + [world])

    # Replaces <param name="robot_description" command="xacro ...">
    # (--inorder no longer exists, and the "dollar" argument hack is not needed)
    robot_description = xacro.process_file(
        os.path.join(desc_share, 'xacro', 'robot.xacro'),
        mappings={'DEBUG': LaunchConfiguration('user_debug').perform(context)},
    ).toxml()

    # Lets Gazebo resolve package://<rname>_description/meshes/... URIs
    resource_path = AppendEnvironmentVariable(
        'GZ_SIM_RESOURCE_PATH', os.path.dirname(desc_share))

    gz_sim = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(os.path.join(
            get_package_share_directory('ros_gz_sim'), 'launch', 'gz_sim.launch.py')),
        launch_arguments={'gz_args': gz_args}.items(),
    )

    # /clock is needed because use_sim_time is true. The foot sensors replace
    # Unitree's foot contact plugin: /<leg>_foot_contact lists what the foot is
    # touching (empty = no contact), /<leg>_foot_force is the foot load.
    # '[' means Gazebo -> ROS only. If WrenchStamped is rejected by your
    # ros_gz_bridge version, use geometry_msgs/msg/Wrench.
    bridge_args = ['/clock@rosgraph_msgs/msg/Clock[gz.msgs.Clock']
    for leg in ('FL', 'FR', 'RL', 'RR'):
        bridge_args.append(
            f'/{leg}_foot_contact@ros_gz_interfaces/msg/Contacts[gz.msgs.Contacts')
        bridge_args.append(
            f'/{leg}_foot_force@geometry_msgs/msg/WrenchStamped[gz.msgs.Wrench')

    clock_bridge = Node(
        package='ros_gz_bridge',
        executable='parameter_bridge',
        arguments=bridge_args,
        output='screen',
    )

    # Publishes TF and the robot_description topic that the spawner reads
    robot_state_publisher = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        output='screen',
        parameters=[{
            'robot_description': robot_description,
            'use_sim_time': use_sim_time,
        }],
    )

    # Replaces gazebo_ros spawn_model
    spawn = Node(
        package='ros_gz_sim',
        executable='create',
        output='screen',
        arguments=[
            '-topic', 'robot_description',
            '-name', f'{rname}_gazebo',
            '-z', LaunchConfiguration('z').perform(context),
        ],
    )

    # joint_state_controller -> joint_state_broadcaster
    joint_state_broadcaster = Node(
        package='controller_manager',
        executable='spawner',
        output='screen',
        arguments=['joint_state_broadcaster',
                   '-c', CONTROLLER_MANAGER,
                   '--controller-manager-timeout', '60'],
    )

    joint_controllers = Node(
        package='controller_manager',
        executable='spawner',
        output='screen',
        arguments=controllers + ['-c', CONTROLLER_MANAGER,
                                 '--controller-manager-timeout', '60'],
    )

    return [
        resource_path,
        gz_sim,
        clock_bridge,
        robot_state_publisher,
        spawn,
        # Load controllers only after the robot has been spawned
        RegisterEventHandler(OnProcessExit(
            target_action=spawn, on_exit=[joint_state_broadcaster])),
        RegisterEventHandler(OnProcessExit(
            target_action=joint_state_broadcaster, on_exit=[joint_controllers])),
        # ROS1 also included unitree_controller/launch/set_ctrl.launch here.
        # Port it separately and pass its values as node parameters.
    ]


def generate_launch_description():
    return LaunchDescription([
        DeclareLaunchArgument('wname', default_value='empty'),
        DeclareLaunchArgument('rname', default_value='go1'),
        # Default is false (was true in ROS1): the controller manager only
        # activates controllers while the simulation is stepping, and ROS2 has
        # no equivalent of spawn_model's -unpause.
        DeclareLaunchArgument('paused', default_value='false'),
        DeclareLaunchArgument('use_sim_time', default_value='true'),
        DeclareLaunchArgument('gui', default_value='true'),
        DeclareLaunchArgument('headless', default_value='false'),
        DeclareLaunchArgument('debug', default_value='false'),
        # Debug mode will hang up the robot, use "true" or "false" to switch it.
        DeclareLaunchArgument('user_debug', default_value='false'),
        DeclareLaunchArgument('z', default_value='0.35'),
        DeclareLaunchArgument('controllers', default_value=DEFAULT_CONTROLLERS),
        OpaqueFunction(function=launch_setup),
    ])
