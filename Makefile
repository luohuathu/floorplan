CC = gcc

HOTSPOT_DIR = ./HotSpot-5.02

INC = -I$(HOTSPOT_DIR)
LIB = -static -L$(HOTSPOT_DIR) -lhotspot -lm

floorplan: floor_plan_lh.c util_lh.c 
	$(CC) -o $@ $^ $(INC) $(LIB)

