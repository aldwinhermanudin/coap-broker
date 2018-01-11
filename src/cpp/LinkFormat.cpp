#include "LinkFormat.hpp"

namespace coap{

	const std::string LinkFormat::lf_path_pattern_ = "<[\\w-][\\w\\/\\-]+[\\w-]>";
	const std::string LinkFormat::lf_attr_name_pattern_ = "[\\w-]+";
	const std::string LinkFormat::lf_attr_t1_pattern_ = "[0-9]+";
	const std::string LinkFormat::lf_attr_t2_pattern_ = "(\"[\\w-]+\")";
	const std::string LinkFormat::lf_attr_t3_pattern_ = "([\\w-])+";
	const std::string LinkFormat::link_format_pattern_= std::string("^") + lf_path_pattern_ + std::string("(;") + lf_attr_name_pattern_ + std::string("=(")
														+ lf_attr_t1_pattern_ + std::string("|") + lf_attr_t2_pattern_ + std::string("|") + lf_attr_t3_pattern_
														+ std::string("))+$");
	/* complete link format pattern => "^<[\\w-][\\w\\/\\-]+[\\w-]>(;[\\w-]+=([0-9]+|(\"[\\w-]+\")|([\\w-])+))+$" */

	/* default to release attr name and value */
	Attribute LinkFormat::parse_attribute(UString attribute_lf, int flags){

		Attribute result;
		std::string str = attribute_lf.get_string();

		std::vector<std::string> temp_result;
		boost::split(temp_result, str, [](char c){return c == '=';});

		result.set_name(UString(temp_result[0]));
		boost::erase_all(temp_result[1], "\"");
		result.set_value(UString(temp_result[1]));
		result.set_flags(flags);
			
		return result;
	}							

	LinkFormat::LinkFormat(coap::UString link_format, int flags){

		if(check_link_format(link_format)){

            raw_data_ = link_format;

			std::string text = link_format.get_string();
			std::vector<std::string> temp_result;
			boost::split(temp_result, text, [](char c){return c == ';';});

			path_ = UString(temp_result[0].substr(1, (temp_result[0].length()-2)));
			temp_result.erase(temp_result.begin());

			for(auto value : temp_result){
				attribute_list_.push_back(parse_attribute(value,flags));
			}
		}
	}

	bool LinkFormat::is_valid(){

		return check_link_format(raw_data_);
	}

	void LinkFormat::set_path(UString path){
		path_ = path;
	}

	UString LinkFormat::get_path(){
		return path_;
	}

	std::vector<coap::Attribute> LinkFormat::get_attribute_list(){
		return attribute_list_;
	}

	Attribute LinkFormat::get_attribute(UString name){

		Attribute result;
		if(is_attribute_exist(name)){
			for (Attribute value : attribute_list_){
				if(value.get_name().get_ustring().compare(name.get_ustring()) == 0){
					result = value;
					break;
				}
			}
		}
		return result;

	}
	bool LinkFormat::is_attribute_exist(UString name){

		for (Attribute value : attribute_list_){
			if(value.get_name().get_ustring().compare(name.get_ustring()) == 0){
				return true;
			}
		}

		return false;
	}

	bool LinkFormat::check_link_format(coap::UString link_format){ 

		std::regex e (link_format_pattern_);

		if (std::regex_match (link_format.get_string(),e)){
			return true;
		}
		return false;
	}
}