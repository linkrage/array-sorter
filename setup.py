from pybind11.setup_helpers import Pybind11Extension, build_ext
from setuptools import setup
import pybind11


ext_modules = [
    Pybind11Extension(
        "array_sorter",
        [
            "src/python/bindings.cpp",
            "src/cpp/sort.cpp",
        ],
        include_dirs=[
            "src/cpp",
            pybind11.get_include(),
            pybind11.get_include(user=True),
        ],
        language="c++",
        cxx_std=17,
    ),
]


setup(
    name="array_sorter",
    version="0.1.0",
    author="Z I",
    author_email="ksdfjwwerkjehfwf@qwoijjsdfkjdfwerdfkjsdf.com",
    description="Efficient Python library for sorting arrays with a C++ backend",
    long_description=(
        "Array Sorter exposes a high-performance C++ std::sort implementation "
        "through convenient Python bindings."
    ),
    long_description_content_type="text/plain",
    ext_modules=ext_modules,
    cmdclass={"build_ext": build_ext},
    zip_safe=False,
    python_requires=">=3.8",
    classifiers=[
        "Programming Language :: Python :: 3",
        "Programming Language :: C++",
        "Operating System :: OS Independent",
    ],
)
