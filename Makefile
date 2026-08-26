CXXFLAGS = "-O2 -Wall -Wextra"
BUILD_DIR = build

all: $(BUILD_DIR)/CMakeCache.txt
	cmake --build $(BUILD_DIR)
	ln -sf $(BUILD_DIR)/cli/crykit crykit

$(BUILD_DIR)/CMakeCache.txt:
	cmake -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=ON\
		-DCMAKE_CXX_FLAGS=$(CXXFLAGS)
	ln -sf $(BUILD_DIR)/compile_commands.json compile_commands.json

clean:
	rm -rf $(BUILD_DIR)
	rm ./crykit ./compile_commands.json -rf

re: clean all

test: all
	cd $(BUILD_DIR) && ctest --output-on-failure

.PHONY: all clean re
