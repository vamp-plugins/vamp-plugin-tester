
LDFLAGS 	+= -lvamp-hostsdk -ldl
CXXFLAGS	+= -Wall -Wextra

OBJECTS		:= vamp-plugin-tester.o Tester.o Test.o TestStaticData.o TestInputExtremes.o

vamp-plugin-tester:	$(OBJECTS)

clean:
	rm -f $(OBJECTS)

distclean:	clean
	rm -f *~ vamp-plugin-tester

depend:
	makedepend -Y *.cpp *.h

# DO NOT DELETE

Test.o: Test.h
TestInputExtremes.o: TestInputExtremes.h
TestStaticData.o: TestStaticData.h Test.h Tester.h
Tester.o: Tester.h Test.h
vamp-plugin-tester.o: Tester.h Test.h
TestStaticData.o: Test.h Tester.h
Tester.o: Test.h
