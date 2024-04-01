all:
	git submodule update --init --recursive
	make -C upstream/mediapipe_cc_lib/cc_lib/
	make -C src

pull:
	git submodule update --remote --rebase --recursive
	git pull --rebase

mediapipe:
	make -C upstream/mediapipe_cc_lib/cc_lib/

backend:
	make -C src
