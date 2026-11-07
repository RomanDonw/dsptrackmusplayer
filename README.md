# Very-simple tracker music player based on libopenmpt for my DSP filters loader.
## Command-line options
- `-f` - specifies path to source being played tracker music file.
- `-a` - amplitude modifier of output signal.
- `-v` - volume modifier of output signal.
- `-s` - suppress quitting/stay connected/alive when song ends.
- `-l` - repeat count: `= 0` - play once, `> 0` - repeat specified number of times, `< 0` - infinite playing (repeat forever).

## Dependencies
- [loaddsp](https://codeberg.org/RomanDonw/loaddsp).
- libopenmpt.
