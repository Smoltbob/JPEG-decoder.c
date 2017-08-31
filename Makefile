# Repertoires du projet
BIN_DIR = bin
SRC_DIR = src
INC_DIR = include
OBJ_DIR = obj
OBJPROF_DIR = obj-prof
TMP = obj-prof

# Options de compilation/édition des liens

CC = clang
LD = clang
INC = -I$(INC_DIR)

CFLAGS += $(INC) -Wall -std=c99 -O0 -g  -Wextra
LDFLAGS =-lm

# Si ELEVE vaut 1, on compile avec les modules bitstream, huffman et jpeg_desc
# que nous avons écrits.
# Si ELEVE vaut 0, on utilise les modules fournis avec le squelette.
ifeq ($(ELEVE),1)
	TMP = $(OBJ_DIR)
endif

# LISTE DES FICHIERS OBJET: TOUS LES .O SAUF CEUX QUI CONTIENNENT DES MAIN
OBJPROF_FILES = $(TMP)/bitstream.o $(TMP)/jpeg_reader.o $(TMP)/huffman.o
OBJ_FILES = $(OBJ_DIR)/extracteur.o $(OBJ_DIR)/ppm.o $(OBJ_DIR)/upsampling.o $(OBJ_DIR)/traitement.o $(OBJ_DIR)/conversion_rgb.o $(OBJ_DIR)/parametre_image.o $(OBJ_DIR)/structure.o $(OBJ_DIR)/securite.o
# OBJETS QUI CONTIENNENT UN MAIN = NOUVELLES CIBLES
# cible principale
JPEG2PPM = $(BIN_DIR)/jpeg2ppm
# cible test extracteur
TESTEX = $(BIN_DIR)/tests_extracteur
# cible test ppm
TESTPPM = $(BIN_DIR)/tests_ppm
# cible test rgb
TESTRGB = $(BIN_DIR)/test_rgb
# cible test_upsampler
TESTUPSAMPLER = $(BIN_DIR)/test_upsampler
# test des structures
TESTSTRUCT = $(BIN_DIR)/test_structure
# test jpeg_reader
TESTJPREADER = $(BIN_DIR)/test_jpeg_reader
# test decodeur
TESTDEC = $(BIN_DIR)/test_decoder
#test traitement
TESTTRAITEMENT = $(BIN_DIR)/test_traitement
# test mon_bitstream
TESTMONBIT = $(BIN_DIR)/test_mon_bitstream
#test mon_huffman
TESTHUFF = $(BIN_DIR)/test_huffman


all : testex testppm test_rgb test_upsampler test_structure test_jpeg_reader test_decoder test_mon_bitstream jpeg2ppm test_traitement test_huffman
# CIBLES POUR CHAQUE MAIN
#
go : $(FAIRE)

# jpeg2ppm
jpeg2ppm:
	@make FAIRE=$(JPEG2PPM) go

# Tests
testex:
	@make FAIRE=$(TESTEX) go

testppm:
	@make FAIRE=$(TESTPPM) go

test_rgb:
	@make FAIRE=$(TESTRGB) go

test_upsampler:
	@make FAIRE=$(TESTUPSAMPLER) go

test_structure:
	@make FAIRE=$(TESTSTRUCT) go

test_jpeg_reader:
	@make FAIRE=$(TESTJPREADER) go

test_decoder:
	@make FAIRE=$(TESTDEC) go

test_traitement:
	@make FAIRE=$(TESTTRAITEMENT) go

# modules
test_mon_bitstream:
	@make FAIRE=$(TESTMONBIT) go

test_huffman:
	@make FAIRE=$(TESTHUFF) go


# REGLES ASSOCIEES
$(FAIRE): $(OBJPROF_FILES) $(OBJ_FILES) $(subst bin, obj, $(FAIRE)).o
	$(LD) $(LDFLAGS) $(OBJPROF_FILES) $(OBJ_FILES) $(subst bin, obj, $(FAIRE)).o -o $@
	@echo -e "\033[3;5;32mMake terminé !\033[0m "

# objets requis par extracteur
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $^ -o $@

.PHONY: clean
.SILENT: $(FAIRE)

clean:
	rm -f $(TESTEX) $(OBJ_FILES) $(TESTPPM) $(TESTRGB) $(TESTUPSAMPLER) $(JPEG2PPM) $(TESTSTRUCT) $(TESTJPREADER) $(TESTDEC) $(TESTMONBIT) $(TESTHUFF) $(TESTTRAITEMENT)

mrproper: clean
	rm -f $(OBJ_FILES) core
