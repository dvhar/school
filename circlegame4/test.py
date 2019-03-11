#!/usr/bin/env python3

from turtle import Turtle
from math import pi, sin, cos
t = Turtle()
t.screen.bgcolor('black')
t.color('yellow')
t.shape('arrow')
t.shapesize(3)
print('tracer',t.screen.tracer())

t.setheading(0)
while(True):
    d = input()
    t.setheading(int(d))

