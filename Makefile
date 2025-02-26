.PHONY: all clean

all:
	$(MAKE) -C src
	mkdir -p build
	mkdir -p build/images
	mkdir -p build/grids
	cp -r data/* build/
	cp src/ui/*glade* build/
	cp src/ui/style.css build/
	cp src/run build/

clean:
	$(MAKE) -C src clean
	rm -rf build
