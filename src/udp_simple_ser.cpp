/* Copyright 2016-2023 - Stefano Sinigardi, Alessandro Fabbri */

/***************************************************************************
This file is part of udp-simple-server.

udp-simple-server is free software : you can redistribute it and / or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

udp-simple-server is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with udp-simple-server. If not, see <http://www.gnu.org/licenses/>.
***************************************************************************/

#include "udp_lib.hpp"

int main(int argc, char** argv) {
  std::string in_addr, in_port, out_addr, out_port;

  // Command line handling
  if (argc == 5) {
    in_addr = argv[1];
    in_port = argv[2];
    out_addr = argv[3];
    out_port = argv[4];
  }
  else if (argc == 2 && std::string(argv[1]) == "def") {
    in_addr = "127.0.0.1";
    in_port = "11111";
    out_addr = "127.0.0.1";
    out_port = "22222";
  }
  else {
    std::cout << "Usage: " << argv[0] << " in_addr in_port out_addr out_port" << std::endl;
    std::cout << "       " << " Set up UDP server listening on in_addr:in_port and sending ACK message to out_addr:out_port" << std::endl;
    std::cout << "Usage: " << " def" << std::endl;
    std::cout << "       " << " Set up default UDP server (listen 127.0.0.1:11111, send ACK 127.0.0.1:22222)" << std::endl;
    exit(1);
  }

  std::cout << "**************************" << std::endl;
  std::cout << "**** BOOST UDP SERVER ****" << std::endl;
  std::cout << "**************************" << std::endl;
  std::cout << "* in_addr  : " << in_addr   << std::endl;
  std::cout << "* in_port  : " << in_port   << std::endl;
  std::cout << "* out_addr : " << out_addr  << std::endl;
  std::cout << "* out_port : " << out_port  << std::endl;
  std::cout << "**************************" << std::endl;

  // Server connection set up
  boost::asio::io_service ioService;
  UDPConnection udp_con(ioService, in_addr, in_port, out_addr, out_port);

  std::array<char, 512> buffer;
  boost::system::error_code err;
  int msg_counter = 0;
  std::string message, client_id, text, hash;
  bool exit = false;

  while (exit == false) {
#ifdef _WIN32
    if (GetAsyncKeyState(VK_ESCAPE))
#elif __APPLE__
    if (getc_unlocked(stdin) == 'q')
#else
    if (fgetc_unlocked(stdin) == 'q') // da implementare con fgetc_unlocked, questo e' solo un tentativo alla cieca, non so come funzioni!
#endif
    {
      exit = true;
    }
    // receive message from client
    udp_con.recv(boost::asio::buffer(&buffer[0], buffer.size()), err);
    message = std::string(buffer.data(), udp_con.len_recv);
    std::cout << ++msg_counter << ": " << message << std::endl;
  }

  return 0;
}
