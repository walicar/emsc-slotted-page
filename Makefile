main: src/main.c
	emcc src/main.c -o web/assets/out.js -pthread -sPROXY_TO_PTHREAD -sPTHREAD_POOL_SIZE=4 --preload-file src/smile.png --use-preload-plugins

emhtml: src/main.c
	emcc src/main.c -o test/index.html -pthread -sPROXY_TO_PTHREAD -sPTHREAD_POOL_SIZE=4 --preload-file src/smile.png --use-preload-plugins

clean:
	rm -f web/assets/out* test/index*