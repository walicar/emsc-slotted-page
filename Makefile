files := $(addprefix src/, main.cxx)

main: 
	em++ src/main.cxx -o web/assets/out.js --preload-file src/smile.png --use-preload-plugins

emhtml: src/main.cxx
	em++ src/main.cxx -o test/index.html --preload-file src/smile.png --use-preload-plugins

clean:
	rm -f web/assets/out* test/index*