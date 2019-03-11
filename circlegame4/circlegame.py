#!/usr/bin/env python3

'''
David Hardy  3/6/19  "cs4500: Introduction to the Software Profession"

Program explanation:
    This program asks the user to enter a file with circles and arrows until one is provided.
    It shows an animated graphic of the circles getting traversed by a green arrow,
    with each circle changing colors from grey to red to blue each time it gets visited.
    It them displays some stats and waits for the user to click on the screen to exit.

Input file:
    Program prompts user for a file until one is provided. It assumes the file is properly 
    formated and contains a strongly connected graph. There is no output file.

Data structures:
    circle: class used to represent each circle. Contains the number of checks, as well as methods
        for manipulating the circle. Also contains graphical information like coordinates and color.
        Methods:
            - __init__: Initialize the circle numerically, as well as graphically when given its number
                and the total number of circles. Brown arrows are handled by the loader function.
            - add_arrow: Adds and arrow to the circle's list of arrows.
            - check: Adds a check to the circle numerically and changes the circle's color.
            - next: Returns a random circle from the circle's arrrow list.
    traverser: class used to represent the marker as it traverses the graph.
        Methods:
            - __init__: Initializes the traverser by setting its circle index, coordinates, and graphic.
            - next: moves the traverser to the next circle numerically and graphically. Calls the target
                circles's check() method to change its color and add a check.
    circles: list of circle objects.

Dependencies:
    Python3. Turtle graphics requires tkinter library.
'''

from math import sin, cos, pi, atan
from random import randint
from turtle import Turtle, TurtleScreen
from time import sleep
import os

# caluclate direction in degrees when given any change in position
def getangle(dx,dy):
    #calculate angle, guard against div by 0 errors
    angle =  (atan(dy/(dx,0.0000001)[dx==0])/pi)*180
    #make sure arrow doesn't point in opposite direction
    if dx > 0:
        angle += 180
    return angle

#class for the marker that traverses the circles
class traverser:

    #initialize taverser at circle 1
    def __init__(self):
        self.i = 1
        #create an arrow graphic
        self.g = Turtle()
        self.g.color('#33ff77')
        self.g.shape('arrow')
        self.g.pensize(2)
        self.g.shapesize(3)
        self.g.speed(4)
        #arrow starts at center
        self.x = 0
        self.y = 0

    #go to the next circle
    def next(self, circle):
        #point the arrow in the right direction and go there
        dx = self.x - circle.x
        dy = self.y - circle.y
        self.g.setheading(getangle(dx,dy))
        self.g.goto(circle.x, circle.y)
        #add an arrowhead to the line that was drawn
        pointer = Turtle(visible=False)
        pointer.screen.tracer(0,0)
        pointer.penup()
        pointer.shape('arrow')
        pointer.color('#33ff77')
        pointer.setx(0.1*self.x+0.9*circle.x)
        pointer.sety(0.1*self.y+0.9*circle.y)
        pointer.setheading(getangle(dx,dy))
        pointer.showturtle()
        pointer.screen.tracer(1,6)
        #update corrdinates, add a check to the circle, and load index of next circle
        self.x = circle.x
        self.y = circle.y
        circle.check()
        self.i = circle.next()
        
