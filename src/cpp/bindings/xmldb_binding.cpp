#include <pybind11/attr.h>
#include <pybind11/detail/common.h>
#include <pybind11/pybind11.h>

// enable type conversion from python objects to C++ containers
#include "pybind11/stl.h"

#include "Entry.hpp"

namespace py = pybind11;

// PYBIND11_MODULE是个宏。第一个参数是直接写的名字，虽然看上去有点奇怪

PYBIND11_MODULE(xmldb, m)
{
    m.doc() = "An interface for xmldb";

    py::class_<Entry> entry(m, "Entry");

    entry.def(py::init());
    entry.def("pushProps", &Entry::pushProps);
    entry.def("getProps", &Entry::getProps);
    entry.def("setAttr", &Entry::setAttr);
    entry.def("getAttr", &Entry::getAttr);
    entry.def("setEntryType", &Entry::setEntryType);

    entry.def_readwrite("properties", &Entry::properties)
        .def_readwrite("attributes", &Entry::attributes);
}