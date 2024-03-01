main: 
	em++ app/src/main.cxx -o web/assets/out.js --preload-file app/assets/smile.png --use-preload-plugins

emhtml: src/main.cxx
	em++ app/src/main.cxx -o test/index.html --preload-file app/assets/smile.png --use-preload-plugins

clean:
	rm -f web/assets/out* test/index*