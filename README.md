# Array Sorter Library

An efficient Python library for sorting arrays, with a high-performance C++ backend implementation.

## Overview

This library provides a Python interface to a C++ sorting implementation, allowing Python developers to leverage optimized C++ code without needing to understand C++ internals. The compute-intensive sorting operation is delegated to C++ using `std::sort`, while Python bindings are provided via pybind11.

**Highlights**
- In-place sorting for Python lists plus any writable numeric buffer (`array.array('d')`, NumPy float64 arrays, etc.)
- Optional reverse ordering and a convenience `array_sorter.sorted()` helper that leaves the input untouched
- Unified CMake build drives both the C++ and Python test suites through CTest
- Modern packaging metadata (`pyproject.toml`) enables `pip install` / editable workflows

## Quick Start

### Non-Docker shell

```bash
# Optional: bootstrap deps the fast way (falls back to python -m venv if uv missing)
make uv-bootstrap

# Build, test, and package
make build
make tests
PYTHON=$(which python3) make wheel
PYTHON=$(which python3) make install
```

`make install` always uses the interpreter referenced by `PYTHON` (default `/usr/bin/python3`), so point it at your active virtual environment when you want the wheel installed there.

### Docker shell

```bash
docker build -t array_sorter .
docker run -it array_sorter bash
```

The Docker image compiles everything, runs Catch2 + pytest through CTest, builds the wheel, installs it, and drops you into `/app` with `array_sorter` already importable.

## Requirements

### System Requirements
- **Python**: 3.8 or higher
- **CMake**: 3.12 or higher
- **C++ Compiler**: Supporting C++17 standard
  - GCC 7+ (Linux)
  - Clang 5+ (macOS/Linux)
  - MSVC 2017+ (Windows)

### Python Dependencies
- pybind11 >= 2.10.0
- pytest >= 7.0.0 (for testing)

## Installation

### Option 1: Using CMake (Recommended for Development)

```bash
# Optional but recommended: keep Python deps isolated
python -m venv .venv
source .venv/bin/activate
python -m pip install --upgrade pip
pip install -r requirements.txt

# Configure + build everything (C++ lib + Python module)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# If multiple Python interpreters are installed, point CMake at the one where
# pybind11 lives (after `pip install -r requirements.txt`):
# cmake -S . -B build -DPython3_EXECUTABLE=$(which python3)

# Run the entire test matrix (C++ harness + pytest through CTest)
ctest --test-dir build --output-on-failure

# Optionally install the library
cmake --install build --prefix ~/.local

# When running Python code directly from the build tree:
export PYTHONPATH=$PWD/build:$PYTHONPATH
python -m pytest tests/python -v
```

