import os
import sys
import shutil
import subprocess
from timeit import timeit

scripts_dir = os.path.dirname(os.path.realpath(__file__))
build_tools_dir = os.path.abspath(os.path.join(scripts_dir, os.pardir))
root_dir = os.path.abspath(os.path.join(build_tools_dir, os.pardir))
build_dir = os.path.join(root_dir, "build")
bin_dir = os.path.join(build_dir, "bin")
generator = "Visual Studio 15 2017 Win64"
cmake_base_command = ["cmake"]
cmake_generator_args = ["-G", generator]

def change_dir_noexcept(path):
    try:
        os.chdir(path)
    except:
        os.chdir(root_dir)

def call_in_directory(dir, fn, *args, **kwargs):
    if not os.path.exists(dir):
        os.makedirs(dir)
    prevWorkingDir=os.getcwd()
    os.chdir(dir)
    try:
        return fn(*args, **kwargs)
    finally:
        change_dir_noexcept(prevWorkingDir)

def run_cmake_command(args):
    command = cmake_base_command.copy()
    command.extend(args)
    call_in_directory(build_dir, subprocess.check_call, command, stderr=subprocess.STDOUT, shell=True)

def delete_build():
    if os.path.exists(build_dir) :
        call_in_directory(root_dir, shutil.rmtree, build_dir, ignore_errors=True)

def generate_project(options):
    command = cmake_generator_args.copy()
    command.append(root_dir)
    command.extend(options)
    run_cmake_command(command)

def build_project(target="ALL_BUILD", config="Release"):
    command = cmake_base_command.copy()
    command.extend(["--build", build_dir])
    command.extend(["--target", target])
    command.extend(["--config", config])
    call_in_directory(build_dir, subprocess.check_call, command, stderr=subprocess.STDOUT, shell=True)

def run_executable(name, args = [], runs = 1):
    subprocess_args = [name]
    subprocess_args.extend(args)
    stmt_str = "subprocess.check_call({0}, stderr=subprocess.STDOUT, shell=True)".format(subprocess_args)
    runtime_dir = bin_dir
    return call_in_directory(runtime_dir, timeit, stmt = stmt_str, setup = "import subprocess", number=runs)
