SRCDIR = src
STRUCTURE = $(shell cd $(SRCDIR) && find . -type d)

CXX ?= g++
CXXFLAGS ?= -g

BINARYDIR = bin
OBJECTDIR = $(BINARYDIR)/obj

TARGET = $(BINARYDIR)/jdmb

LIBS += -lpthread

# event/fd
_OBJECTS += event/fd/epoll_fd.o
_HEADERS += event/fd/epoll_fd.h

_OBJECTS += event/fd/fd.o
_HEADERS += event/fd/fd.h

_HEADERS += event/fd/io_handle.h

# event
_OBJECTS += event/event_loop.o
_HEADERS += event/event_loop.h

_OBJECTS += event/timer.o
_HEADERS += event/timer.h

# log
_OBJECTS += log/logger_context.o
_HEADERS += log/logger_context.h

_HEADERS += log/logger_type.h

_OBJECTS += log/logger.o
_HEADERS += log/logger.h

# net/tcp
_OBJECTS += net/tcp/client.o
_HEADERS += net/tcp/client.h

_OBJECTS += net/tcp/server.o
_HEADERS += net/tcp/server.h

# util
_HEADERS += util/result.h

_OBJECTS += util/str.o
_HEADERS += util/str.h

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