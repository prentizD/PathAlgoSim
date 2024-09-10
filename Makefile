
game.htlm: main.c 
	emcc -o game.html main.c -Os -Wall ./lib/libraylib.a -I./include -s USE_GLFW=3 -s --shell-file ./minshell.html -DPLATFORM_WEB

clean:
	rm -f game.wasm game.html game.js