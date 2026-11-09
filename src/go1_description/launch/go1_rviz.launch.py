from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import (
    Command,
    FindExecutable,
    LaunchConfiguration,
    PathJoinSubstitution,
)
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue
from launch_ros.substitutions import FindPackageShare


def generate_launch_description():
    pkg_share = FindPackageShare('go1_description')
    user_debug = LaunchConfiguration('user_debug')

    # `--inorder` is gone in ROS2 xacro (it is the default behaviour).
    # ParameterValue(..., value_type=str) stops the URDF string from being
    # parsed as YAML when it is passed as a parameter.
    robot_description = ParameterValue(
        Command([
            FindExecutable(name='xacro'), ' ',
            PathJoinSubstitution([pkg_share, 'xacro', 'robot.xacro']),
            ' DEBUG:=', user_debug,
        ]),
        value_type=str,
    )

    rviz_config = PathJoinSubstitution([pkg_share, 'launch', 'check_joint.rviz'])

    return LaunchDescription([
        DeclareLaunchArgument('user_debug', default_value='false'),

        Node(
            package='robot_state_publisher',
            executable='robot_state_publisher',
            name='robot_state_publisher',
            output='screen',
            parameters=[{
                'robot_description': robot_description,
                'publish_frequency': 50.0,  # 1000 Hz was overkill; raise if needed
            }],
        ),

        # use_gui param no longer exists; the GUI is its own package/executable.
        Node(
            package='joint_state_publisher_gui',
            executable='joint_state_publisher_gui',
            name='joint_state_publisher_gui',
        ),

        Node(
            package='rviz2',
            executable='rviz2',
            name='rviz2',
            output='screen',
            arguments=['-d', rviz_config],
        ),
    ])
