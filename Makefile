OUT_RELEASE	= ./bin/static/libSimpleHTTPGet.a
OUT_RELEASE_LINUX = ./bin/static/libSimpleHTTPGet.a
OUT_DEBUG = ./bin/Debug/SimpleHTTPGet.exe
OUT_DEBUG_LINUX = ./bin/Debug/SimpleHTTPGet
OBJ_DEBUG_PATH = ./obj/Debug
OBJ_RELEASE_PATH = ./obj/Release
SRC_PATH = ./src
CFLAGS_DEBUG = -Wall -std=c11 -g -O0 -D DIAGNOSTIC
#CFLAGS_DEBUG += -fsanitize=address
CFLAGS_RELEASE = -Wall -std=c11 -O3 

$(OUT_RELEASE): Release

$(OUT_DEBUG): Debug

$(OUT_DEBUG_LINUX): DebugLinux

Release:
	gcc $(CFLAGS_RELEASE) -c $(SRC_PATH)/socket.c -o $(OBJ_RELEASE_PATH)/socket.o
	ar rcs $(OUT_RELEASE) $(OBJ_RELEASE_PATH)/socket.o
	
ReleaseLinux: $(OBJ_RELEASE_PATH)/socket.o 
	ar rcs $(OUT_RELEASE_LINUX) $(OBJ_RELEASE_PATH)/socket.o

TestRelease: $(OUT_RELEASE)
	gcc $(CFLAGS_RELEASE) -o ./bin/Release/Main.exe $(SRC_PATH)/main.c -Lbin/static -l:libSimpleHTTPGet.a -lws2_32 -lssl -lcrypto -latomic -lpthread

TestReleaseLinux: $(OUT_RELEASE_LINUX)
	gcc $(CFLAGS_RELEASE) -o ./bin/Release/Main $(SRC_PATH)/main.c -Lbin/static -l:libSimpleHTTPGet.a -lssl -lcrypto -latomic
	 
Debug:
	gcc $(CFLAGS_DEBUG) -o $(OUT_DEBUG) $(SRC_PATH)/test.c -lws2_32 -lssl -lcrypto -lpthread -latomic

DebugLinux: 
	gcc $(CFLAGS_DEBUG) -o $(OUT_DEBUG_LINUX) $(SRC_PATH)/test.c -lssl -lcrypto -static-libasan

$(OBJ_DEBUG_PATH)/socket.o:
	gcc $(CFLAGS_DEBUG) -c $(SRC_PATH)/socket.c -o $(OBJ_DEBUG_PATH)/socket.o

$(OBJ_RELEASE_PATH)/socket.o:
	gcc $(CFLAGS_RELEASE) -c $(SRC_PATH)/socket.c -o $(OBJ_RELEASE_PATH)/socket.o

cleanDebug:
	rm $(OUT_DEBUG) $(OBJ_DEBUG_PATH)/socket.o $(OBJ_DEBUG_PATH)/test.o
	
cleanRelease:
	rm $(OUT_RELEASE) $(OBJ_RELEASE_PATH)/socket.o $(OBJ_RELEASE_PATH)/test.o

debug: $(OUT_DEBUG)
	gdb $(OUT_DEBUG)
	
debugLinux: $(OUT_DEBUG_LINUX)
	gdb $(OUT_DEBUG_LINUX)
	
valgrind: $(OUT_DEBUG_LINUX)
	valgrind $(OUT_DEBUG_LINUX)

valgrind_leakcheck: $(OUT_DEBUG_LINUX)
	valgrind --leak-check=full $(OUT_DEBUG_LINUX)

valgrind_extreme: $(OUT_DEBUG_LINUX)
	valgrind --leak-check=full --show-leak-kinds=all --leak-resolution=high --track-origins=yes --vgdb=yes $(OUT_DEBUG_LINUX)
