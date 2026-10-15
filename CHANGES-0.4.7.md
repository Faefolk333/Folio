# Folio 0.4.7

- Removes post-loop rendering when HOME Menu requests Close Software; busy frames
  also reject aptShouldClose(). Avoids an unnecessary VBlank/GPU wait on shutdown.
- Final progress save uses an isolated FS session and at most 500 ms of waiting;
  a stalled save cannot block the main thread indefinitely. On storage failure
  the final checkpoint may be lost. Save through Library before exiting if needed.
- Keeps the valid Homebrew startup logo from the owner-confirmed 0.4.6 build.
- HOME banner contains only the supplied book icon and Folio. Faefolk333 remains
  credited at startup, in-app and in publisher metadata.
- Stable Title ID and compatible per-book data. CIA installation revision 7.

ARM builds and host regressions are checked separately from hardware tests.
The owner confirmed 0.4.6 launch; this new HOME closing fix needs a console retest.
