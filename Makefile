CC := gcc
CFLAGS := -g
CPPFLAGS := -MP -MMD
LDFLAGS := -lm -lSDL2 -lSDL2_image

BUILD_DIR := ./build
SRC_DIR := ./src

TARGET_EXEC := kartgame

SRCS := $(basename $(notdir $(wildcard $(SRC_DIR)/*.c)))
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DEPS := $(OBJS:.o=.d)


$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)

-include $(DEPS)