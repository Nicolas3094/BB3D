BUILD_DIR_PREFIX := build
BUILD_TYPE := $(if $(BUILD_TYPE),$(BUILD_TYPE),Debug)
TEST_LOG_FILE := test_$(BUILD_TYPE).log
BENCHMARK_LOG_FILE := benchmark_$(BUILD_TYPE).log
BUILD_DIR := "$(BUILD_DIR_PREFIX)/$(BUILD_TYPE)"

PERF := $(shell command -v perf 2> /dev/null)
STACKCOLLAPSE := $(shell command -v stackcollapse-perf.pl 2> /dev/null)
FLAMEGRAPH := $(shell command -v flamegraph.pl 2> /dev/null)
PROFILE_EXECUTABLE := "$(BUILD_DIR)/app/allocate"
PROFILE_EXECUTABLE_ARGS := --maximum_running_time_seconds=10
PROFILE_INPUT_FILE := ./doc/set_15_instance_1.json
FREQUENCY := 1000
PERF_RECORD_OUTPUT := "perf.record.out"
PERF_SCRIPT_OUTPUT := "perf.script.out"
PERF_REPORT_OUTPUT := "perf.report.out"
PERF_FOLDED_OUTPUT := "perf.folded.out"
PERF_FIGURE_OUTPUT := "perf.svg"

NUMBER_OF_THREADS_AVAILABLE := $$(nproc --all)
NUMBER_OF_THREADS_REDUCED := $$((3 * $(NUMBER_OF_THREADS_AVAILABLE) / 4))
NUMBER_OF_THREADS_TO_USE := $$(($(NUMBER_OF_THREADS_REDUCED) < 1 ? 1 : $(NUMBER_OF_THREADS_REDUCED)))

.PHONY: build test benchmark profile memcheck enforce_code_style clean

build:
	cmake . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) -DCMAKE_INSTALL_PREFIX=$(INSTALL_LOCATION)
	cmake --build $(BUILD_DIR) -- -j $(NUMBER_OF_THREADS_TO_USE)

test: build
	ctest --timeout 9 --test-dir $(BUILD_DIR) -j $(NUMBER_OF_THREADS_TO_USE) --output-log $(TEST_LOG_FILE) --output-on-failure

benchmark: build
	echo "" > $(BENCHMARK_LOG_FILE)
	for BENCHMARK in `find $(BUILD_DIR)/benchmark/ -type f -perm /111` ; do \
		$$BENCHMARK --benchmark-confidence-interval 0.99 --rng-seed 1234 | tee -a $(BENCHMARK_LOG_FILE) ; \
	done

profile: build
ifndef PERF
	$(error "the 'perf' command is not available. Please install it to run performance profiles")
endif
	perf record -F $(FREQUENCY) -g -o $(PERF_RECORD_OUTPUT) -- $(PROFILE_EXECUTABLE) $(PROFILE_EXECUTABLE_ARGS) $(PROFILE_INPUT_FILE)
	perf script -i $(PERF_RECORD_OUTPUT) > $(PERF_SCRIPT_OUTPUT)
	perf report -i $(PERF_RECORD_OUTPUT) --stdio --call-graph none > $(PERF_REPORT_OUTPUT)
	@echo "go to <https://thlorenz.com/flamegraph/web/> and upload the file '$(PERF_SCRIPT_OUTPUT)'"
ifdef STACKCOLLAPSE
	stackcollapse-perf.pl $(PERF_SCRIPT_OUTPUT) > $(PERF_FOLDED_OUTPUT)
endif
ifdef FLAMEGRAPH
	flamegraph.pl $(PERF_FOLDED_OUTPUT) > $(PERF_FIGURE_OUTPUT)
	@echo "or open the file $(PERF_FIGURE_OUTPUT) in the browser: firefox $(PERF_FIGURE_OUTPUT)"
endif

memcheck: build
	valgrind \
		--leak-check=full \
		--show-leak-kinds=all \
		--track-origins=yes \
		--verbose \
		--log-file=app_allocate_memcheck.log \
		$(BUILD_DIR)/app/allocate --model=g2la $(BUILD_DIR)/../app/set_15_instance_1.json --save-output --output-file=./foo.json

enforce_code_style:
	find include/ src/ -iname "*.hpp" -o -iname "*.cpp" | xargs clang-format -i

clean:
	rm -rf $(BUILD_DIR) .cache/ *.log
