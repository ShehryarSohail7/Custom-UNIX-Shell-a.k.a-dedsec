#!/usr/bin/python3

import subprocess
import json
from rich.console import Console
from rich.table import Table
import shutil
import time
import sys
import os
import datetime

class Test:
    """
    The Test class is the main class that runs the tests.

    All the configuration params are loaded from a json file. 
    """
    def __init__(self, config="config.json", roll_num=""):
        """Initializes the test environment with the params defined in the configuration json file.

        Args:
            config (str, optional): The json file with the params. Defaults to "config.json".
        """

        config = json.load(open(config, "r"))

        self.test_directory  = config["test_directory"]
        self.default_tests   = config["default_tests"]
        self.test_files      = config["test_files"]
        self.shell           = config["shell"]
        self.reference_shell = config["reference_shell"]
        self.default_timeout = config["default_timeout"]
        self.log_to_file     = config["log_to_file"]
        self.tests_weightage = config["weightage"]
        self.verbose         = config["verbose"]
        self.output_csv      = open(config["output_csv"], "a")
        
        # initialize a dictionary to store test stats, the format is {file_name : status (passed/failed)} false by default
        self.test_stats = {}
        for test in self.default_tests:
            for file in self.test_files[test]:
                self.test_stats[file] = False
        
        self.test_stats = dict(sorted(self.test_stats.items()))

        # temp_str = ",".join(list(map(str, list(self.test_stats.keys()))))
        # print(temp_str)
        # self.output_csv.write(f"Roll number,Score,{temp_str}\n")
        
        self.rollnum      = roll_num
        self.score        = 0
        self.total_tests  = 0
        self.tests_passed = 0
        self.tests_failed = 0

        self.tests = self.default_tests

        self.console = Console()
        self.temp_output_dir = os.path.join(self.test_directory, "test_output")

    def print_config(self):
        """Prints the configuration params.
        """
        self.console.print("[green]Traces Directory[/green]: ", self.test_directory)
        self.console.print("[green]Tests[/green]: ", ", ".join(self.tests))
        self.console.print("[green]Shell[/green]: ", self.shell)
        self.console.print("[green]Reference Shell[/green]: ", self.reference_shell)
        self.console.print("[green]Default Timeout[/green]: ", str(self.default_timeout))
        self.console.print("[green]Log To File[/green]: ", str(self.log_to_file))
        print()

    def set_tests(self, tests):
        """Sets the tests to run.

        Args:
            tests (list): A list of tests to run. Possible options: easy, medium, advanced
        """

        # make sure the list is correct
        for test in tests:
            if test not in self.default_tests:
                self.console.print(f"[bold red]ERROR[/bold red] : Test {test} does not exist")
                self.console.print("Using default tests")
                return

        self.tests = tests

    def execute_command(self, test_file):
        """Runs the testing "scripts" which are basically a collection of commands, on both shells, the student shell, as well as the reference shell, and dumps the outpus to their respective .out files in the outputs folder.

        Args:
            test_file (str): The name of the test file being tested right now.

        Returns:
            Boolean: True if the shells executed correctly, False otherwise.
        """

        test_shell_file = self.test_directory + "/" + test_file
        ref_shell_file = self.test_directory + "/" + test_file

        ref_specific = test_shell_file + ".custom"

        if os.path.exists(ref_specific):
            ref_shell_file = ref_specific

        test_cmd = f"{self.shell} {test_shell_file} > {self.temp_output_dir}/{test_file}.msh.out 2> /dev/null"
        expected_cmd = f"{self.reference_shell} {ref_shell_file} > {self.temp_output_dir}/{test_file}.{self.reference_shell.split('/')[-1]}.out 2> /dev/null"

        try:
            proc = subprocess.run(test_cmd, shell=True, check=True, timeout=self.default_timeout)
        except subprocess.TimeoutExpired:
            self.console.print("[bold red]FAILED[/bold red]")
            self.console.print(f"\t=> [red]Your shell timed out[/red]")
            return False
        except subprocess.CalledProcessError:
            self.console.print("[bold red]FAILED[/bold red]")
            self.console.print(f"\t=> [red]Your shell exited with non-zero return code. Possible segfault.[/red]")
            return False
        except:
            self.console.print("[bold red]FAILED[/bold red]")
            self.console.print(f"\t=> [red]Your shell failed to execute the file[/red]")
            return False

        try:
            proc = subprocess.run(expected_cmd, shell=True, timeout=self.default_timeout)
        except Exception as error:
            self.console.print("[bold red]FAILED[/bold red]")   
            self.console.print(f"\t=> [red]{self.reference_shell} failed to execute the file {test_cmd}[/red]")
            self.console.print(f"\t=> [red]{error}[/red]")
            return False

        return True
    
    def test_validity(self, test_file):
        """Tests whether the output generated by the student shell is correct or not. This is done by comparing it to the referene shell output.

        Args:
            test_file (str): The test file being tested right now.

        Returns:
            Boolean: True if the output is correct, False otherwise.
        """

        test_shell_out = self.temp_output_dir + "/" + test_file + ".msh.out"
        ref_shell_out = f"{self.temp_output_dir}/{test_file}.{self.reference_shell.split('/')[-1]}.out"    
        
        # read in lines from both files
        try:
            with open(test_shell_out, "r") as file:
                test_out_lines = file.readlines()
        except:
            self.console.print("[bold red]FAILED[/bold red]")
            self.console.print(f"\t=> [red]Your shell failed to execute the command[/red]")
            return False
        
        with open(ref_shell_out, "r") as file:
            ref_out_lines = file.readlines()

        # strip everything in each list
        test_out_lines = list(map(lambda x: x.strip(), test_out_lines))
        ref_out_lines = list(map(lambda x: x.strip(), ref_out_lines))

        for line in ref_out_lines:
            if line not in test_out_lines:
                self.console.print("[bold red]FAILED[/bold red]")
                self.console.print(f"\t=> [red][underline]\"{line}\"[/underline] not found in your shell's output[/red]")
                return False

        self.console.print("[bold green]PASSED[/bold green]")
        return True

    def run_test(self, test_name):
        """Runs a test.

        Args:
            test_name (str): The name of the test to run.
        """

        total_ran    = 0
        total_passed = 0

        command_files = self.test_files[test_name]

        for file in command_files:
            
            self.console.print(f"  Running file: {file:<25}", style="cyan", end="")

            if self.execute_command(file) and self.test_validity(file):
                total_passed += 1
                self.test_stats[file] = True

            total_ran += 1

            self.console.print

        return total_ran, total_passed

    def run(self):
        """Main function of the testing class. Runs the test suite according to how its defined in the configuration file.
        """

        self.console.print("\n\n[bold]SHELL TEST SUITE[/bold]\n")
        self.print_config()
        
        if os.path.exists("../build/build_mode"):
            # open the file and see if the build_mode is release, only allow testing in build mode
            with open("../build/build_mode", "r") as file:
                build_mode = file.read().strip()
                if build_mode != "release":
                    self.console.print("[bold red]ERROR[/bold red] : Build mode is not release")
                    self.console.print("Please build in release mode to run the tests")
                    return

        if not os.path.exists(self.shell):
            self.console.print(f"[bold red]ERROR[/bold red] : Shell {self.shell} does not exist")
            return

        if os.path.exists(self.temp_output_dir):
            shutil.rmtree(self.temp_output_dir)

        os.mkdir(self.temp_output_dir)

        with open(f"{self.temp_output_dir}/timestamp.log", "w") as file:
            file.write(str(datetime.datetime.now()))

        print("...\n")

        for test in self.tests:
            
            self.console.print(f"Running tests : {test}", style="bold yellow")

            total, passed = self.run_test(test)

            self.total_tests  += total
            self.tests_passed += passed
            self.tests_failed += (total - passed)

            if total == 0:
                continue
            
            self.score += self.tests_weightage[test] * (passed / total)


        print("\n...\n")
        self.console.print("[bold]SUMMARY[/bold]\n")
        self.console.print(f"Total : {self.total_tests}, Passed : {self.tests_passed}, Failed : {self.tests_failed}")
        self.console.print(f"Score : {self.score}")
        
        # logging results to csv
        if self.output_csv:
            temp_str = ",".join(list(map(str, list(self.test_stats.values()))))
            self.output_csv.write(f"{self.rollnum},{self.score:<.2f},{temp_str}\n")
            
        self.output_csv.close()

if __name__ == "__main__":
    
    # the tests to run are passed via the command line args, if none is passed use the default tests

    roll_num = ""
    if sys.argv[1:]:
        roll_num = sys.argv[1]
    
    test = Test(roll_num=roll_num)
    # test.set_tests(["easy"])
    start = time.time()
    test.run()
    end = time.time()
    print(f"Finished in {end - start:<.2f}s.")