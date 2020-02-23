# morsekey
[WiP] Morse code parser for Arduino, to be expanded into a telegraphy HID device

Connect a li'l speaker to pin 3 and a telegraph key to pin 2, and prepare to descend into utter despair, because while this thing is fully capable of flawlessly parsing Morse code when it feels like it, it's also perfectly capable of locking itself into a confused mode where everything is a dash.

The idea is to keep track of a moving average of dot times, both for tones and spaces, and when we encounter a new event that deviates more than a certain threshold, we can reevaluate our interpretation of previous events, so if we assumed we had encountered a bunch of dots and suddenly get a new tone that's much shorter, we may decide that all those dots were actually dashes, and update the baseline time for a 1U event (= a dot).

While this works surprisingly well, the current implementation quite often fails spectacularly.

If you dare to dive into this code, feel free to fix my bugs!

I've tried to use fairly sensible variable and function names but there's no coherent, carefully crafted code structure here, so enter at your own peril.
