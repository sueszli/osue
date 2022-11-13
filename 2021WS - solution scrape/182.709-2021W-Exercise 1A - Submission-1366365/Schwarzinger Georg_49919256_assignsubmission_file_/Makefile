# Georg Schwarzinger
# 12024747
# ispalindrom_td

CC := gcc
SRCDIR := src
BUILDDIR := build
BIN := bin
TARGET := $(BIN)/main.out
SRCEXT := c
SOURCES := $(shell find $(SRCDIR) -type f -name *.$(SRCEXT))
OBJECTS := $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.$(SRCEXT)=.o))
CFLAGS := -std=c99 -pedantic -Wall -D_DEFAULT_SOURCE -D_SVID_SOURCE -D_POSIX_C_SOURCE=200809L 

# LIB := #-pthread
# INC := -I include

all: $(TARGET)
$(TARGET): $(OBJECTS) | $(BIN)
	  @echo " Linking..."
	    @echo " $(CC) $^ -o $(TARGET) $(LIB)"; $(CC) $^ -o $(TARGET) $(LIB)

$(BUILDDIR)/%.o: $(SRCDIR)/%.$(SRCEXT)
	  @mkdir -p $(BUILDDIR)
	    @echo " $(CC) $(CFLAGS) $(INC) -c -o $@ $<"; $(CC) $(CFLAGS) -c -o $@ $< 

$(BIN):
	mkdir $@

.PHONY: clean
clean:
	  @echo " Cleaning..."; 
	    @echo " $(RM) -r $(BUILDDIR) $(TARGET)"; $(RM) -r $(BUILDDIR) $(TARGET)
