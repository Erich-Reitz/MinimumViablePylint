"""
This is the main file for the project.
"""

# standard library imports
import argparse
from dataclasses import dataclass
import sys
import subprocess
from typing import List, Set, Tuple
from subprocess import CompletedProcess

# HANDLING
def handle_no_pylint_errors():
    print("No pylint errors found.")
    sys.exit(1)


def handle_no_messages_control_section():
    print("No messages control section found in pylintrc file.")
    print("Add one by placing [MESSAGES CONTROL] in the pylintrc file.")
    sys.exit(1)


# MODELS
@dataclass
class PylintError:
    """
    Relevant information about a pylint error.
    """

    error_code_name: str

    def __hash__(self) -> int:
        return hash(self.error_code_name)


# BOOLEAN FUNCTIONS
def follows_pylint_name_convention(line: str) -> bool:
    """
    Check if a line of pylint output follows the convention
    of containing alphabetical characters split by hyphens
    """
    return line.split("-")[0].isalpha()


# PARSING PYLINT OUTPUT
def parse_error_code(error_code_line: str) -> PylintError:
    """
    Parse a line of pylint error code.
    """
    # parse text in the last set of parentheses
    error_code_name = error_code_line.split("(")[-1].split(")")[0]
    if follows_pylint_name_convention(error_code_name):
        return PylintError(error_code_name)

    return None


def parse(pylint_output: str) -> List[PylintError]:
    """
    Parse the output of pylint.
    """
    errors: Set(PylintError) = set()
    for line in pylint_output.split("\n"):
        if parse_error_code(line):
            errors.add(parse_error_code(line))

    return errors


def get_completed_process_output(command: str):
    process: CompletedProcess = subprocess.run(
        command, stdout=subprocess.PIPE, check=False
    )
    if process.returncode == 0:
        handle_no_pylint_errors()

    return process.stdout.decode("utf-8")


# PARSING PYLINTRC FILE
def mgs_control_begin_and_end_lines(rc_file: List[str]) -> Tuple[int, int]:
    """
    Returns the lines of the messages control section of the pylintrc file.
    """
    begin = 0
    end = 0
    for i, line in enumerate(rc_file):
        if line.startswith("[MESSAGES CONTROL]"):
            begin = i
        if line.startswith("[") and begin != i:
            end = i
            break

    if begin == 0 or end == 0:
        handle_no_messages_control_section()

    return begin, end


def get_sections_of_config_file(file_name: str):
    """
    Returns the sections of the pylintrc file.
    """
    with open(file_name, "r") as pylint_rc_file:
        rc_file: List[str] = pylint_rc_file.readlines()

    begin, end = mgs_control_begin_and_end_lines(rc_file)

    # parse blank lines off the end of the messages control section
    faux_end = end
    while not rc_file[faux_end - 1] or rc_file[faux_end - 1] == "\n":
        faux_end -= 1

    before = rc_file[:begin]
    msgs_control_section = rc_file[begin:faux_end]
    after = rc_file[end:]

    return before, msgs_control_section, after


# CURATING OUTPUT
def add_errors_to_messages_control_section(
    messages_control_section: List[str], errors: Set[PylintError]
):
    """
    Add the new messages control section to the pylintrc file.
    """
    for error in errors:
        error_code_line = f"\t{error.error_code_name},\n"
        messages_control_section.append(error_code_line)

    messages_control_section.append("\n")


def write_pylint_rc_file(
    before: List[str], msgs_control_section: List[str], after: List[str]
):
    with open(".pylintrc", "w+") as pylint_rc_file:
        for line in before:
            pylint_rc_file.write(line)
        for line in msgs_control_section:
            pylint_rc_file.write(line)
        for line in after:
            pylint_rc_file.write(line)


def main():
    argparser = argparse.ArgumentParser()
    argparser.add_argument(
        "--command", "-c", required=True, help="command to run", nargs="+", type=str
    )
    argparser.add_argument(
        "--file",
        "-f",
        required=False,
        help="configuration file",
        default=".pylintrc",
        type=str,
    )

    supplied_command = argparser.parse_args().command
    config_filename = argparser.parse_args().file

    pylint_output = get_completed_process_output(supplied_command)

    errors: Set[PylintError] = parse(pylint_output)

    before, msgs_control_section, after = get_sections_of_config_file(config_filename)

    add_errors_to_messages_control_section(msgs_control_section, errors)

    write_pylint_rc_file(before, msgs_control_section, after)


if __name__ == "__main__":
    main()
