import importlib.util
from pathlib import Path
import unittest

spec = importlib.util.spec_from_file_location('checkVideoDDLog', Path(__file__).resolve().parents[1] / 'checkVideoDDLog.py')
video = importlib.util.module_from_spec(spec)
spec.loader.exec_module(video)

# The retained browser run's appended record, excluding historical log data.
RECORD = b'''
      ****************************************************************
  Windows 9X/NT/2K/XP Direct Draw Test Version 4.1, Thu Sep 10 06:24:15 2026
                 Copyright (C) 1998 - 2003, Roy Longbottom
                     5 seconds tests, Primary Display Driver,
              Millions of Pixels Per Second ......... Frames Per Second .....
  Resolution  bltfast blt_and bltfill bltrect bltfast  vsynch bltfast bltfast
              vid_ram stretch rectngl +vsynch cpu_ram refresh vid_ram cpu_ram
  800  600 32     6.0     8.2    10.0     7.7     7.0    17.0    15.4    15.4
  Windows NT Version 6.2, build 9200,
  CPU GenuineIntel, Features Code 0788A131, Model Code 00000F07, 99 MHz
  Memory 1048572 KB, Free 786432 KB
          Run Time Each Test 5 seconds, end at Thu Sep 10 06:24:40 2026
'''


class VideoDDLogTests(unittest.TestCase):
    def test_history_is_excluded_and_exit_is_never_waived(self):
        before = RECORD + b'ESCAPE PRESSED\r\n'
        result = video.check_log(before, before + RECORD.replace(b'\n', b'\r\n'))
        self.assertTrue(result['completion_recorded'])
        self.assertFalse(result['game_acceptance'])
        self.assertFalse(result['exit_status_accepted'])
        self.assertEqual(result['historical_bytes'], len(before))
        self.assertEqual(len(result['measurements']), 8)

    def test_old_or_rewritten_log_is_rejected(self):
        for after in (RECORD, RECORD[:-1], b'changed' + RECORD):
            with self.subTest(after=after[:8]), self.assertRaises(ValueError):
                video.check_log(RECORD, after)

    def test_escape_or_error_despite_footer_is_rejected(self):
        for line in (b'ESCAPE PRESSED', b'DirectDraw failed', b'ERROR 88760096'):
            with self.subTest(line=line), self.assertRaises(ValueError):
                video.check_log(b'', RECORD.replace(b'  Windows NT', line + b'\n  Windows NT'))

    def test_incomplete_and_duplicate_records_are_rejected(self):
        for data in (RECORD[:RECORD.index(b'Run Time')], RECORD + RECORD,
                     RECORD.replace(b'    15.4    15.4', b'    15.4')):
            with self.subTest(length=len(data)), self.assertRaises(ValueError):
                video.check_log(b'', data)

    def test_wrong_resolution_duration_and_bad_values_are_rejected(self):
        for old, new in ((b'800  600 32', b'640  480 32'), (b'Each Test 5', b'Each Test 2'),
                         (b'6.0', b'0.0'), (b'6.0', b'nan'), (b'06:24:40', b'06:24:10')):
            with self.subTest(new=new), self.assertRaises(ValueError):
                video.check_log(b'', RECORD.replace(old, new))

    def test_out_of_order_footer_or_missing_headings_is_rejected(self):
        for data in (RECORD + b'  CPU trailing\n', RECORD.replace(b'Resolution', b'wrong-columns')):
            with self.subTest(data=data[-25:]), self.assertRaises(ValueError):
                video.check_log(b'', data)


if __name__ == '__main__':
    unittest.main()
