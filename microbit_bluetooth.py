def on_bluetooth_connected():
    global connected
    basic.show_icon(IconNames.HEART)
    connected = 1
bluetooth.on_bluetooth_connected(on_bluetooth_connected)

def on_bluetooth_disconnected():
    global connected
    basic.show_icon(IconNames.NO)
    connected = 0
bluetooth.on_bluetooth_disconnected(on_bluetooth_disconnected)

def on_button_pressed_a():
    basic.show_string(control.device_name())
input.on_button_pressed(Button.A, on_button_pressed_a)

def on_button_pressed_b():
    if connected == 1:
        bluetooth.uart_write_line("Hello from microbit")
        basic.show_icon(IconNames.YES)
        basic.pause(500)
        basic.show_icon(IconNames.HEART)
input.on_button_pressed(Button.B, on_button_pressed_b)

connected = 0
basic.show_icon(IconNames.NO)
bluetooth.start_uart_service()
