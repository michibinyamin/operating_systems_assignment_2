
all: targil_1 targil_2 targil_3 targil_4 targil_6

.PHONY: targil_1 targil_2 targil_3 targil_4 targil_6


targil_1:
	$(MAKE) -C targil_1

targil_2:
	$(MAKE) -C targil_2

targil_3:
	$(MAKE) -C targil_3

targil_4:
	$(MAKE) -C targil_4

targil_6:
	$(MAKE) -C targil_6

clean :
	$(MAKE) -C targil_1 clean
	$(MAKE) -C targil_2 clean
	$(MAKE) -C targil_3 clean
	$(MAKE) -C targil_4 clean
	$(MAKE) -C targil_6 clean