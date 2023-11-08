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

#include <iostream>
#include <fstream>
#include "udp_lib.hpp"

int main(int argc, char** argv) {
  std::string in_addr, in_port;
  in_addr = "192.168.83.1";
  in_port = "61480";
  bool log_enabled = false;
  std::string filename = "";
  std::ofstream logfile;

  if (argc > 1) { /* Parse arguments, if there are arguments supplied */
    for (int i = 1; i < argc; i++) {
      if ((argv[i][0] == '-') || (argv[i][0] == '/')) { // switches or options...
        switch (tolower(argv[i][1])) {
        case 'p':
          in_port = argv[++i];
          break;
        case 'a':
          in_addr = argv[++i];
          break;
        case 'f':
          filename = argv[++i];
          log_enabled = true;
          break;
        default: // no match...
          std::cout << argv[i] << " not recognized" << std::endl;
          break;
        }
      } else {
        std::cout << argv[i] << " not recognized" << std::endl;
        break;
      }
    }
  } else {
    std::cout << "Usage: " << argv[0] << " [-f filename] [-a in_addr] [-p in_port]" << std::endl;
    std::cout << "       " << " Set up UDP server listening on in_addr:in_port, with  or without log to file enabled" << std::endl;
    std::cout << "       " << " If not specified, defaults are 192.168.83.1:61480 and log is disabled" << std::endl;
  }

  std::cout << "**************************" << std::endl;
  std::cout << "****    UDP SERVER    ****" << std::endl;
  std::cout << "**************************" << std::endl;
  std::cout << "* in_addr  : " << in_addr   << std::endl;
  std::cout << "* in_port  : " << in_port   << std::endl;
  std::cout << "**************************" << std::endl;

  // Server connection set up
  boost::asio::io_service ioService;
  UDPConnection udp_con(ioService, in_addr, in_port);

  std::array<char, 512> buffer;
  boost::system::error_code err;
  int msg_counter = 0;
  std::string message, client_id, text, hash;
  bool exit = false;

  if (log_enabled) {
    std::cout << "Writing all received data also to file " << filename << std::endl;
    logfile.open(filename, std::ios_base::app);
  }

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
    if (log_enabled)
      logfile << message << std::endl;
  }

  if (log_enabled)
    logfile.close();

  return 0;
}
