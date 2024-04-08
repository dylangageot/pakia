CXX ?= g++

translation-map-generator: translation_map_generator.cc
	$(CXX) -I.. $^ -o $@

clean:
	rm -rf translation-map-generator

all: translation-map-generator

