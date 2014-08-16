import pygame
import serial
import pygame.midi
from time import sleep

instrument = 0
note = 74
volume = 127

pygame.init()
pygame.midi.init()

#start serial connection
ser = serial.Serial('/dev/rfcomm0', 9600) #0733191091 - devaki :)

for id in range(pygame.midi.get_count()):
	print pygame.midi.get_device_info(id)

midiOutput = pygame.midi.Output(0, 14)
midiOutput.set_instrument(54)

notes = [60, 62, 64, 65, 67, 69, 71, 72, 73, 75]
prev_inputs = [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]

mode = 'normal'

while True:
	if mode == 'normal':
		string = ser.readline()
		#print string
		inputs = string.split('A')
		#print inputs
		if len(inputs) == 13:
			for i in xrange(12):
				try:
					input = int(inputs[i])
				except:
					print "OH NO!!"
					input = 0

				if i < 10:
					if input == 1:
						if prev_inputs[i] == 0:
							prev_inputs[i] = 1
							midiOutput.note_on(notes[i], 127, 1)
							print "noteon, ", notes[i]
					else:
						if prev_inputs[i] == 1:
							prev_inputs[i] = 0
							midiOutput.note_off(notes[i], 127, 1)
							print "noteoff, ", notes[i]
		ser.write("hello")

del midiOutput
pygame.midi.quit()