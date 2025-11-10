#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <algorithm>
#include <cstring>
#include <sstream>
#include <vector>

#include "sort.h"

namespace py = pybind11;

namespace {

enum class BufferPrecision {
    kNone,
    kFloat32,
    kFloat64,
};

bool is_text_type(const py::handle& object) {
    return PyUnicode_Check(object.ptr()) || PyBytes_Check(object.ptr());
}

std::string type_name(const py::handle& handle) {
    return std::string(py::str(py::type::of(handle).attr("__name__")));
}

std::vector<double> to_numeric_vector(const py::sequence& seq) {
    const py::ssize_t length = py::len(seq);
    std::vector<double> values;
    values.reserve(static_cast<size_t>(length));

    for (py::ssize_t i = 0; i < length; ++i) {
        py::handle item = seq[i];
        try {
            values.push_back(py::cast<double>(item));
        } catch (const py::cast_error&) {
            std::ostringstream oss;
            oss << "array_sorter.sort expects numeric values; index " << i
                << " is of type '" << type_name(item) << "'";
            throw py::type_error(oss.str().c_str());
        }
    }

    return values;
}

void write_back(py::sequence& seq, const std::vector<double>& values) {
    const py::ssize_t length = py::len(seq);
    if (values.size() != static_cast<size_t>(length)) {
        throw std::runtime_error("sequence length changed while sorting");
    }

    if (!PyObject_HasAttrString(seq.ptr(), "__setitem__")) {
        throw py::type_error("sequence does not support item assignment");
    }

    py::object setter = seq.attr("__setitem__");
    for (size_t i = 0; i < values.size(); ++i) {
        setter(static_cast<py::ssize_t>(i), values[i]);
    }
}

std::vector<double> sort_vector(std::vector<double> values, bool reverse) {
    sort_array(values);
    if (reverse) {
        std::reverse(values.begin(), values.end());
    }
    return values;
}

std::string normalize_format(const char* raw_format) {
    if (raw_format == nullptr) {
        return {};
    }
    const char* cursor = raw_format;
    while (*cursor == '@' || *cursor == '<' || *cursor == '>' || *cursor == '!' || *cursor == '=') {
        ++cursor;
    }
    return std::string(cursor);
}

BufferPrecision deduce_precision(const Py_buffer& view) {
    const std::string format = normalize_format(view.format);
    if (view.itemsize == sizeof(double) &&
        (format == "d" || format == "f8" || format == "float64" || format == "double")) {
        return BufferPrecision::kFloat64;
    }
    if (view.itemsize == sizeof(float) &&
        (format == "f" || format == "f4" || format == "float32")) {
        return BufferPrecision::kFloat32;
    }
    return BufferPrecision::kNone;
}

bool try_sort_via_buffer(py::object& arr, bool reverse) {
    if (!PyObject_CheckBuffer(arr.ptr())) {
        return false;
    }

    Py_buffer view{};
    if (PyObject_GetBuffer(arr.ptr(), &view, PyBUF_WRITABLE | PyBUF_FORMAT) != 0) {
        PyErr_Clear();
        throw py::type_error("array_sorter.sort requires a writable numeric buffer");
    }

    struct BufferReleaser {
        Py_buffer* view;
        explicit BufferReleaser(Py_buffer* v) : view(v) {}
        ~BufferReleaser() {
            if (view != nullptr) {
                PyBuffer_Release(view);
            }
        }
    } releaser(&view);

    if (view.ndim != 1) {
        throw py::type_error("array_sorter.sort only supports one-dimensional buffers");
    }

    const BufferPrecision precision = deduce_precision(view);
    if (precision == BufferPrecision::kNone) {
        return false;
    }

    const size_t count = static_cast<size_t>(view.len) / static_cast<size_t>(view.itemsize);
    std::vector<double> values(count);
    if (precision == BufferPrecision::kFloat64) {
        std::memcpy(values.data(), view.buf, count * sizeof(double));
    } else {
        const float* src = static_cast<const float*>(view.buf);
        for (size_t i = 0; i < count; ++i) {
            values[i] = static_cast<double>(src[i]);
        }
    }

    values = sort_vector(std::move(values), reverse);

    if (precision == BufferPrecision::kFloat64) {
        std::memcpy(view.buf, values.data(), count * sizeof(double));
    } else {
        float* dst = static_cast<float*>(view.buf);
        for (size_t i = 0; i < count; ++i) {
            dst[i] = static_cast<float>(values[i]);
        }
    }

    return true;
}

}  // namespace

void sort_in_place(py::object arr, bool reverse) {
    if (try_sort_via_buffer(arr, reverse)) {
        return;
    }

    if (!PySequence_Check(arr.ptr()) || is_text_type(arr)) {
        throw py::type_error(
            "array_sorter.sort expects a mutable sequence (for example, a list)");
    }

    py::sequence sequence = py::reinterpret_borrow<py::sequence>(arr);

    std::vector<double> values = to_numeric_vector(sequence);
    values = sort_vector(std::move(values), reverse);

    try {
        write_back(sequence, values);
    } catch (const py::error_already_set& error) {
        if (error.matches(PyExc_TypeError) || error.matches(PyExc_RuntimeError)) {
            throw py::type_error(
                "array_sorter.sort requires a sequence that supports item assignment");
        }
        throw;
    }
}

py::list sorted_copy(py::object iterable, bool reverse) {
    py::list snapshot = py::list(iterable);
    std::vector<double> values = to_numeric_vector(snapshot);
    values = sort_vector(std::move(values), reverse);

    py::list result(values.size());
    for (size_t i = 0; i < values.size(); ++i) {
        result[i] = values[i];
    }
    return result;
}

PYBIND11_MODULE(array_sorter, m) {
    m.doc() = "Efficient array sorting library with a C++ backend";

    m.def("sort", &sort_in_place, R"pbdoc(
        Sort a mutable Python sequence in-place.

        Args:
            arr: A mutable sequence (e.g. list, array.array) containing numeric values.
            reverse: If True, the sequence is sorted in descending order.
    )pbdoc",
          py::arg("arr"),
          py::kw_only(),
          py::arg("reverse") = false);

    m.def("sorted", &sorted_copy, R"pbdoc(
        Return a sorted copy of the provided iterable.

        Args:
            iterable: Any iterable containing numeric values.
            reverse: If True, return the values in descending order.
    )pbdoc",
          py::arg("iterable"),
          py::kw_only(),
          py::arg("reverse") = false);
}
