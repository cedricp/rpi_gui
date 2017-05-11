.PHONY: fm_lib

all: wiringpi fm_lib tda7419 lcd

fm_lib:
	$(MAKE) -C fm_lib
	
tda7419:
	$(MAKE) -C tda7419_lib
	
wiringpi:
	$(MAKE) -C wiringpi_lib
	
lcd:
	$(MAKE) -C lcd_lib