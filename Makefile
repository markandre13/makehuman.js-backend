all:
	git submodule update --init --recursive

mediapipe:
	make -C upstream/mediapipe_cc_lib/cc_lib/

backend:
	make -C src
