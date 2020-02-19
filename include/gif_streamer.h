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

#ifndef GIF_STREAMER_GIF_STREAMER_H
#define GIF_STREAMER_GIF_STREAMER_H

#include "gif_header.h"
#include "gif_logical_screen.h"
#include "gif_trailer.h"
#include "gif_image.h"
#include "gif_graphic_control_extension.h"
#include "my_bmp.h"
#include "my_color_alpha.h"
#include <ostream>
#include <string>
#include <set>
#include <map>

namespace gif_streamer
{
    class gif_streamer
    {
      public:

        explicit inline
        gif_streamer( std::ostream & p_stream
                    , unsigned int p_width
                    , unsigned int p_height
                    );

        inline
        ~gif_streamer();

        inline
        void send_bmp( const std::string & p_file_name
                     , unsigned int p_delay_ms = 0
                     );

        inline
        void send_trailer();
      private:

        static inline
        unsigned int convert_to_gif_color( const lib_bmp::my_color_alpha & p_bmp_color
                                         , const std::map<lib_bmp::my_color_alpha,unsigned int> & p_conversion_table
                                         );

        std::ostream & m_stream;

        lib_gif::gif_logical_screen m_logical_screen;

        bool m_first_picture;

        /**
         * Indicate if GIF trailer has been sent or not
         */
        bool m_trailer_streamed;

        std::set<lib_bmp::my_color_alpha> m_global_palette;

        std::map<lib_bmp::my_color_alpha,unsigned int> m_global_conversion_colors;
    };

    //-------------------------------------------------------------------------
    gif_streamer::gif_streamer( std::ostream & p_stream
                              , unsigned int p_width
                              , unsigned int p_height
                              )
    : m_stream(p_stream)
    , m_logical_screen(p_width, p_height)
    , m_first_picture(true)
    , m_trailer_streamed(false)
    {
        if(p_width >= 65536)
        {
            throw quicky_exception::quicky_logic_exception("Width value is greater than 65535 : " + std::to_string(p_width), __LINE__, __FILE__);
        }
        if(p_height >= 65536)
        {
            throw quicky_exception::quicky_logic_exception("Width value is greater than 65535 : " + std::to_string(p_width), __LINE__, __FILE__);
        }
        lib_gif::gif_header l_header;
        l_header.write(p_stream);
    }

    //-------------------------------------------------------------------------
    gif_streamer::~gif_streamer()
    {
        if(!m_trailer_streamed)
        {
            lib_gif::gif_trailer l_trailer;
            l_trailer.write(m_stream);
        }
    }

