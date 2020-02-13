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

#include "quicky_exception.h"
#include "gif_streamer.h"
#include <iostream>

#ifdef GIF_STREAMER_SELF_TEST

int main(int p_argc, char ** p_argv)
{
    try
    {
        std::ofstream l_output_file;
        std::string l_file_name{"stream.gif"};
        l_output_file.open(l_file_name, std::ios::binary);
        if(!l_output_file.is_open())
        {
            throw quicky_exception::quicky_runtime_exception("Unable to create file \"" + l_file_name + "\"", __LINE__, __FILE__);
        }

        {
            gif_streamer::gif_streamer l_stream(l_output_file, 640, 480);
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