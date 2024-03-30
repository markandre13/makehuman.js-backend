all:
	git submodule update --init --recursive

MEDIAPIPE_CPP_DIR=/Users/mark/upstream/mediapipe_cpp_lib

link:
	ln -s $(MEDIAPIPE_CPP_DIR)/src/gmod_api.h src
	ln -s $(MEDIAPIPE_CPP_DIR)/import_files mediapipe
	ln -s $(MEDIAPIPE_CPP_DIR)/mediapipe_graphs .
	ln -s $(MEDIAPIPE_CPP_DIR)/mediapipe_models .

update:
	git submodule update --remote --rebase --recursive
