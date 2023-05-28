OBJ_DIR = object
LIB_DIR = lib
OBJECTOS = data.o entry.o tree.o
CLIENT_LIB = client_stub.o network_client.o data.o entry.o message.o
SERVER= network_server.o tree_server.o tree_skel.o sdmessage.pb-c.o message.o client_stub.o network_client.o
TREE_CLIENT= tree_client.o sdmessage.pb-c.o
data.o = data.h
entry.o = entry.h
tree.o = tree.h
client_stub.o = client_stub.h
network_client.o = network_client.h
tree_skel.o = tree_skel.h
network_server.o = network_server.h
tree_client.o = tree_client.h
tree_server.o = tree_server.h
sdmessage.pb-c.o = sdmessage.pb-c.h
message.o = message-private.h

CC = gcc
FLAGS = -lrt -lpthread -lm

vpath %.o $(OBJ_DIR)

all: proto cd-proto build-proto client_lib tree-client tree-server

proto:
	protoc --c_out=. sdmessage.proto

cd-proto:
	mv sdmessage.pb-c.c source/
	mv sdmessage.pb-c.h include/

build-proto: source/sdmessage.pb-c.c include/sdmessage.pb-c.h
	$(CC) -g -I include -o $(OBJ_DIR)/sdmessage.pb-c.o -c source/sdmessage.pb-c.c

client_lib: $(CLIENT_LIB)
	ld -r $(addprefix $(OBJ_DIR)/,$(CLIENT_LIB)) -o lib/client-lib.o

tree-client: $(TREE_CLIENT)
	$(CC) -g  $(LIB_DIR)/client-lib.o $(addprefix $(OBJ_DIR)/,$(TREE_CLIENT)) -I/usr/local/include -L/usr/local/lib -lprotobuf-c -lzookeeper_mt -DTHREADED -o binary/tree-client $(FLAGS)

tree-server: $(SERVER) $(OBJECTOS)
	$(CC) -g  $(addprefix $(OBJ_DIR)/,$(SERVER)) $(addprefix $(OBJ_DIR)/,$(OBJECTOS)) -I/usr/local/include -L/usr/local/lib -lprotobuf-c -lzookeeper_mt -DTHREADED -o binary/tree-server $(FLAGS)

%.o: source/%.c $($@)
	$(CC) -g -I include -o $(OBJ_DIR)/$@ -c $<

clean:
	rm -rf $(OBJ_DIR)/*.o
	rm -rf $(LIB_DIR)/*.o
	rm -rf binary/*
	rm -rf source/sdmessage.pb-c.c
	rm -rf include/sdmessage.pb-c.h