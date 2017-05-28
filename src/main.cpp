// Author: Dmitry Kukovinets (d1021976@gmail.com), 28.05.2017, 20:00

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <regex>
#include <algorithm>
#include <iterator>
#include <exception>
#include <stdexcept>

#include <boost/program_options.hpp>


int
main(int argc, char **argv)
{
	namespace po = boost::program_options;
	
	const auto print_help =
		[&](std::ostream &stream = std::cout)
		{
			stream
				<< "Usage:" << std::endl
				<< "    " << argv[0] << " [OPTIONS] REGEX [FILE ...]" << std::endl;
		};
	
	
	std::string regex;
	std::vector<std::string> input_files;
	
	try {
		// Declare the supported options.
		po::options_description options_description("Allowed options");
		options_description.add_options()
			(
				"help,h",
				"Print help message and exit"
			)
			
			(
				"escape,e",
				"Process regex as string with escaped backslashes (just copy regex from .cpp file)"
			)
			
			(
				"regex,r", po::value<std::string>(&regex),
				"Set regex"
			)
			
			(
				"input-file,i", po::value<std::vector<std::string>>(&input_files),
				"Set input file"
			)
		;
		
		po::positional_options_description positional_options;
		positional_options.add("regex", 1);
		positional_options.add("input-file", -1);
		
		
		po::variables_map vm;
		
		po::store(
			po::command_line_parser(argc, argv).options(options_description).positional(positional_options).run(),
			vm
		);
		
		po::notify(vm);
		
		if (vm.count("help")) {
			print_help();
			std::cout << options_description;
			return 0;
		}
		
		if (input_files.empty())
			throw std::logic_error{"No input files given"};
		
		if (vm.count("escape")) {
			std::string tmp;
			tmp.reserve(regex.size());
			
			std::regex_replace(
				std::back_inserter(tmp),
				regex.begin(), regex.end(),
				std::regex{"\\\\\\\\", std::regex::optimize},
				"\\"
			);
			
			regex = std::move(tmp);
		}
		
		
		const std::regex r{regex, std::regex::optimize};
		std::smatch m;
		std::string str;
		
		for (const auto &input_file: input_files) {
			std::ifstream fin;
			fin.exceptions(std::ifstream::failbit);
			try {
				fin.open(input_file);
			} catch (const std::exception &e) {
				std::cerr << "Error with file: \"" << input_file << "\": " << e.what() << '.' << std::endl;
				continue;
			}
			fin.exceptions(std::ifstream::goodbit);
			
			
			std::cout << input_file << ':' << std::endl;
			
			std::size_t line_number = 1;
			while (std::getline(fin, str)) {
				if (std::regex_match(str, m, r)) {
					std::cout << "    [" << line_number << "] match (groups: " << m.size() << "):" << std::endl;
					
					auto max_group_index = m.size();
					if (max_group_index > 0)
						--max_group_index;
					const auto group_index_width = std::to_string(max_group_index).size();
					
					const auto str_index_width = std::to_string(str.size()).size();
					
					std::size_t group_index = 0;
					for (const auto &g: m) {
						std::cout
							<< "        ["
							<< std::setw(group_index_width) << std::right << group_index
							<< std::setw(0) << std::left << "; pos: "
							<< std::setw(str_index_width) << std::right << std::distance(str.cbegin(), g.first)
							<< std::setw(0) << std::left << "; len: "
							<< std::setw(str_index_width) << std::right << g.length()
							<< "] \"" << g.str() << '\"' << std::endl;
						
						++group_index;
					}
				} else {
					std::cout << "    [" << line_number << "] --" << std::endl;
				}
				
				++line_number;
			}
		}
	} catch (const std::exception &e) {
		std::cerr << "Error: " << e.what() << '.' << std::endl;
		return 1;
	}
	
	return 0;
}
