////////////////////////////////////////////////////////////////////////////////
#include <pybind11/pybind11.h>
#include <pybind11/eigen.h>
#include <pybind11/cast.h>
#include <pybind11/stl.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <type_traits>
////////////////////////////////////////////////////////////////////////////////

namespace py = pybind11;
namespace nl = nlohmann;

using namespace pybind11::literals;

namespace nlohmann
{

    namespace detail
    {
        py::object from_json_impl(const json& j)
        {
            if (j.is_null())
            {
                return py::none();
            }
            if (j.is_boolean())
            {
                return py::bool_(j.get<bool>());
            }
            if (j.is_number())
            {
                double number = j.get<double>();
                if (number == std::floor(number))
                {
                    return py::int_(j.get<int>());
                }
                else
                {
                    return py::float_(number);
                }
            }
            if (j.is_string())
            {
                return py::str(j.get<std::string>());
            }
            if (j.is_array())
            {
                py::list obj;
                for (const auto& el: j)
                {
                    obj.attr("append")(from_json_impl(el));
                }
                return obj;
            }
            if (j.is_object())
            {
                py::dict obj;
                for (json::const_iterator it = j.cbegin(); it != j.cend(); ++it)
                {
                    obj[py::str(it.key())] = from_json_impl(it.value());
                }
                return obj;
            }
        }

        json to_json_impl(py::handle obj)
        {
            if (obj.is_none())
            {
                return nullptr;
            }
            if (py::isinstance<py::bool_>(obj))
            {
                return obj.cast<bool>();
            }
            if (py::isinstance<py::int_>(obj))
            {
                return obj.cast<long>();
            }
            if (py::isinstance<py::float_>(obj))
            {
                return obj.cast<double>();
            }
            if (py::isinstance<py::str>(obj))
            {
                return obj.cast<std::string>();
            }
            if (py::isinstance<py::tuple>(obj) || py::isinstance<py::list>(obj))
            {
                json out;
                for (py::handle value: obj)
                {
                    out.push_back(to_json_impl(value));
                }
                return out;
            }
            if (py::isinstance<py::dict>(obj))
            {
                json out;
                for (py::handle key: obj)
                {
                    out[key.cast<std::string>()] = to_json_impl(obj[key]);
                }
                return out;
            }
            throw std::runtime_error("to_json not implemented for this type of object: " + obj.cast<std::string>());
        }
    }

    template <>
    struct adl_serializer<py::object>
    {
        static py::object from_json(const json& j)
        {
            return detail::from_json_impl(j);
        }

        static void to_json(json& j, const py::object& obj)
        {
            j = detail::to_json_impl(obj);
        }
    };

}

////////////////////////////////////////////////////////////////////////////////

PYBIND11_MODULE(pyjson, m) {
    m.doc() = "pyjson module"; // optional module docstring

    // Dummy json
    m.def("dummy_json", []() {
        nl::json j = {
            {"pi", 3.141},
            {"happy", true},
            {"name", "Niels"},
            {"nothing", nullptr},
            {"answer", {
                {"everything", 42}
            }},
            {"list", {1, 0, 2}},
            {"object", {
                {"currency", "USD"},
                {"value", 42.99}
            }}
        };
        py::object out = j;
        return out;
    }, "returns a dummy json object");
}
