import pytest
import subprocess
import os

# Directory paths
INPUT_DIR = os.path.normpath("inputs")
OUTPUT_DIR = os.path.normpath("outputs")

def pytest_addoption(parser):
    """Add custom command-line option for executable path."""
    parser.addoption(
        "--exe", 
        action="store", 
        default="../yacc.exe", 
        help="Path to the executable to test"
    )

@pytest.fixture(scope="session")
def executable_path(pytestconfig):
    """Get the executable path from command-line argument."""
    exe_path = pytestconfig.getoption("--exe")
    return os.path.normpath(exe_path)

def collect_test_cases():
    """Collect test cases from inputs directory."""
    test_cases = []
    if not os.path.exists(INPUT_DIR):
        pytest.fail(f"Input directory {INPUT_DIR} does not exist")
    
    for input_file in os.listdir(INPUT_DIR):
        if not input_file.endswith(".txt"):
            continue
        input_path = os.path.join(INPUT_DIR, input_file)
        output_file = input_file.replace(".txt", ".out")
        output_path = os.path.join(OUTPUT_DIR, output_file)
        
        # Determine expected exit code based on filename
        if input_file.startswith("valid"):
            expected_exit_code = 0
        elif input_file.startswith("invalid"):
            expected_exit_code = 1
        else:
            continue  # Skip files that don't start with valid/invalid
        
        test_cases.append((input_path, output_path, expected_exit_code))
    
    if not test_cases:
        pytest.fail(f"No valid test cases found in {INPUT_DIR}")
    
    return test_cases

@pytest.mark.parametrize(
    "input_file,expected_output_file,expected_exit_code",
    collect_test_cases()
)
def test_program(input_file, expected_output_file, expected_exit_code, executable_path):
    """Test the executable with the given input file."""
    assert os.path.exists(input_file), f"Input file {input_file} not found"

    # Check if the filename ends with "_0_" or "_1_"
    filename = os.path.basename(input_file)
    check_output = "_1_" in filename

    expected_output = ""
    if check_output and os.path.exists(expected_output_file):
        with open(expected_output_file, "r", encoding="utf-8") as f:
            expected_output = f.read().strip()

    assert os.path.exists(executable_path), f"Executable {executable_path} not found"

    try:
        process = subprocess.run(
            [executable_path, input_file],
            text=True,
            capture_output=True,
            timeout=5,
            encoding="utf-8"
        )
    except subprocess.TimeoutExpired:
        pytest.fail(f"Executable timed out on input: {input_file}")
    except UnicodeDecodeError:
        pytest.fail(f"Output encoding error on input: {input_file}")
    # In test_program function, before the assert
    print(f"Input content: {open(input_file).read()}")
    print(f"Actual stderr: {process.stderr}")
    print(f"Actual stdout: {process.stdout}")
    assert process.returncode == expected_exit_code, (
        f"Expected exit code {expected_exit_code}, got {process.returncode}\n"
        f"Input file: {input_file}\n"
        f"Stderr: {process.stderr}"
    )

    if check_output:
        if expected_exit_code == 0:
            actual_output = process.stdout.strip()
            assert actual_output == expected_output, (
                f"Output mismatch!\nInput: {input_file}\n"
                f"Expected:\n{expected_output}\nGot:\n{actual_output}"
            )
        else:
            actual_error = process.stderr.strip()
            assert actual_error == expected_output, (
                f"Error message mismatch!\nInput: {input_file}\n"
                f"Expected:\n{expected_output}\nGot:\n{actual_error}"
            )
