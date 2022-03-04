#include <iostream>

#include "boost/json.hpp"


void pretty_print(std::ostream &os, boost::json::value const &jv, std::string *indent = nullptr)
{
    namespace json = boost::json;
    std::string indent_;
    if (!indent)
        indent = &indent_;
    switch (jv.kind())
    {
    case json::kind::object:
    {
        os << "This is Json object ! \n";
        os << "{\n";
        indent->append(4, ' ');
        auto const &obj = jv.get_object();
        if (!obj.empty())
        {
            auto it = obj.begin();
            for (;;)
            {
                os << *indent << json::serialize(it->key()) << " : ";
                pretty_print(os, it->value(), indent);
                if (++it == obj.end())
                    break;
                os << ",\n";
            }
        }
        os << "\n";
        indent->resize(indent->size() - 4);
        os << *indent << "}";
        break;
    }

    case json::kind::array:
    {
        os << "[\n";
        indent->append(4, ' ');
        auto const &arr = jv.get_array();
        if (!arr.empty())
        {
            auto it = arr.begin();
            for (;;)
            {
                os << *indent;
                pretty_print(os, *it, indent);
                if (++it == arr.end())
                    break;
                os << ",\n";
            }
        }
        os << "\n";
        indent->resize(indent->size() - 4);
        os << *indent << "]";
        break;
    }

    case json::kind::string:
    {
        os << json::serialize(jv.get_string());
        break;
    }

    case json::kind::uint64:
        os << jv.get_uint64();
        break;

    case json::kind::int64:
        os << jv.get_int64();
        break;

    case json::kind::double_:
        os << jv.get_double();
        break;

    case json::kind::bool_:
        if (jv.get_bool())
            os << "true";
        else
            os << "false";
        break;

    case json::kind::null:
        os << "null";
        break;
    }

    if (indent->empty())
        os << "\n";
}



int main(int argc, char* argv[]) {
  std::string validJson ( "{\"menu\" : { \"id\" : \"file\", \"value\" : \"File\", \"popup\" : {\"menuitem\" : [{\"value\" : \"New\", \"onclick\" : \"CreateNewDoc()\"}, {\"value\" : \"Open\", \"onclick\" : \"OpenDoc()\"}, {\"value\" : \"Close\", \"onclick\" : \"CloseDoc()\"}]}}}"
  );

    boost::json::error_code errorConde;
    boost::json::value jv ;
    try
    {
        jv = boost::json::parse(validJson);
    } 
    catch( std::bad_alloc const& e)
    {
        std::cout << "Parsing failed: " << e.what() << "\n";
    }

    
    pretty_print(std::cout , jv);

    std::cout << "\n\n serialize " << boost::json::serialize(jv) << std::endl;

}