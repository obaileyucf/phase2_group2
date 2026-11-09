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

ARM_CONTROLLERS = [f'Joint{i:02d}_controller' for i in range(1, 7)]

CONTROLLER_MANAGER = '/controller_manager'


def _flag(context, name):
    return LaunchConfiguration(name).perform(context).lower() == 'true'


def launch_setup(context):
    rname = LaunchConfiguration('rname').perform(context)
    wname = LaunchConfiguration('wname').perform(context)
    use_sim_time = _flag(context, 'use_sim_time')
    gripper = _flag(context, 'UnitreeGripperYN')

    desc_share = get_package_share_directory(f'{rname}_description')

    world_file = os.path.join(
        get_package_share_directory('unitree_gazebo'), 'worlds', f'{wname}.sdf')
    world = world_file if os.path.exists(world_file) else f'{wname}.sdf'

    flags = []
    if not _flag(context, 'paused'):
        flags.append('-r')
    if _flag(context, 'headless') or not _flag(context, 'gui'):
        flags.append('-s')
    if _flag(context, 'headless'):
        flags.append('--headless-rendering')
    if _flag(context, 'debug'):
        flags.append('-v 4')
    gz_args = ' '.join(flags + [world])

    robot_description = xacro.process_file(
        os.path.join(desc_share, 'xacro', 'robot.xacro'),
        mappings={'UnitreeGripper': 'true' if gripper else 'false'},
    ).toxml()

    resource_path = AppendEnvironmentVariable(
        'GZ_SIM_RESOURCE_PATH', os.path.dirname(desc_share))

    gz_sim = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(os.path.join(
            get_package_share_directory('ros_gz_sim'), 'launch', 'gz_sim.launch.py')),
        launch_arguments={'gz_args': gz_args}.items(),
    )

    clock_bridge = Node(
        package='ros_gz_bridge',
        executable='parameter_bridge',
        arguments=['/clock@rosgraph_msgs/msg/Clock[gz.msgs.Clock'],
        output='screen',
    )

    robot_state_publisher = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        output='screen',
        parameters=[{
            'robot_description': robot_description,
            'use_sim_time': use_sim_time,
        }],
    )

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

    joint_state_broadcaster = Node(
        package='controller_manager',
        executable='spawner',
        output='screen',
        arguments=['joint_state_broadcaster',
                   '-c', CONTROLLER_MANAGER,
                   '--controller-manager-timeout', '60'],
    )

    # Replaces the <group if/unless="UnitreeGripperYN"> pair
    controllers = ARM_CONTROLLERS + (['gripper_controller'] if gripper else [])
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
        RegisterEventHandler(OnProcessExit(
            target_action=spawn, on_exit=[joint_state_broadcaster])),
        RegisterEventHandler(OnProcessExit(
            target_action=joint_state_broadcaster, on_exit=[joint_controllers])),
    ]


def generate_launch_description():
    return LaunchDescription([
        DeclareLaunchArgument('wname', default_value='empty'),
        DeclareLaunchArgument('rname', default_value='z1'),
        # Default is false (was true in ROS1), see normal.launch.py
        DeclareLaunchArgument('paused', default_value='false'),
        DeclareLaunchArgument('use_sim_time', default_value='true'),
        DeclareLaunchArgument('gui', default_value='true'),
        DeclareLaunchArgument('headless', default_value='false'),
        DeclareLaunchArgument('debug', default_value='false'),
        DeclareLaunchArgument('UnitreeGripperYN', default_value='true'),
        DeclareLaunchArgument('z', default_value='0.0'),
        OpaqueFunction(function=launch_setup),
    ])
