#!/usr/bin/env python3.5
#conding=utf-8

class washer:
	'''
	Full Atomatic Washing
	'''
	name = "Full Aotomatic washing"
	product = "2010"

	def __init__(self, water=200, scour=10):
		self.water = water
		self.scour = scour
	
	def set_water(self, water):
		self.water = water

	def set_scour(self, scour):
		self.scour = scour

	def start_washer(self):
		print("Water: ", self.water)
		print("Scour: ", self.scour)

	def washer_info(self):
		print("\n*************Washer info***************")
		print("Washer name: ", washer.name)
		print("Washer production date: ", washer.product);
		print("Current water: ", self.water)
		print("Current scour: ", self.scour)
		
if __name__ == '__main__':

	dir(washer)
	washer.__doc__
	w = washer(100, 5)
	w.set_water(500)
	w.set_scour(20)
	w.start_washer()
	w.washer_info()
