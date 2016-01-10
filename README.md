# gabber
Gabber was a Jabber instant messaging client for the Gnome desktop environment. Gabber was one of the earliest major Linux/Gnome applications to be written in C++ using gtkmm and libglade. This code is an archive of the last known commit to Gabber 2 on JabberStudio.

## Jabberoo

This project depends on Jabberoo. Along with many other libraries that are likely quite old.

## Notes

JKM: I've committed this code exactly as I had it in my archive. I've not tested automake, configure, nor build of this code. It was dated 2004-07-09.

## README.cvs

README.cvs contained the following information:

### autogen
autoconf >= 2.50

automake >= 1.5 (?)

gettext  >= 0.11.2

intltool >= 0.21

jabberoo is in its own CVS module on JabberStudio.

### make


### make install
Should you choose to *not* 'make install' (as may often be the case for testing) and you have never installed gabber2 before, then be sure to at least install the GConf files by doing:
        make install-schemas
Otherwise, GConf won't be too happy.