    //-------------------------------------------------------------------------
    void
    gif_streamer::send_bmp( const std::string & p_file_name
                          , unsigned int p_delay_ms
                          )
    {
        // Check if file exist
        std::ifstream l_file;
        l_file.open(p_file_name.c_str(),std::ifstream::binary);
        if(!l_file.is_open())
        {
            throw quicky_exception::quicky_runtime_exception(R"(Unable to read from file ")" + p_file_name + R"(")", __LINE__, __FILE__);
        }
        l_file.close();

        lib_bmp::my_bmp l_bmp(p_file_name);

        // Check picture resolution
        if(l_bmp.get_width() != m_logical_screen.get_width() || l_bmp.get_height() != m_logical_screen.get_height())
        {
            throw quicky_exception::quicky_logic_exception("Image size ("
                                                          + std::to_string(l_bmp.get_width()) + "*" + std::to_string(l_bmp.get_height())
                                                          + ") doesn't correspond to GIF size ("
                                                          + std::to_string(m_logical_screen.get_width()) + "*" + std::to_string(m_logical_screen.get_height())
                                                          + ")", __LINE__, __FILE__
                                                          );
        }

        // Check number of bits per pixel
        if(8 != l_bmp.get_nb_bits_per_pixel())
        {
            throw quicky_exception::quicky_logic_exception("Only BMP files with 8 bits per pixel are supported : " + std::to_string(l_bmp.get_nb_bits_per_pixel()), __LINE__, __FILE__);
        }

        std::set<lib_bmp::my_color_alpha> l_local_palette;
        std::set<lib_bmp::my_color_alpha> & l_palette = m_first_picture ? m_global_palette : l_local_palette;

        // Fill the Palette
        uint32_t l_palette_size = l_bmp.get_palette().get_size();
        for(unsigned int l_index = 0 ; l_index < l_palette_size ; ++l_index)
        {
            l_palette.insert(l_bmp.get_palette().get_color(l_index));
        }

        if(l_palette.size() > 256)
        {
            throw quicky_exception::quicky_logic_exception(R"(Palette size from file ")" + p_file_name + R"(" is greater than 256 : )" + std::to_string(l_palette.size()),__LINE__,__FILE__);
        }

        // In case of non first picture, check if all colors are part of global palette
        bool l_need_local_palette = false;
        if(!m_first_picture)
        {
            // Check if all colors of picture's palette belong to global palette
            for(unsigned int l_index = 0 ; l_index < l_palette_size && !l_need_local_palette; ++l_index)
            {
                l_need_local_palette = m_global_palette.end() == m_global_palette.find(l_bmp.get_palette().get_color(l_index));
            }
        }

        std::map<lib_bmp::my_color_alpha,unsigned int> l_local_conversion_colors;
        std::map<lib_bmp::my_color_alpha,unsigned int> & l_conversion_colors = (m_first_picture || !l_need_local_palette) ? m_global_conversion_colors : l_local_conversion_colors;

        // Create conversion color to index structure
        if(m_first_picture || l_need_local_palette)
        {
            unsigned int l_index = 0;
            for(auto l_color_iter : l_palette)
            {
                l_conversion_colors.insert(std::map<lib_bmp::my_color_alpha,unsigned int>::value_type(l_color_iter,l_index));
                ++l_index;
            }
        }

        lib_gif::gif_image l_picture{(uint16_t)l_bmp.get_width(), (uint16_t)l_bmp.get_height(), 0, 0, l_need_local_palette, false, false, l_palette_size};
        if(l_need_local_palette)
        {
            // Fill local palette
            unsigned int l_index = 0;
            for(auto l_color_iter : l_palette)
            {
                lib_gif::gif_color l_gif_color(l_color_iter.get_red(),l_color_iter.get_green(),l_color_iter.get_blue());
                l_picture.set_color_in_color_table(l_index, l_gif_color);
                ++l_index;
            }
        }
        else if(m_first_picture)
        {
            // Fill global palette
            unsigned int l_index = 0;
            for(auto l_color_iter : l_palette)
            {
                lib_gif::gif_color l_gif_color(l_color_iter.get_red(),l_color_iter.get_green(),l_color_iter.get_blue());
                m_logical_screen.set_color_in_global_color_table(l_index, l_gif_color);
                ++l_index;
            }

            m_logical_screen.write(m_stream);
            m_first_picture = false;
        }

        if(p_delay_ms)
        {
            lib_gif::gif_graphic_control_extension l_delay_extension(p_delay_ms, false, 0);
            l_delay_extension.write(m_stream);
        }

        // Create gif picture
        for(unsigned int l_y = 0; l_y < l_bmp.get_height(); ++ l_y)
        {
            for(unsigned int l_x = 0; l_x < l_bmp.get_width(); ++l_x)
            {
                l_picture.set_color_index(l_x, l_y, convert_to_gif_color(l_bmp.get_pixel_color(l_x, l_y), l_conversion_colors));
            }
        }
        l_picture.write(m_stream);

    }

    //-------------------------------------------------------------------------
    void
    gif_streamer::send_trailer()
    {
        lib_gif::gif_trailer l_trailer;
        l_trailer.write(m_stream);
        m_trailer_streamed = true;
    }

    //-------------------------------------------------------------------------
    unsigned int
    gif_streamer::convert_to_gif_color( const lib_bmp::my_color_alpha & p_bmp_color
                                      , const std::map<lib_bmp::my_color_alpha, unsigned int> & p_conversion_table
                                      )
    {
        auto l_color_iter = p_conversion_table.find(p_bmp_color);
        if(p_conversion_table.end() != l_color_iter)
        {
            return l_color_iter->second;
        }
        std::stringstream l_color_stream;
        l_color_stream << p_bmp_color;
        throw quicky_exception::quicky_logic_exception("BMP color \"" + l_color_stream.str() + " not found insigned GIF palette",__LINE__,__FILE__);

    }

}
#endif //GIF_STREAMER_GIF_STREAMER_H
