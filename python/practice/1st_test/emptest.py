#!/usr/bin/python
#coding=utf-8

class Employee:
	'all employee'
	empCount = 0

	def __init__(self, name, salary):
		self.name = name
		self.salary = salary
		Employee.empCount += 1
	
	def displayCount(self):
		print("Total employee", Employee.empCount)

	def displayEmployee(self): 
		print("Name: ", self.name,  ", Salary: ", self.salary)

if __name__ == '__main__':
	emp1 = Employee("lily", 2000) 
	emp2 = Employee("jack", 3000)
	emp1.displayEmployee()
	emp2.displayEmployee()
	emp1.displayCount()
