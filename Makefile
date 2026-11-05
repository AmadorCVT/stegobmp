CC      = gcc
CFLAGS  = -std=c11 -Wall -Wextra -Wpedantic -g -Iinclude
LDFLAGS =
LDLIBS  = -lcrypto

# `make release`: optimized build without debug info (see the rule below).
RELEASE_CFLAGS = -std=c11 -Wall -Wextra -Wpedantic -O2 -DNDEBUG -Iinclude

TARGET    = stegobmp
SRC_DIR   = src
BUILD_DIR = build

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)
DEPS = $(OBJS:.o=.d)

.PHONY: all clean release

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# -MMD -MP generate .d files so changes in headers trigger a rebuild
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -MMD -MP -c $< -o $@

# Rebuilds everything from scratch with optimizations and without -g.
release:
	$(MAKE) clean
	$(MAKE) CFLAGS="$(RELEASE_CFLAGS)" all

$(BUILD_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET)

-include $(DEPS)
