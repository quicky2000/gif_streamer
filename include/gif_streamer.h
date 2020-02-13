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
#include <ostream>

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

      private:

        std::ostream & m_stream;
    };

    //-------------------------------------------------------------------------
    gif_streamer::gif_streamer( std::ostream & p_stream
                              , unsigned int p_width
                              , unsigned int p_height
                              )
    : m_stream(p_stream)
    {
        lib_gif::gif_header l_header;
        l_header.write(p_stream);
        if(p_width >= 65536)
        {
            throw quicky_exception::quicky_logic_exception("Width value is greater than 65535 : " + std::to_string(p_width), __LINE__, __FILE__);
        }
        if(p_height >= 65536)
        {
            throw quicky_exception::quicky_logic_exception("Width value is greater than 65535 : " + std::to_string(p_width), __LINE__, __FILE__);
        }
        lib_gif::gif_logical_screen l_logical_screen(p_width, p_height);
        l_logical_screen.write(p_stream);
    }

    //-------------------------------------------------------------------------
    gif_streamer::~gif_streamer()
    {
        lib_gif::gif_trailer l_trailer;
        l_trailer.write(m_stream);
    }

}
#endif //GIF_STREAMER_GIF_STREAMER_H
