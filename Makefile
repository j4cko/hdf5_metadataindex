CXXFLAGS:= -g -std=c++11 -Wall
HDRS_INDEXER:= attributes.h indexHdf5.h h5helpers.h sqliteHelpers.h
OBJS_INDEXER:= attributes.o indexHdf5.o sqliteHelpers.cc
DEPS_INDEXER:= $(HDRS_INDEXER) $(OBJS_INDEXER)
HDRS_QUERY:= attributes.h indexHdf5.h sqliteHelpers.h conditions.h jsonToValue.h
OBJS_QUERY:= attributes.o indexHdf5.o sqliteHelpers.cc
DEPS_QUERY:= $(HDRS_QUERY) $(OBJS_QUERY)

PROGS:= indexHdf5 queryDb

all: $(PROGS)

%.o: %.cc $(HDRS_INDEXER)
	$(CXX) -c -o $@ $(CXXFLAGS) $<

indexHdf5: indexer.o $(DEPS_INDEXER)
	$(CXX) -o $@ $(CXXFLAGS) -lhdf5 -lsqlite3 $< $(OBJS_INDEXER)

queryDb: queryDb.o $(DEPS_QUERY)
	$(CXX) -o $@ $(CXXFLAGS) -lhdf5 -lsqlite3 -ljsoncpp $< $(OBJS_QUERY)

clean:
	rm -vf *~ *.o $(PROGS)
