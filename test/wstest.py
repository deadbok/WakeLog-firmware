import sys
import websocket

if __name__ == "__main__":
	if (sys.argv[1]):
		print("You must supply the DUT IP address on the command line.")
		
	websocket.enableTrace(True)
	
	uri = "ws://" + sys.argv[1] + "/ws"
	print("Connecting to " + uri)
	ws = websocket.create_connection(uri)
    
	ws.send("getlog")
	print("Sent")
	print("Receiving...")
	result = ws.recv()
	print("Received '%s'" % result)
	ws.close()
