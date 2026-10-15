"""Source-level lifecycle regression, supplementing the compiled I/O tests.

This checks shutdown wiring, not actual APT/HOME or GPU operation on hardware.
"""
from pathlib import Path

source = (Path(__file__).resolve().parents[1] / 'source/main.c').read_text(encoding='utf-8')
busy = source.split('static void busy_frame(', 1)[1].split('static unsigned hash_path', 1)[0]
assert 'if(!aptIsActive()||aptShouldClose())return;' in busy
assert busy.index('aptShouldClose()') < busy.index('C3D_FrameBegin(')
exit_code = source.split('/* HOME\'s Close command', 1)[1].split('init_failed:', 1)[0]
assert 'save_progress_on_exit();goto cleanup;' in exit_code
assert not any(call in exit_code for call in ('busy_frame(', 'save_progress();', 'C3D_FrameBegin(', 'gspWait'))
checkpoint = source.split('static void save_progress_on_exit(', 1)[1].split('static bool lcdNeedsApply;', 1)[0]
assert 'shutdown_save_write(' in checkpoint
assert not any(call in checkpoint for call in ('busy_frame(', 'fopen(', 'fclose(', 'C3D_'))
print('PASS: close-request busy-frame guard and no post-loop frame/synchronous SD save')
