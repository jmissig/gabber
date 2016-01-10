/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 * 
 *  Gabber 2
 *  Based on Gabber, Copyright (c) 1999-2002 Dave Smith & Julian Missig
 *  Copyright (c) 2002 Julian Missig
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include "GabberApp.hh"
#include "PlatformFuncs.hh"

#include <libintl.h>
#ifndef WIN32
#  include <gconfmm/init.h>
#endif

using namespace Gabber;

#ifdef WIN32
void win32_log_func(const gchar* domain, GLogLevelFlags lvl, const gchar* message, gpointer user_data)
{
}
#endif

int main (int argc, char** argv)
{
#ifndef WIN32
    bindtextdomain (GETTEXT_PACKAGE, LOCALEDIR);
    bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
    textdomain (GETTEXT_PACKAGE);
    // Start up our trusty friend GConf
	Gnome::Conf::init();
#else
    g_log_set_handler(NULL, 
        (GLogLevelFlags)(G_LOG_LEVEL_MASK), win32_log_func, NULL);
    g_log_set_handler("GLib", 
        (GLogLevelFlags)(G_LOG_LEVEL_MASK), win32_log_func, NULL);
    g_log_set_handler("Gtk", 
        (GLogLevelFlags)(G_LOG_LEVEL_MASK), win32_log_func, NULL);
#endif

    StartupPlatform();

    GabberApp *app;

    try
    {
        app = new GabberApp(argc, argv);
        app->run();
	delete app;
    }
    catch(const Glib::Error& ex)
    {
        cout << ex.what() << endl;
    }

    ShutdownPlatform();

    return 0;
}


/* -*- mode: C++ c-basic-offset: 4  -*-
 * main.C - source file for main function
 * Copyright (c) 1999 Joe Yandle <jwy@divisionbyzero.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * 
 */

// #include "MainWin.hh"
// #include "Config.hh"

// #ifdef HAVE_CONFIG_H
// #include <config.h>
// #endif

// #include <gnome--/main.h>

// #include <exception>

// #include <sys/types.h>
// #include <sys/stat.h>
// #include <fcntl.h>

// #include <popt.h>

// char* popt_mailto_url=0;

// struct poptOption opts[] = {
//     { 
//         "mailto",
//         'm', 
//         POPT_ARG_STRING, 
//         popt_mailto_url,
//         0,
//         "send mail",
//         "mailto:foo@bar.com"
//     },

//     POPT_AUTOHELP
//     { NULL, 0, 0, NULL, 0 }
// };


// const std::string MAILTO_OPT = "--mailto";

// bool is_running();
// bool is_mailto(int argc, char** argv);
// std::string get_mailto(int argc, char** argv);
// void write_pipe(std::string mailto);
// std::string get_mailto_file();


// int main(int argc, char** argv) {
//     // do popt init stuff;
//     poptContext ctx;

//     if(getenv("GTKMAIL_MAIN_DEBUG")) {
//         std::cout << "gtkmail::main()" << std::endl;
//     }
//     try {
//         if(is_mailto(argc,argv) && is_running()) {
//             if(getenv("GTKMAIL_MAIN_DEBUG")) {
//                 std::cout << "\tsend the mailto command through the pipe" << std::endl;
//             }
//             write_pipe(get_mailto(argc,argv));
//         }
//         else {
//             if(is_mailto(argc,argv)) {
//                 if(getenv("GTKMAIL_MAIN_DEBUG")) {
//                     std::cout << "\trun normally, but send the mailto" << std::endl;
//                 }
//                 Gnome::Main main(PACKAGE,VERSION,argc,argv,opts,0,&ctx);
//                 gtkmail::MainWin win;

//                 write_pipe(get_mailto(argc,argv));

//                 main.run();
//             }
//             else {
//                 if(getenv("GTKMAIL_MAIN_DEBUG")) {
//                     std::cout << "\trun normally" << std::endl;
//                 }
//                 //Gnome::Main main(PACKAGE,VERSION,argc,argv);
//                 Gnome::Main main(PACKAGE,VERSION,argc,argv,opts,0,&ctx);
//                 gtkmail::MainWin win;
//                 main.run();
//             }
//         }
//     }
//     catch(std::exception& e) {
//         std::cout << e.what() << std::endl;
//     }
//     catch(...) {
//         std::cout << "Caught unknown exception, exiting\n";
//     }
// }

// bool is_running() {
//     if(getenv("GTKMAIL_MAIN_DEBUG")) {
//         std::cout << "gtkmail::is_running()" << std::endl;
//     }
//     int fd = -2;

//     fd = open(get_mailto_file().c_str(), O_WRONLY|O_NONBLOCK);
//     if(fd > 0) {
//         if(getenv("GTKMAIL_MAIN_DEBUG")) {
//             std::cout << "\tfd = " << fd << ", returning true" << std::endl;
//         }
//         close(fd);
//         return true;
//     }
//     else {
//         if(getenv("GTKMAIL_MAIN_DEBUG")) {
//             std::cout << "\tfd = " << fd << ", returning false" << std::endl;
//         }
//         return false;
//     }
// }

// bool is_mailto(int argc, char** argv) {
//     if(getenv("GTKMAIL_MAIN_DEBUG")) {
//         std::cout << "gtkmail::is_mailto()" << std::endl;
//     }
//     for(int i=1;i<argc;i++) {
//         if(MAILTO_OPT == argv[i]) {
//             if(getenv("GTKMAIL_MAIN_DEBUG")) {
//                 std::cout << "\treturning true" << std::endl;
//             }
//             return true;
//         }
//     }
//     if(getenv("GTKMAIL_MAIN_DEBUG")) {
//         std::cout << "\treturning false" << std::endl;
//     }
//     return false;
// }

// std::string get_mailto(int argc, char** argv) {
//     if(getenv("GTKMAIL_MAIN_DEBUG")) {
//         std::cout << "gtkmail::get_mailto()" << std::endl;
//     }
//     std::string ret;
//     for(int i=1;i<argc;i++) {
//         if(MAILTO_OPT == argv[i] && (i+1)<argc) {
//             ret = argv[i+1];
//         }
//     }
//     if(getenv("GTKMAIL_MAIN_DEBUG")) {
//         std::cout << "\treturning '" << ret << "'" << std::endl;
//     }
//     return ret;
// }

// void write_pipe(std::string mailto) {
//     std::ofstream opipe(get_mailto_file().c_str());
//     opipe.write(mailto.data(),mailto.length());
//     opipe.flush();
//     opipe.close();
// }

// std::string get_mailto_file() {
//     return std::string(getenv("HOME"))+"/.gtkmail/mailto";
// }
