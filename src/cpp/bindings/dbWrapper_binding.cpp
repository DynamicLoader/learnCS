#include <pybind11/attr.h>
#include <pybind11/detail/common.h>
#include <pybind11/pybind11.h>
#include <string>

// enable type conversion from python objects to C++ containers
#include "intkey.hpp"
#include "pybind11/stl.h"

#include "DBWrapper.hpp"

namespace py = pybind11;

PYBIND11_MODULE(dbwrapper, m)
{
    m.doc() = "An interface for dbwrapper";

    py::class_<DBWrapper> dbwrapper(m, "DBWrapper");

    dbwrapper
        .def(py::init())
        // Insertion and update
        .def("insertFullTable", &DBWrapper::insertFullTable)
        .def("insertAuthorName", &DBWrapper::insertAuthorName)
        .def("pushAuthorTable", &DBWrapper::pushAuthorTable)
        .def("pushCoauthorTable", &DBWrapper::pushCoauthorTable)
        .def("pushTitleTable", &DBWrapper::pushTitleTable)
        .def("computeKeyword", &DBWrapper::computeKeyword)
        .def("calcMaxKClique", &DBWrapper::calcMaxKClique)
        .def("saveCli", &DBWrapper::saveCli)
        .def("commit", &DBWrapper::commit)

        // Table querys
        .def("queryByFullTitle", &DBWrapper::queryByFullTitle)
        .def("queryByFullTitleRaw", &DBWrapper::queryByFullTitleRaw)
        .def("queryByAuthor", &DBWrapper::queryByAuthor)
        .def("queryCoauthor", &DBWrapper::queryCoauthor)
        .def("getRealName", &DBWrapper::getRealName)
        .def("getRealNameStr", &DBWrapper::getRealNameStr)
        .def("queryByKeyword", &DBWrapper::queryByKeyword)
        .def("queryByKeywordRaw", &DBWrapper::queryByKeywordRaw)
        .def("queryTopKeyword", &DBWrapper::queryTopKeyword)
        .def("queryTopAuthor", &DBWrapper::queryTopAuthor)
        .def("queryTopYearKeyword", &DBWrapper::queryTopYearKeyword)

        // Key conversions
        .def("keyList2HexStringList", &DBWrapper::keyList2HexStringList)
        .def("HexString2Result", &DBWrapper::HexString2Result)

        // Other utilities
        .def("preload", &DBWrapper::preload)
        .def("test_using", &DBWrapper::test_using)
        .def("queryMaxCli", &DBWrapper::queryMaxCli)
        .def("saveTopYearKeyword", &DBWrapper::saveTopYearKeyword) ;

    py::class_<intKey_t> intkey(m, "intKey_t");
    intkey.def("to_bytes", &intKey_t::to_bytes);
    intkey.def("toString", &intKey_t::operator std::string);
    intkey.def_static("fromHexString", &intKey_t::fromHexString);
};