> **Need a faster bootstrap?** If [uv](https://docs.astral.sh/uv/getting-started/) is installed you can run
> `uv venv .venv && source .venv/bin/activate && uv pip install -r requirements.txt`
> (or simply `make uv-bootstrap`). The rest of the workflow remains the same and
> standard `pip` instructions continue to be supported for portability.

> **Parallel builds by default:** The provided Makefile drives `cmake --build` with
> `max(1, CPU cores - 1)` workers via `CMAKE_PARALLEL`, so one core always remains free.
> Override with `CMAKE_PARALLEL=<n>` if you prefer a different job count.

### Option 2: Using pip / editable installs

A modern `pyproject.toml` is provided, so any `pip` (v23+) can build wheels:

```bash
python -m venv .venv
source .venv/bin/activate
pip install --upgrade pip

# Regular install
pip install .

# Or editable mode for iterative development
pip install -e .
```

### Option 3: Using Docker

1. Build the Docker image:
```bash
docker build -t array_sorter .
```

2. Run the container:
```bash
docker run -it array_sorter bash
```

The container automatically builds, tests, and installs the wheel during `docker build`, so you can drop into a shell and start developing with `array_sorter` ready to import.

## Usage

### Basic Example

```python
import array_sorter

# Create an array
numbers = [3.5, 1.2, 4.7, 2.1, 5.9]

# Sort in-place (ascending by default)
array_sorter.sort(numbers)

print(numbers)  # Output: [1.2, 2.1, 3.5, 4.7, 5.9]

# Get a sorted copy without mutating the input
descending = array_sorter.sorted(numbers, reverse=True)
print(descending)  # Output: [5.9, 4.7, 3.5, 2.1, 1.2]
```

### Example Script

See `examples/example.py` for a complete usage example:

```bash
python examples/example.py
```

### Python API

| Function | Description |
| --- | --- |
| `array_sorter.sort(arr, *, reverse=False)` | Sorts Python lists or any writable numeric buffer (`array.array('d')`, NumPy float64 arrays, etc.) in-place. Raises `TypeError` for immutable or non-numeric inputs. |
| `array_sorter.sorted(iterable, *, reverse=False)` | Returns a brand-new Python list with the sorted values, leaving the original iterable untouched. |

## Continuous Integration

GitHub Actions (`.github/workflows/ci.yml`) runs on every push, pull request, or manual dispatch. The workflow:

1. Installs system toolchains (apt-based setup on Linux runners).
2. Sets up Python 3.10 and installs `requirements.txt`.
3. Configures and builds via CMake, explicitly pointing to the runner’s Python interpreter.
4. Executes `ctest --test-dir build --output-on-failure`, which runs both the C++ harness and the pytest suite.
5. Performs `pip install .` to ensure packaging metadata remains valid.

Use this workflow as the baseline for any additional CI providers or extended matrices (e.g., extra Python versions).

## Testing

### Running All Tests

```bash
cmake --build build
ctest --test-dir build --output-on-failure
```

The `cpp_sort_tests` (Catch2) cases and the `python_sort_tests` entry registered with CTest execute the Catch2-powered C++ suite alongside `pytest` with the freshly built extension on `PYTHONPATH`, so both layers run with a single command.

### Running C++ Tests Only

```bash
cmake --build build --target test_sort_cpp
./build/test_sort_cpp
```

### Running Python Tests Only

```bash
# Ensure the freshly built module is on the path
export PYTHONPATH=$PWD/build:$PYTHONPATH
python -m pytest tests/python -v
```

## Project Structure

```
.
├── src/
│   ├── cpp/
│   │   ├── sort.h          # C++ header file
│   │   └── sort.cpp        # C++ implementation
│   └── python/
│       └── bindings.cpp     # Python bindings (pybind11)
├── tests/
│   ├── cpp/
│   │   └── test_sort.cpp   # C++ unit tests (lightweight harness)
│   └── python/
│       └── test_sort.py    # Python unit tests (pytest)
├── examples/
│   └── example.py          # Usage example
├── CMakeLists.txt          # CMake build configuration
├── setup.py                # Python packaging configuration
├── Dockerfile              # Docker build configuration
├── requirements.txt        # Python dependencies
└── README.md               # This file
```

## Build System Details

### CMake Configuration

The project uses CMake for building both the C++ library and Python bindings:

- **C++ Library**: Static library (`cpp_sort`) containing the sorting implementation
- **Python Module**: Shared library (`array_sorter`) with pybind11 bindings
- **Testing**: Catch2-powered C++ suite plus pytest, orchestrated via CTest

### Key CMake Features

- Automatic pybind11 detection and configuration
- C++17 standard enforcement
- Unified CTest entry points for C++ and Python testing
- Cross-platform support (Linux, macOS, Windows)

## Development

### Building from Source

1. Ensure all dependencies are installed
2. Create build directory: `mkdir build && cd build`
3. Configure: `cmake ..`
4. Build: `cmake --build .`
5. Test: `ctest` (C++ tests) and `pytest tests/python/` (Python tests)

### Adding New Features

1. Add C++ implementation in `src/cpp/`
2. Update Python bindings in `src/python/bindings.cpp` if needed
3. Add tests in `tests/cpp/` and `tests/python/`
4. Update CMakeLists.txt if new source files are added

## License

This project is provided under the terms of the [MIT License](LICENSE).

## Notes

- The library sorts arrays **in-place** (modifies the input array)
- Currently supports `double` precision floating-point numbers
- Uses `std::sort` from the C++ standard library for optimal performance
- The Docker environment provides a consistent build and test environment
