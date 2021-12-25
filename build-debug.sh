mkdir -p bin
gcc -o bin/zzviewer-rrw -DMATH_CONST=const -DBINDING=SWRAST -lSDL -lm \
-Wno-unused-variable -Wno-parentheses -Wno-unused-function -Wno-unused-parameter -Wno-ignored-qualifiers -Wall -Wextra \
-Imain/include main/src/*.c main/src/sdl/*.c \
-Iz64/include z64/src/*.c z64/src/sdl/*.c z64/src/swrast/*.c
