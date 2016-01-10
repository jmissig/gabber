#ifndef INCL_ENVIRONMENT
#define INCL_ENVIRONMENT
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <glibmm/ustring.h>
#include <glib/gfileutils.h>
#include <glib/gmessages.h>

namespace Gabber
{

/**
 * Information that has been set by either the compilation or system environment
 */
class Environment {
public:
    const Glib::ustring package;
    const Glib::ustring version;

    // Locate files (e.g., pixmaps, .glade files, etc.)
    static Glib::ustring findPath (Glib::ustring local_relative_dir,
            Glib::ustring systemwide_dir, Glib::ustring filename) {
        Glib::ustring temp_path; 
        if (g_file_test(("./" + local_relative_dir + filename).c_str(), 
                    G_FILE_TEST_EXISTS))
            temp_path = "./" + local_relative_dir;
        else if (g_file_test(("../" + local_relative_dir + filename).c_str(), 
                    G_FILE_TEST_EXISTS))
            temp_path = "../" + local_relative_dir;
        else if (g_file_test(("./" + filename).c_str(), 
                    G_FILE_TEST_EXISTS))
            temp_path = "./";
        else if (g_file_test((systemwide_dir + filename).c_str(), 
                    G_FILE_TEST_EXISTS))
            temp_path = systemwide_dir;
        else {
            g_error(Glib::ustring("Could not find " + filename + "!").c_str());
            exit(-1);
        }
        return temp_path;
    }
    
    Environment() :
        package    (PACKAGE),
        version    (VERSION)
    {}
};

static const Environment ENV_VARS; // environment variables

}; // namespace Gabber

#endif
