
ARCHFLAGS	?=

LDFLAGS 	+= $(ARCHFLAGS) -ldl -pthread
CXXFLAGS	+= $(ARCHFLAGS) -std=c++98 -g -Wall -Wextra -Ivamp-plugin-sdk -pthread

# We include the Vamp Host SDK sources in the build here, so that we
# can build the entire thing with debug symbols even though the SDK
# would not normally have them

VAMP_SRCDIR	:= vamp-plugin-sdk/src/vamp-hostsdk

VAMP_OBJECTS	:= \
	$(VAMP_SRCDIR)/PluginHostAdapter.o \
	$(VAMP_SRCDIR)/RealTime.o \
	$(VAMP_SRCDIR)/PluginBufferingAdapter.o \
	$(VAMP_SRCDIR)/PluginChannelAdapter.o \
	$(VAMP_SRCDIR)/PluginInputDomainAdapter.o \
	$(VAMP_SRCDIR)/PluginLoader.o \
	$(VAMP_SRCDIR)/PluginSummarisingAdapter.o \
	$(VAMP_SRCDIR)/PluginWrapper.o \
	$(VAMP_SRCDIR)/Files.o \
	$(VAMP_SRCDIR)/acsymbols.o

OBJECTS		:= \
	$(VAMP_OBJECTS) \
	vamp-plugin-tester.o \
	Tester.o \
	Test.o \
	TestStaticData.o \
	TestInputExtremes.o \
	TestMultipleRuns.o \
	TestOutputs.o \
	TestDefaults.o \
	TestInitialise.o

vamp-plugin-tester:	vamp-plugin-sdk/README $(OBJECTS) $(VAMP_OBJECTS)
	$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)

vamp-plugin-sdk/README:
	./vext install

clean:
	rm -f $(OBJECTS) $(VAMP_OBJECTS)

distclean:	clean
	rm -f *~ vamp-plugin-tester

depend:
	makedepend -Y *.cpp *.h $(VAMP_SRCDIR)/*.cpp

# DO NOT DELETE

Test.o: Test.h
TestDefaults.o: TestDefaults.h Test.h Tester.h
Tester.o: Tester.h Test.h
TestInitialise.o: TestInitialise.h Test.h Tester.h
TestInputExtremes.o: TestInputExtremes.h Test.h Tester.h
TestMultipleRuns.o: TestMultipleRuns.h Test.h Tester.h
TestOutputs.o: TestOutputs.h Test.h Tester.h
TestStaticData.o: TestStaticData.h Test.h Tester.h
vamp-plugin-tester.o: Tester.h Test.h
TestDefaults.o: Test.h Tester.h
Tester.o: Test.h
TestInitialise.o: Test.h Tester.h
TestInputExtremes.o: Test.h Tester.h
TestMultipleRuns.o: Test.h Tester.h
TestOutputs.o: Test.h Tester.h
TestStaticData.o: Test.h Tester.h
vamp-plugin-sdk/src/vamp-hostsdk/PluginInputDomainAdapter.o: vamp-plugin-sdk/src/vamp-hostsdk/Window.h
vamp-plugin-sdk/src/vamp-hostsdk/PluginInputDomainAdapter.o: vamp-plugin-sdk/src/vamp-sdk/FFTimpl.cpp
vamp-plugin-sdk/src/vamp-hostsdk/RealTime.o: vamp-plugin-sdk/src/vamp-sdk/RealTime.cpp
