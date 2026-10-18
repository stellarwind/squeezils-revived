CXX = g++
CXXFLAGS = -Wall -O2 -fconserve-space

QvLib.o: QvLib.c++
	$(CXX) $(CXXFLAGS) -I ../include -c QvLib.c++

clean:
	-$(RM) QvLib.o

dist: clean
	cd ../.. && (find QvLib-1.0 -type f -print | tar -cvf - -T- | gzip -9 > qvlib10.tgz)


