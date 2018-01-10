#ifndef LINKFORMAT_HPP
#define LINKFORMAT_HPP


#include <regex>
#include <algorithm>
#include "UString.hpp"
#include <algorithm>
#include <iterator>
#include <boost/algorithm/string.hpp>
#include "Attribute.hpp"

namespace coap{
    class LinkFormat{
    
    private:
        static const std::string lf_path_pattern_;
        static const std::string lf_attr_name_pattern_;
        static const std::string lf_attr_t1_pattern_;
        static const std::string lf_attr_t2_pattern_;
        static const std::string lf_attr_t3_pattern_;
        static const std::string link_format_pattern_;

        UString raw_data_;
        UString path_;
        std::vector<Attribute> attribute_list_;
        Attribute parse_attribute(UString attribute_lf, int flags);

    public:
        LinkFormat(UString link_format, int flags);
        bool is_valid();
        void set_path(UString path);
        UString get_path();
        std::vector<Attribute> get_attribute_list();
        Attribute get_attribute(UString name);
        bool is_attribute_exist(UString name);
        static bool check_link_format(UString link_format);
    };
}

#endif