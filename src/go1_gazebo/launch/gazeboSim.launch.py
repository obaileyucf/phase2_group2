# ROS 2 / Gazebo Harmonic counterpart of Unitree Guide's gazeboSim.launch.
from pathlib import Path
from shlex import quote
from xml.etree import ElementTree as ET

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import (
    DeclareLaunchArgument, EmitEvent, IncludeLaunchDescription,
    OpaqueFunction, RegisterEventHandler,
)
from launch.event_handlers import OnProcessExit
from launch.events import Shutdown
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue


def go1_description(model_directory, controllers):
    robot = ET.parse(model_directory / "urdf" / "go1.urdf").getroot()
    for extension in robot.findall("gazebo"):
        for child in list(extension):
            if child.tag not in {"mu1", "mu2", "self_collide"}:
                extension.remove(child)
        if not len(extension):
            robot.remove(extension)

    for mesh in robot.iter("mesh"):
        filename = mesh.get("filename")
        if filename.startswith("package://go1_description/"):
            mesh_file = model_directory / filename.removeprefix("package://go1_description/")
            if not mesh_file.is_file():
                raise FileNotFoundError(mesh_file)
            mesh.set("filename", str(mesh_file))

    hardware = ET.SubElement(robot, "ros2_control", name="Go1System", type="system")
    ET.SubElement(ET.SubElement(hardware, "hardware"), "plugin").text = (
        "gz_ros2_control/GazeboSimSystem"
    )
    initial_positions = {"hip": "0.0", "thigh": "1.05", "calf": "-2.10"}
    for leg in ("FR", "FL", "RR", "RL"):
        for role, initial_position in initial_positions.items():
            name = f"{leg}_{role}_joint"
            source = robot.find(f"./joint[@name='{name}']")
            if source is None:
                raise ValueError(f"Go1 model is missing {name}")
            joint = ET.SubElement(hardware, "joint", name=name)
            effort = ET.SubElement(joint, "command_interface", name="effort")
            limit = source.find("limit").get("effort")
            ET.SubElement(effort, "param", name="min").text = f"-{limit}"
            ET.SubElement(effort, "param", name="max").text = limit
            position = ET.SubElement(joint, "state_interface", name="position")
            ET.SubElement(position, "param", name="initial_value").text = initial_position
            ET.SubElement(joint, "state_interface", name="velocity")
            ET.SubElement(joint, "state_interface", name="effort")

    imu_sensor = ET.SubElement(
        hardware, "sensor", name="trunk_imu"
    )
    
    imu_interfaces = [
        "orientation.x",
        "orientation.y",
        "orientation.z",
        "orientation.w",
        "angular_velocity.x",
        "angular_velocity.y",
        "angular_velocity.z",
        "linear_acceleration.x",
        "linear_acceleration.y",
        "linear_acceleration.z",
    ]
    
    for interface_name in imu_interfaces:
        ET.SubElement(
            imu_sensor,
            "state_interface",
            name=interface_name
        )
    

    plugin = ET.SubElement(
        ET.SubElement(robot, "gazebo"), "plugin",
        filename="gz_ros2_control-system",
        name="gz_ros2_control::GazeboSimROS2ControlPlugin",
    )
    ET.SubElement(plugin, "parameters").text = str(controllers)

    imu = ET.SubElement(
        ET.SubElement(robot, "gazebo", reference="trunk"),
        "sensor", name="trunk_imu", type="imu",
    )
    ET.SubElement(imu, "always_on").text = "true"
    ET.SubElement(imu, "update_rate").text = "500"
    ET.SubElement(imu, "topic").text = "/trunk_imu"
    return ET.tostring(robot, encoding="unicode")


def launch_go1(context):
    model_directory = Path(LaunchConfiguration("go1_description").perform(context)).expanduser().resolve()
    gui = LaunchConfiguration("gui").perform(context).lower() == "true"
    config = Path(get_package_share_directory("go1_gazebo")) / "config"
    simulation = Path(get_package_share_directory("ros_gz_sim")) / "launch" / "gz_sim.launch.py"
    description = go1_description(model_directory, config / "go1_controllers.yaml")

    gazebo = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(str(simulation)),
        launch_arguments={
            "gz_args": f"-r {'' if gui else '-s '}{quote(str(config / 'empty.sdf'))}",
            "on_exit_shutdown": "true",
        }.items(),
    )
    robot_state = Node(
        package="robot_state_publisher", executable="robot_state_publisher",
        parameters=[{
            "use_sim_time": True,
            "robot_description": ParameterValue(description, value_type=str),
        }],
    )
    bridge = Node(
        package="ros_gz_bridge", executable="parameter_bridge",
        arguments=[
            "/clock@rosgraph_msgs/msg/Clock[gz.msgs.Clock",
            "/trunk_imu@sensor_msgs/msg/Imu[gz.msgs.IMU",
        ],
    )
    spawn = Node(
        package="ros_gz_sim", executable="create", output="screen",
        arguments=["-world", "empty", "-name", "go1", "-topic", "/robot_description", "-z", "0.25"],
    )
    controllers = Node(
        package="controller_manager", executable="spawner", output="screen",
        arguments=[
            "joint_state_broadcaster", "go1_effort_controller",
            "--controller-manager-timeout", "60",
        ],
    )

    def start_controllers(event, _context):
        if event.returncode != 0:
            return [EmitEvent(event=Shutdown(reason="Go1 spawn failed"))]
        return [controllers]

    return [
        gazebo, robot_state, bridge,
        RegisterEventHandler(OnProcessExit(target_action=spawn, on_exit=start_controllers)),
        spawn,
    ]


def generate_launch_description():
    return LaunchDescription([
        DeclareLaunchArgument("go1_description", description="Path to Unitree's Go1 description directory"),
        DeclareLaunchArgument("gui", default_value="true", choices=["true", "false"]),
        OpaqueFunction(function=launch_go1),
    ])
