FROM python:3.11-slim

# Install build dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    git \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Install Python requirements early to leverage Docker layer caching
COPY requirements.txt .
RUN python -m pip install --upgrade pip \
    && pip install --no-cache-dir -r requirements.txt

# Copy the remainder of the project
COPY . .

# Configure, build and exercise the entire test suite (C++ + Python)
RUN JOBS=$(python -c "import multiprocessing; print(max(1, multiprocessing.cpu_count() - 1))") && \
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release && \
    cmake --build build --config Release --parallel ${JOBS} && \
    ctest --test-dir build --output-on-failure

# Build and install the Python wheel to validate packaging metadata
RUN pip install . && python -m pip install --upgrade pip

# Smoke-test the installed module
RUN python - <<'PY'
import array_sorter
sample = [3.0, 1.0, 2.0]
array_sorter.sort(sample)
assert sample == [1.0, 2.0, 3.0]
print("array_sorter ready ({} values)".format(len(sample)))
PY

CMD ["python"]
