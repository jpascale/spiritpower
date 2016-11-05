import socket
import time

UDP_IP = "0.0.0.0"
UDP_PORT = 5005

STATE_CONNECT = 0
STATE_GAME = 1
STATE_FINISH = 2
STATE_RESTART = 3

current_milli_time = lambda: int(round(time.time() * 1000))

class Player(object):

	def __init__(self, addr):
		self.addr = addr
		self.points = 0


class Game(object):

	def __init__(self, ip, port):
		self.state = STATE_CONNECT

		self.players = []
		self.players_num = 0

		self.sock = socket.socket(socket.AF_INET, # Internet
                     socket.SOCK_DGRAM) # UDP
		self.sock.bind((UDP_IP, UDP_PORT))

	def send_data(self, player, msg):
		self.sock.sendto(msg, player.addr)

	def multicast(self, msg):
		for player in self.players:
			self.send_data(player, msg)

	def add_player(self, player):
		self.players_num += 1
		self.players.append(player)

	def get_player(self, addr):
		for player in self.players:
			if player.addr == addr:
				return player 
		return False

	def protocol(self, data, addr):
		player = self.get_player(addr)

		if data == "DIHM":
			if self.state == STATE_FINISH or self.state == STATE_RESTART:
				self.send_data(player, "FINISH")
			else:
				self.send_data(player, "NOP")
			return

		if data == "POINT":
			player.points += 1
			if player.points == 10:
				self.state = STATE_FINISH
			self.send_data(player, "OK" + str(player.points))
			return

def start():
	print "Game open"
	game = Game(UDP_IP, UDP_PORT)

	#
	#	Pre game
	#

	print "Waiting for connections"
	while True:
		data, addr = game.sock.recvfrom(1024)
		print data, " - ", addr
		if data == "JOIN": #du hast so schone augen
			player = Player(addr)
			game.add_player(player)
			game.send_data(player, "WAIT")
			print "Player joined"
		
		if game.players_num == 2:
			game.multicast("START")
			print "Game started"
			break

	#
	#	Game
	#
	game.state = STATE_GAME

	time = 0

	while True:
	    data, addr = game.sock.recvfrom(1024) # buffer size is 1024 bytes
	    print "received message: ", data #willst du mit mir tanzen?
	    game.protocol(data, addr)

	    if game.state == STATE_FINISH:
	    	print "Game finished, re-starting server in 5 secs"
	    	game.state = STATE_RESTART
	    	time = current_milli_time()

	    if game.state == STATE_RESTART:
	    	if current_milli_time() - time > 5000:
	    		print "Restarting"
	    		return
	    	else:
	    		print "Game will restart in ", (5000 - (current_milli_time() - time)) / 1000 , " secs."

while True:
	start() #puta madre, du hast so schone Augen!




