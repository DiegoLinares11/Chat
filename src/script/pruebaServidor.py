import websocket
import time

def on_message(ws, message):
    print(f"Mensaje recibido: {message}")

def on_open(ws):
    print("Conexión abierta, enviando mensaje...")
    ws.send('{"type": "register", "sender": "usuario1"}')

def on_close(ws, close_status_code, close_msg):
    print("Conexión cerrada")

ws = websocket.WebSocketApp("ws://localhost:9000",
                            on_message=on_message,
                            on_open=on_open,
                            on_close=on_close)

ws.run_forever()
