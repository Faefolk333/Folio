# Folio 0.4.8 — HOME close candidate

The owner confirms START exits immediately in 0.4.7, but HOME Menu's Close
Software still hangs. The earlier rendering/SD change was not a confirmed fix.

- Saves a bounded reading checkpoint before HOME suspension, including the
  suspend hook, rather than relying on the application being resumed to exit.
- A CIA-only low-priority guard observes the existing libctru close flag. After
  a sustained system close request while suspended, it performs the same APT
  PrepareToCloseApplication / CloseApplication sequence as libctru's normal exit.
  HOME alone, ordinary suspension and active reading do not trigger this path.
- The post-loop suspended-close path also closes directly without GPU teardown,
  covering a cancelled wakeup that does not restore graphics rights.
- Main and guard use atomic exit ownership to avoid concurrent APT exit calls.
- START/HBL keeps normal cleanup. No new SD paths, startup writes, permissions,
  Title ID or launch-logo changes. CIA installation revision 8.

Host tests cover close/no-close, active/inactive, suspended/restored, grace
period, failed prepare, main ownership, HBL exclusion and bounded saves. These
are mocks, not an on-console HOME test. This is a candidate pending that test.