#circle class
class circle:

    def __init__(self, number, outof):
        #initialize a circle with an empty list of arrows
        self.arrows = []
        self.num_arrows = 0
        self.checks = 0
        #calculate the postition and initialize the color to grey
        self.x = cos(2*pi*(float(number)/outof))*200
        self.y = sin(2*pi*(float(number)/outof))*200 + 30
        self.color = [45,45,45]
        self.colorstate = 0
        self.i = number
        #if the circle is not in the placeholder position, give it a graphic
        if number > 0:
            self.g = Turtle(visible=False)
            #speed up rendering by setting tracer to 0,0
            self.g.screen.tracer(0,0)
            self.g.screen.bgcolor('black')
            self.g.screen.colormode(255)
            self.g.color(self.color)
            self.g.shape('circle')
            self.g.shapesize(3)
            self.g.penup()
            self.g.goto(self.x, self.y)
            self.g.pendown()
            self.g.showturtle()
            #once circle is displayed, reset rendering to normal speed
            self.g.screen.tracer(1,6)

    #add an arrow during the input loading phase
    def add_arrow(self, target):
        self.arrows.append(int(target))
        self.num_arrows += 1

    #alter the circle when it recieves a checkmark
    #global vars: total_checks, max_checks
    def check(self):
        # add the check numerically
        global total_checks, max_checks
        total_checks += 1
        self.checks += 1
        if self.checks > max_checks:
            max_checks = self.checks

        #increment the color. When one RGB component hits a limit, change the next one
        if self.colorstate == 0:
            self.color[0] += 70
            if self.color[0] == 255:
                self.colorstate += 1
        if self.colorstate == 1:
            self.color[1] += 70
            if self.color[1] == 255:
                self.colorstate += 1
        if self.colorstate == 2:
            self.color[2] += 70
            if self.color[2] == 255:
                self.colorstate += 1
        if self.colorstate == 3:
            self.color[0] -= 70
            if self.color[0] == 45:
                self.colorstate += 1
        if self.colorstate == 4:
            self.color[1] -= 70
            if self.color[1] == 45:
                self.colorstate += 1
        #render the newly caluculated color
        self.g.color(self.color)

    #return a random member of the circle's arrow array
    def next(self):
        return self.arrows[randint(1,self.num_arrows)-1]

#global variables
infile = 'i2';
total_checks = 0
max_checks = 0
num_circles = 0
circles = []

#load circls from file when given a file name
#global vars: circles, num_circles
def load_file(infile):
    global num_circles, circles

    #open the file and get circle data
    with open(infile) as f:
        for i,line in  enumerate(f.readlines()):
            #get the number of circles from the first line and initialize list of circles
            if i == 0:
                num_circles = int(line)
                circles = [circle(x, num_circles) for x in range(num_circles+1)]
            #get each arrow
            if i > 1:
                l = line.split(' ')
                circles[int(l[0])].add_arrow(l[1])

    #draw all the arrows from each circle in brown
    for c in circles[1:]:
        for a in c.arrows:
            #draw the line between each circle
            arrow = Turtle(visible=False)
            arrow.screen.tracer(0,0)
            arrow.penup()
            arrow.color('#553300')
            arrow.goto(c.x,c.y)
            arrow.pendown()
            arrow.pensize(2)
            arrow.goto(circles[a].x, circles[a].y)
            #add an arrowhead to the line showing direction
            arrowHead = Turtle(visible=False)
            arrowHead.penup()
            arrowHead.shape('arrow')
            arrowHead.color('#553300')
            #linear interpolation to find the right position near the end of the line
            arrowHead.setx(0.1*c.x+0.9*circles[a].x)
            arrowHead.sety(0.1*c.y+0.9*circles[a].y)
            arrowHead.setheading(getangle(c.x-circles[a].x, c.y-circles[a].y))
            arrowHead.showturtle()
            arrowHead.screen.tracer(1,6)

#play the game
#global vars: circles, num_circles
def game():
    global circles, num_circles
    num_marked = 0
    marker = traverser()
    while num_marked < num_circles:
        c = circles[marker.i]
        if c.checks == 0:
            num_marked += 1
        marker.next(c)

#prompt user for an input file
while (True):
    print('Please enter a file name')
    infile = input()
    #stop asking when given a valid file name
    if os.path.isfile(infile):
        break
    print('File not found. Enter another one')

#load the file and play the game
load_file(infile)
game()
#draw game statistics under the game area when it is done
text = Turtle(visible=False)
text.penup()
text.goto(0,-250)
text.pendown()
text.color('#88FF88')
text.write("Total checks: "+str(total_checks)+"   Max: "+str(max_checks)+"   Avg: "+str(total_checks/num_circles),
            move=False,align="center",font=("Cambria",20,"normal"))
text.screen.exitonclick()

