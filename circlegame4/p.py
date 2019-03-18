#!/usr/bin/env python3

import numpy as np
from keras.models import Sequential
from keras.layers import Dense
import matplotlib.pyplot as plt

fname = "winequality-red.csv"

#load data from file
data = np.genfromtxt(fname, delimiter=',')[1:]
np.random.shuffle(data)
data = data.astype(float)
trainx = data[:1200, :11]
trainy = data[:1200, 11:]
testx = data[1200:, :11]
testy = data[1200:, 11:]

#show data
#print(train.shape)
#print(train)
#print(validate.shape)
print(testx[:5,:])
print(testx.shape)
print(testy[:5,:])
print(testy.shape)


#create model
model = Sequential()
model.add(Dense(15, input_dim = 11, activation='sigmoid'))
model.add(Dense(10, input_dim = 15, activation='relu'))
model.add(Dense(15, input_dim = 10, activation='relu'))
model.add(Dense(1, input_dim = 10, activation='linear'))

print(model.summary())
model.compile(loss='mse', optimizer='adam', metrics=['mse'])


history = model.fit(trainx, trainy, epochs=1000 ,batch_size=10, verbose = 2, validation_data = (testx, testy))

prediction = model.predict(testx)
print(prediction[:10])
print(testy[:10])
