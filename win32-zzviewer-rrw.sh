mkdir -p bin
i686-w64-mingw32.static-gcc -o bin/zzviewer-rrw.exe -DMATH_CONST=const -DBINDING=SWRAST `i686-w64-mingw32.static-pkg-config --cflags --libs sdl` -lm -mconsole -mwindows \
-DNDEBUG -Ofast -s \
-Wno-unused-variable -Wno-parentheses -Wno-unused-function -Wno-unused-parameter -Wno-ignored-qualifiers -Wall -Wextra \
-Imain/include main/src/*.c main/src/sdl/*.c \
-Iz64/include z64/src/*.c z64/src/sdl/*.c z64/src/swrast/*.c
