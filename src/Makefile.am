#
# replace 'plugin' on the right side by your plugin's name
#
plugin_LTLIBRARIES = libdlvhexplugin-casp.la

#
# replace 'plugin' on the left side as above and
# add all sources of your plugin
#
libdlvhexplugin_casp_la_SOURCES = \
		Utility.cpp \
		GecodeSolver.cpp \
		SimpleParser.cpp \
		LearningProcessor.cpp \
		CaspPlugin.cpp \
		ConsistencyAtom.cpp 

#
# extend compiler flags by CFLAGS of other needed libraries
#
AM_CPPFLAGS = \
	-I$(top_srcdir)/include \
	$(BOOST_CPPFLAGS) \
	$(DLVHEX_CFLAGS)

libdlvhexplugin_casp_la_LDFLAGS = -avoid-version -module


libdlvhexplugin-casp-static.la: $(libdlvhexplugin_casp_la_OBJECTS)
	$(CXXLINK) -avoid-version -module -rpath $(plugindir) $(libdlvhexplugin_casp_la_OBJECTS)

install-static: libdlvhexplugin-casp-static.la
	$(LIBTOOL) --mode=install $(INSTALL) -s libdlvhexplugin-casp-static.la $(DESTDIR)$(plugindir)/libdlvhexplugin-casp-static.la

uninstall-static:
	$(LIBTOOL) --mode=uninstall rm -f $(DESTDIR)$(plugindir)/libdlvhexplugin-casp-static.la
