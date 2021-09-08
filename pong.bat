@ECHO OFF
g++ -Isrc/Include -Lsrc/lib -o pong pong.cpp -lmingw32 -lSDL2main -lSDL2