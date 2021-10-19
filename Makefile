OUT_RELEASE	= ./bin/Release/SimpleHTTPGet.exe
OUT_RELEASE_LINUX = ./bin/Release/Prayer_times.a
OUT_DEBUG = ./bin/Debug/SimpleHTTPGet.exe
OUT_DEBUG_LINUX = ./bin/Debug/Prayer_Times.a
OBJ_DEBUG_PATH = ./obj/Debug
OBJ_RELEASE_PATH = ./obj/Release
SRC_PATH = ./src
CFLAGS_DEBUG = -Wall -std=c11 -g -O0
#CFLAGS_DEBUG += -fsanitize=address
CFLAGS_RELEASE = -Wall -std=c11 -O3 

all: DebugLinux

release: Release

$(OUT_RELEASE): Release

$(OUT_DEBUG): Debug

Release: $(OBJ_RELEASE_PATH)/socket.o $(OBJ_RELEASE_PATH)/test.o 
	gcc $(CFLAGS_RELEASE) -o $(OUT_RELEASE) $(OBJ_RELEASE_PATH)/test.o  -mwindows -lws2_32 -lssl -lcrypto
	
ReleaseLinux: $(OBJ_RELEASE_PATH)/socket.o $(OBJ_RELEASE_PATH)/test.o 
	gcc $(CFLAGS_RELEASE) -o $(OUT_RELEASE_LINUX) $(OBJ_RELEASE_PATH)/socket.o $(OBJ_RELEASE_PATH)/test.o  -lssl -lcrypto
 
Debug:
	gcc $(CFLAGS_DEBUG) -o $(OUT_DEBUG) $(SRC_PATH)/test.c -lws2_32 -lssl -lcrypto

DebugLinux: $(OBJ_DEBUG_PATH)/socket.o $(OBJ_DEBUG_PATH)/test.o 
	gcc $(CFLAGS_DEBUG) -o $(OUT_DEBUG_LINUX) $(OBJ_DEBUG_PATH)/socket.o $(OBJ_DEBUG_PATH)/test.o  -lssl -lcrypto -static-libasan

$(OBJ_DEBUG_PATH)/socket.o:
	gcc $(CFLAGS_DEBUG) -c $(SRC_PATH)/socket.c -o $(OBJ_DEBUG_PATH)/socket.o
	
$(OBJ_DEBUG_PATH)/test.o:
	gcc $(CFLAGS_DEBUG) -c $(SRC_PATH)/test.c -o $(OBJ_DEBUG_PATH)/test.o

$(OBJ_RELEASE_PATH)/socket.o:
	gcc $(CFLAGS_RELEASE) -c $(SRC_PATH)/socket.c -o $(OBJ_RELEASE_PATH)/socket.o
	
$(OBJ_RELEASE_PATH)/test.o:
	gcc $(CFLAGS_RELEASE) -c $(SRC_PATH)/test.c -o $(OBJ_RELEASE_PATH)/test.o

cleanDebug:
	rm $(OUT_DEBUG) $(OBJ_DEBUG_PATH)/socket.o $(OBJ_DEBUG_PATH)/test.o
	
cleanRelease:
	rm $(OUT_RELEASE) $(OBJ_RELEASE_PATH)/socket.o $(OBJ_RELEASE_PATH)/test.o

run: $(OUT_RELEASE)
	$(OUT_RELEASE)
	
runLinux: $(OUT_RELEASE_LINUX)
	$(OUT_RELEASE_LINUX)

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
