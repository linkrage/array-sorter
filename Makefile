.PHONY: all help configure build cpp-tests python-tests tests wheel install clean distclean uv-bootstrap

## Tooling configuration ----------------------------------------------------- ##
CMAKE ?= cmake
CTEST ?= ctest
PYTHON ?= /usr/bin/python3

BUILD_DIR ?= build
BUILD_TYPE ?= Release
CMAKE_CONFIG_FLAGS ?=
ifneq (,$(shell command -v ninja 2>/dev/null))
CMAKE_CONFIG_FLAGS += -G Ninja
endif
CMAKE_BUILD_FLAGS ?=
CTEST_FLAGS ?= --output-on-failure
DEFAULT_PARALLEL := $(shell python3 -c 'import multiprocessing; print(max(1, multiprocessing.cpu_count() - 1))' 2>/dev/null || echo 1)
CMAKE_PARALLEL ?= $(DEFAULT_PARALLEL)

## High-level targets -------------------------------------------------------- ##
all: build

help:
	@echo "Common targets:"
	@echo "  make build          - Configure (if needed) and build the project"
	@echo "  make uv-bootstrap   - Create .venv and install requirements (prefers uv)"
	@echo "  make tests          - Run the full CTest suite (C++ + Python)"
	@echo "  make cpp-tests      - Run only the native Catch2 tests"
	@echo "  make python-tests   - Run only the Python pytest suite via CTest"
	@echo "  make wheel          - Build a Python wheel into dist/"
	@echo "  make install        - Install the freshly built wheel with pip"
	@echo "  make clean          - Clean the CMake build tree"
	@echo "  make distclean      - Remove the entire build directory"
	@echo ""
	@echo "Variables:"
	@echo "  BUILD_DIR=<dir>     - Build tree location (default: build)"
	@echo "  BUILD_TYPE=<type>   - CMake build type (default: Release)"
	@echo "  PYTHON=<path>       - Python interpreter passed to CMake"
	@echo "  CMAKE_PARALLEL=<n>  - Override default parallel builds (default: cores-1)"

## Build orchestration ------------------------------------------------------- ##
configure:
	$(CMAKE) -S . -B $(BUILD_DIR) \
	    -DPython3_EXECUTABLE=$(PYTHON) \
	    -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
	    $(CMAKE_CONFIG_FLAGS)

build: configure
	$(CMAKE) --build $(BUILD_DIR) --config $(BUILD_TYPE) --parallel $(CMAKE_PARALLEL) $(CMAKE_BUILD_FLAGS)

## Test helpers -------------------------------------------------------------- ##
tests: build
	cd $(BUILD_DIR) && $(CTEST) $(CTEST_FLAGS)

cpp-tests: build
	cd $(BUILD_DIR) && ./test_sort_cpp

python-tests: build
	cd $(BUILD_DIR) && $(CTEST) $(CTEST_FLAGS) -R python_sort_tests

wheel:
	$(PYTHON) -m pip install --upgrade build
	$(PYTHON) -m build --wheel --outdir dist

install:
	@if ! ls dist/array_sorter-*.whl >/dev/null 2>&1; then \
		$(MAKE) wheel || exit $$?; \
	fi
	$(PYTHON) -m pip install --upgrade dist/array_sorter-*.whl

uv-bootstrap:
	@if command -v uv >/dev/null 2>&1; then \
		echo ">> Using uv to bootstrap dependencies into .venv"; \
		uv venv .venv; \
		uv pip install --python .venv/bin/python -r requirements.txt; \
	else \
		echo ">> uv not found; falling back to python -m venv + pip"; \
		python3 -m venv .venv; \
		.venv/bin/python -m pip install --upgrade pip; \
		.venv/bin/pip install -r requirements.txt; \
	fi

## Cleanup ------------------------------------------------------------------- ##
clean:
	@if [ -f "$(BUILD_DIR)/CMakeCache.txt" ]; then \
		$(CMAKE) --build $(BUILD_DIR) --config $(BUILD_TYPE) --target clean; \
	elif [ -d "$(BUILD_DIR)" ]; then \
		echo "Warning: $(BUILD_DIR) exists but is not a CMake build tree; removing it."; \
		rm -rf "$(BUILD_DIR)"; \
	fi
	rm -rf array_sorter.egg-info build-catch

distclean: clean
	rm -rf $(BUILD_DIR) dist
