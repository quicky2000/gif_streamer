/*    This file is part of gif_streamer
      Copyright (C) 2020  Julien Thevenon ( julien_thevenon at yahoo.fr )

      This program is free software: you can redistribute it and/or modify
      it under the terms of the GNU General Public License as published by
      the Free Software Foundation, either version 3 of the License, or
      (at your option) any later version.

      This program is distributed in the hope that it will be useful,
      but WITHOUT ANY WARRANTY; without even the implied warranty of
      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
      GNU General Public License for more details.

      You should have received a copy of the GNU General Public License
      along with this program.  If not, see <http://www.gnu.org/licenses/>
*/


#ifdef GIF_STREAMER_SELF_TEST
#include "quicky_exception.h"
#include "gif_streamer.h"
#include "parameter_manager.h"
#include <iostream>

int main(int p_argc, char ** p_argv)
{
    try
    {
        // Defining application command line parameters
        parameter_manager::parameter_manager l_param_manager("gif_streamer.exe","--",1);
        parameter_manager::parameter_if l_name__pattern_parameter("name-pattern", false);
        l_param_manager.add(l_name__pattern_parameter);
        parameter_manager::parameter_if l_delay_ms_parameter("delay-ms",true);
        l_param_manager.add(l_delay_ms_parameter);
        parameter_manager::parameter_if l_start_index_parameter("start-index",true);
        l_param_manager.add(l_start_index_parameter);
        parameter_manager::parameter_if l_nb_files_parameter("nb-files",true);
        l_param_manager.add(l_nb_files_parameter);

        // Treating parameters
        l_param_manager.treat_parameters(p_argc, p_argv);

        std::string l_name_pattern = l_name__pattern_parameter.get_value<std::string>();
        unsigned int l_delay = l_delay_ms_parameter.value_set() ? std::stoul(l_delay_ms_parameter.get_value<std::string>()) : 500;
        unsigned int l_start_index = l_start_index_parameter.value_set() ? std::stoul(l_start_index_parameter.get_value<std::string>()) : 0;
        unsigned int l_nb_files = l_nb_files_parameter.value_set() ? std::stoul(l_nb_files_parameter.get_value<std::string>()) : 0;

        size_t l_pattern_pos = l_name_pattern.find('#');
        if(std::string::npos == l_pattern_pos)
        {
            throw quicky_exception::quicky_logic_exception("Failed to find '#' character in file pattern \"" + l_name_pattern + "\"",__LINE__,__FILE__);
        }

        std::vector<std::string> l_file_list;

        // Search files matching pattern
        bool l_continu = true;
        unsigned int l_file_index = l_start_index;
        do
        {
            std::string l_file_name = l_name_pattern;
            l_file_name.replace(l_pattern_pos,1,std::to_string(l_file_index));

            // Check if file exist
            std::ifstream l_file;
            l_file.open(l_file_name.c_str(),std::ifstream::binary);
            if(l_file.is_open())
            {
                l_file.close();
                ++l_file_index;
                l_file_list.emplace_back(l_file_name);
            }
            else
            {
                l_continu = false;
            }
            if(l_nb_files == l_file_index)
            {
                l_continu = false;
            }
        }
        while(l_continu);

        if(l_file_list.empty())
        {
            throw quicky_exception::quicky_logic_exception("No file found matching pattern \"" + l_name_pattern + "\"",__LINE__,__FILE__);
        }

        unsigned int l_width;
        unsigned int l_height;
        {
            lib_bmp::my_bmp l_bmp(l_file_list.front());
            l_width = l_bmp.get_width();
            l_height = l_bmp.get_height();
        }

        std::ofstream l_output_file;
        std::string l_gif_file_name{"stream.gif"};
        l_output_file.open(l_gif_file_name, std::ios::binary);
        if(!l_output_file.is_open())
        {
            throw quicky_exception::quicky_runtime_exception("Unable to create file \"" + l_gif_file_name + "\"", __LINE__, __FILE__);
        }

        {
            gif_streamer::gif_streamer l_stream(l_output_file, l_width, l_height);
            for(auto & l_file_iter:l_file_list)
            {
                std::cout << R"(Add file ")" + l_file_iter + R"(")" << std::endl;
                l_stream.send_bmp(l_file_iter, l_delay);
            }

        }
        l_output_file.close();
    }
    catch(const quicky_exception::quicky_logic_exception & e)
    {
        std::cerr << "ERROR : " << e.what() << " from " << e.get_file() << ":" << e.get_line() << std::endl;
    }
    catch(const quicky_exception::quicky_runtime_exception & e)
    {
        std::cerr << "ERROR : " << e.what() << " from " << e.get_file() << ":" << e.get_line() << std::endl;
    }
}

#endif // GIF_STREAMER_SELF_TEST
// EOF