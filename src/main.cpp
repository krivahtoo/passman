/*
 * Copyright (c) 2021 Noah Too
 * 
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 * 
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */


#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>

#include <sqlcipher/sqlite3.h>
#include <sqlite_orm/sqlite_orm.h>
#include <cxxopts.hpp>

#ifdef WIN32
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

#include "database.h"

#define VERSION "0.1.0"

void setEcho(bool enable = true)
{
#ifdef WIN32
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE); 
    DWORD mode;
    GetConsoleMode(hStdin, &mode);

    if( !enable )
        mode &= ~ENABLE_ECHO_INPUT;
    else
        mode |= ENABLE_ECHO_INPUT;

    SetConsoleMode(hStdin, mode );

#else
    struct termios tty;
    tcgetattr(STDIN_FILENO, &tty);
    if( !enable )
        tty.c_lflag &= ~ECHO;
    else
        tty.c_lflag |= ECHO;

    (void) tcsetattr(STDIN_FILENO, TCSANOW, &tty);
#endif
}

int main(int argc, char* argv[]) {
   using namespace sqlite_orm;
   std::string file;
   std::string key;
   int option;
   bool new_db = true;
   cxxopts::Options opts("passman", "A simple cross platform CLI password manager");

   opts.add_options("Generic")
      ("v,version", "Print version")
      ("h,help", "Display help")
   ;

   opts.add_options("Config")
      ("f,file", "Database file to use", cxxopts::value<std::string>(), "<file>")
      ("p,password", "Database password to use", cxxopts::value<std::string>(), "<password>")
   ;

   opts.custom_help("[OPTIONS].. [file]");

   auto result = opts.parse(argc, argv);

   if (result.count("help")) {
      std::cout << opts.help() << std::endl;
      exit(0);
   }

   if (result.count("version")) {
      std::cout << "Version: " << VERSION << std::endl;
      exit(0);
   }

   if (result.count("file")) {
      file = result["file"].as<std::string>();
   } else {
      file = DB_FILE;
   }

   if (result.count("password")) {
      key = result["password"].as<std::string>();
   } else {
      std::cout << "Enter your database password:";
      setEcho(false);
      std::cin >> key;
      setEcho(true);
      std::cout << std::endl;
   }

   auto storage = make_storage(file,
      make_table("passwords",
         make_column("id", &Password::id, autoincrement(), primary_key()),
         make_column("pass", &Password::pass),
         make_column("site", &Password::site)
      )
   );

   storage.on_open = [&](sqlite3* db){
      sqlite3_key(db, key.c_str(), key.size());
   };
   if (storage.table_exists("passwords")) {
      new_db = false;
   }

   storage.sync_schema();

   std::cout << " 1. Add password" << std::endl;
   if (!new_db) {
      std::cout << " 2. Get password" << std::endl;
   }
   std::cout << " 3. Quit" << std::endl;
   std::cout << "What do you want to do:";
   std::cin >> option;
   
   switch (option)
   {
   case 1:
      {
         std::string site_name, site_pass;
         std::cout << "Enter site name:";
         std::cin >> site_name;
         std::cout << "Enter password:";
         setEcho(false);
         std::cin >> site_pass;
         setEcho(true);
         Password pass{-1, site_pass, site_name};
         auto pass_id = storage.insert(pass);
         std::cout << std::endl << "Password saved succefully id:" << pass_id << std::endl;

      }
      std::cout << "Bye" << std::endl;
      break;

   case 2:
      if (!new_db) {
         for (auto &pass : storage.iterate<Password>()) {
            std::cout << " " << pass.id << ". " << pass.site << std::endl;
         }
         std::cout << "Enter site:";
         std::cin >> option;
         if(auto pass = storage.get_pointer<Password>(option)){
            std::cout << "pass: " << pass->pass << std::endl;
         }else{
            std::cout << "no site with id " << option << std::endl;
            exit(1);
         }
      } else {
         std::cout << "Invalid option" << std::endl;
         exit(1);
      }
      break;

   case 3:
      std::cout << "Bye" << std::endl;
      exit(0);
      break;
   
   default:
      std::cout << "Invalid option" << std::endl;
      break;
   }
   return 0;
}
