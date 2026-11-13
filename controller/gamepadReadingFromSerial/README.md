== Microcontroller companion app for Overcooked2_NotifyInteractable

Exposes a microcontroller (RP2040) as 1 or 2 HID gamepads, and read from Serial the current
interactable (chopping board, dishwashing sink...) for each player, as notified by Overcooked2_NotifyInteractable
mod.
According to the current interactable, various actions send an event or not. 