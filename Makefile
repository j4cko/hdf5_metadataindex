CXXFLAGS:= -g -std=c++11 -Wall
HDRS:= attributes.h indexHdf5.h postselection.h h5helpers.h sqliteHelpers.h conditions.h
HDRS_JSON:= jsonToValue.h
OBJS:= sqliteHelpers.o indexHdf5.o postselection.o attributes.o h5helpers.o
DEPS:= $(HDRS) $(OBJS)
DEPS_QUERY:= $(HDRS) $(HDRS_JSON) $(OBJS)

PROGS:= indexHdf5 queryDb

all: $(PROGS)

%.o: %.cc $(HDRS)
	$(CXX) -c -o $@ $(CXXFLAGS) $<

indexHdf5: indexer.o $(DEPS)
	$(CXX) -o $@ $(CXXFLAGS) -lhdf5 -lsqlite3 $< $(OBJS)

queryDb: queryDb.o $(DEPS_QUERY)
	$(CXX) -o $@ $(CXXFLAGS) -lhdf5 -lsqlite3 -ljsoncpp $< $(OBJS)

clean:
	rm -vf *~ *.o $(PROGS)
