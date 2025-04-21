SRCDIR = src
STRUCTURE = $(shell cd $(SRCDIR) && find . -type d)

CXX ?= g++
CXXFLAGS ?= -g

BINARYDIR = bin
OBJECTDIR = $(BINARYDIR)/obj

TARGET = $(BINARYDIR)/jdmb

LIBS += -lpthread

# log
_OBJECTS += log/logger_context.o
_HEADERS += log/logger_context.h

_HEADERS += log/logger_type.h

_OBJECTS += log/logger.o
_HEADERS += log/logger.h

# util
_HEADERS += util/result.h

# root
_OBJECTS += config.o
_HEADERS += config.h

_OBJECTS += jdmb.o
_HEADERS += jdmb.h

_OBJECTS += main.o

OBJECTS = $(patsubst %,$(OBJECTDIR)/%,$(_OBJECTS))
HEADERS = $(patsubst %,$(SRCDIR)/%,$(_HEADERS))

$(OBJECTDIR)/%.o: $(SRCDIR)/%.cpp $(HEADERS) | $(OBJECTDIR)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

$(TARGET): $(OBJECTS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

$(OBJECTDIR):
	mkdir -p $(OBJECTDIR)
	mkdir -p $(addprefix $(OBJECTDIR)/,$(STRUCTURE))

relink: $(OBJECTS)
	$(CXX) -o $(TARGET) $^ $(CXXFLAGS) $(LIBS)

clean:
	rm -rf bin

.PHONY: clean